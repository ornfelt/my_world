#include "procfs.h"

#include <errno.h>
#include <proc.h>
#include <file.h>
#include <uio.h>
#include <std.h>
#include <mem.h>

#define CAT_INO(id) ((ino_t)(id) << 32)
#define CAT_ROOT CAT_INO(0)
#define CAT_TID CAT_INO(1)

#define TID_MASK(ino) (ino & 0xFFFFFFFF)
#define TID_INO(tid, ino) (CAT_TID | (tid) | ((ino_t)(ino) << 36))
#define TID_DIR  0x0
#define TID_NAME 0x1
#define TID_MAPS 0x2

#define ROOT_INO(ino) (CAT_ROOT | ((ino_t)(ino)))
#define ROOT_SELF 0x0

struct procfs_sb
{
	struct fs_sb *sb;
	ino_t ino;
	struct procfs_node *self;
};

struct procfs_node
{
	struct node node;
	struct node *parent;
	char *name;
	LIST_ENTRY(procfs_node) chain;
};

struct procfs_dir
{
	struct procfs_node node;
	LIST_HEAD(, procfs_node) nodes;
};

static int root_lookup(struct node *node, const char *name, size_t name_len,
                       struct node **child);
static int root_readdir(struct node *node, struct fs_readdir_ctx *ctx);
static int tid_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child);
static int tid_readdir(struct node *node, struct fs_readdir_ctx *ctx);

static int procfs_node_release(struct node *node);

static int procfs_mount(struct node *dir, struct node *proc,
                        unsigned long flags, const void *udata,
                        struct fs_sb **sb);
static int procfs_stat(struct fs_sb *sb, struct statvfs *st);

static ssize_t tid_name_read(struct file *file, struct uio *uio);
static ssize_t tid_maps_read(struct file *file, struct uio *uio);
static ssize_t tid_vm_read(struct file *file, struct uio *uio);

static ssize_t self_readlink(struct node *node, struct uio *uio);

static struct procfs_sb g_procfs;

static const struct fs_type_op fs_type_op =
{
	.mount = procfs_mount,
	.stat = procfs_stat,
};

struct fs_type g_procfs_type =
{
	.op = &fs_type_op,
	.name = "procfs",
	.flags = 0,
};

static const struct file_op root_fop =
{
};

static const struct node_op root_op =
{
	.release = procfs_node_release,
	.lookup = root_lookup,
	.readdir = root_readdir,
	.getattr = vfs_common_getattr,
};

static const struct node_op reg_op =
{
	.release = procfs_node_release,
	.getattr = vfs_common_getattr,
};

static const struct file_op self_fop =
{
};

static const struct node_op self_op =
{
	.readlink = self_readlink,
};

static const struct file_op tid_fop =
{
};

static const struct node_op tid_op =
{
	.release = procfs_node_release,
	.lookup = tid_lookup,
	.readdir = tid_readdir,
	.getattr = vfs_common_getattr,
};

static const struct file_op tid_name_fop =
{
	.read = tid_name_read,
};

static const struct file_op tid_maps_fop =
{
	.read = tid_maps_read,
};

static const struct file_op tid_vm_fop =
{
	.read = tid_vm_read,
};

static int fs_mknode(struct procfs_sb *sb, struct procfs_dir *parent,
                     ino_t ino, const char *name, mode_t mode,
                     fs_attr_mask_t mask, const struct fs_attr *attr,
                     const struct file_op *fop, const struct node_op *op,
                     size_t st_size, struct procfs_node **nodep)
{
	if (strchr(name, '/'))
		return -EINVAL;
	char *namedup = strdup(name);
	if (!namedup)
		return -ENOMEM;
	struct procfs_node *node = malloc(st_size, M_ZERO);
	if (!node)
	{
		free(namedup);
		return -ENOMEM;
	}
	node->node.sb = sb->sb;
	node->node.ino = ino;
	node->node.attr.mode = mode | (attr->mode & 07777);
	if (mask & FS_ATTR_UID)
		node->node.attr.uid = attr->uid;
	if (mask & FS_ATTR_GID)
		node->node.attr.gid = attr->gid;
	if (mask & FS_ATTR_ATIME)
		node->node.attr.atime = attr->atime;
	if (mask & FS_ATTR_CTIME)
		node->node.attr.ctime = attr->ctime;
	if (mask & FS_ATTR_MTIME)
		node->node.attr.mtime = attr->mtime;
	refcount_init(&node->node.refcount, 1);
	node->node.fop = fop;
	node->node.op = op;
	node->name = namedup;
	node->parent = parent ? &parent->node.node : &node->node;
	node_ref(node->parent);
	if (parent)
		LIST_INSERT_HEAD(&parent->nodes, node, chain);
	*nodep = node;
	return 0;
}

static int fs_mkdir(struct procfs_sb *sb, struct procfs_dir *parent,
                    ino_t ino, const char *name, fs_attr_mask_t mask,
                    const struct fs_attr *attr, const struct file_op *fop,
                    const struct node_op *op, struct procfs_dir **dirp)
{
	struct procfs_dir *dir;
	int ret = fs_mknode(sb, parent, ino, name, S_IFDIR, mask, attr,
	                    fop, op, sizeof(*dir), (struct procfs_node**)&dir);
	if (ret)
		return ret;
	LIST_INIT(&dir->nodes);
	*dirp = dir;
	return 0;
}

static int fs_mkreg(struct procfs_sb *sb, struct procfs_dir *parent,
                    ino_t ino, const char *name, fs_attr_mask_t mask,
                    const struct fs_attr *attr, const struct file_op *fop,
                    const struct node_op *op, struct procfs_node **nodep)
{
	return fs_mknode(sb, parent, ino, name, S_IFREG, mask, attr,
	                 fop, op, sizeof(**nodep), nodep);
}

static int procfs_node_release(struct node *node)
{
	struct procfs_node *procfs_node = (struct procfs_node*)node;
	free(procfs_node->name);
	node_free(procfs_node->parent);
	return 0;
}

static ssize_t self_readlink(struct node *node, struct uio *uio)
{
	(void)node;
	char name[64];
	snprintf(name, sizeof(name), "%" PRId32, curcpu()->thread->proc->pid);
	return uio_copyin(uio, name, strlen(name));
}

static int root_lookup(struct node *node, const char *name, size_t name_len,
                       struct node **childp)
{
	struct procfs_dir *dir = (struct procfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_LOOKUP(node, dir->node.parent, name, name_len, childp);
	if (name_len == 4 && !memcmp(name, "self", name_len))
	{
		*childp = &g_procfs.self->node;
		node_ref(*childp);
		return 0;
	}
	struct thread *thread;
	spinlock_lock(&g_thread_list_lock);
	TAILQ_FOREACH(thread, &g_thread_list, chain)
	{
		char tid[64];
		snprintf(tid, sizeof(tid), "%" PRIu32, thread->tid);
		if (strlen(tid) != name_len
		 || strncmp(name, tid, name_len))
			continue;
		ino_t ino = TID_INO(thread->tid, TID_DIR);
		node_cache_lock(&node->sb->node_cache);
		*childp = node_cache_find(&node->sb->node_cache, ino);
		if (*childp)
		{
			spinlock_unlock(&g_thread_list_lock);
			node_cache_unlock(&node->sb->node_cache);
			return 0;
		}
		struct procfs_dir *thread_node;
		fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
		struct fs_attr attr;
		attr.uid = thread->proc->cred.euid;
		attr.gid = thread->proc->cred.egid;
		attr.mode = 0555;
		int ret = fs_mkdir(node->sb->private, dir, ino, tid, mask, &attr,
		                   &tid_fop, &tid_op, &thread_node);
		if (ret)
		{
			spinlock_unlock(&g_thread_list_lock);
			return ret;
		}
		thread_node->node.node.fop = &tid_fop;
		thread_node->node.node.op = &tid_op;
		ret = node_cache_add(&node->sb->node_cache, &thread_node->node.node);
		node_cache_unlock(&node->sb->node_cache);
		if (ret)
		{
			spinlock_unlock(&g_thread_list_lock);
			node_free(&thread_node->node.node);
			return ret;
		}
		spinlock_unlock(&g_thread_list_lock);
		*childp = &thread_node->node.node;
		return 0;
	}
	spinlock_unlock(&g_thread_list_lock);
	return -ENOENT;
}

static int root_readdir(struct node *node, struct fs_readdir_ctx *ctx)
{
	int written = 0;
	struct procfs_dir *dir = (struct procfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_READDIR(node, dir->node.parent, ctx, written);
	int res;
	if (ctx->off == 2)
	{
		res = ctx->fn(ctx, "self", 4, 2, ROOT_INO(ROOT_SELF), DT_LNK);
		if (res)
			return written;
		written++;
		ctx->off++;
	}
	size_t i = 3;
	struct proc *proc;
	spinlock_lock(&g_proc_list_lock);
	TAILQ_FOREACH(proc, &g_proc_list, chain)
	{
		if (i == (size_t)ctx->off)
		{
			char name[64];
			snprintf(name, sizeof(name), "%" PRIu32, proc->pid);
			res = ctx->fn(ctx, name, strlen(name), 3,
			              TID_INO(proc->pid, TID_DIR), DT_DIR);
			if (res)
				break;
			written++;
			ctx->off++;
		}
		i++;
	}
	spinlock_unlock(&g_proc_list_lock);
	return written;
}

static int fetch_reg(struct procfs_dir *dir, const char *name, ino_t ino,
                     mode_t mode, uid_t uid, gid_t gid,
                     const struct file_op *fop, struct node **childp)
{
	node_cache_lock(&dir->node.node.sb->node_cache);
	*childp = node_cache_find(&dir->node.node.sb->node_cache, ino);
	if (*childp)
	{
		node_cache_unlock(&dir->node.node.sb->node_cache);
		return 0;
	}
	struct procfs_node *name_node;
	fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	struct fs_attr attr;
	attr.uid = uid;
	attr.gid = gid;
	attr.mode = mode;
	int ret = fs_mkreg(dir->node.node.sb->private, dir, ino, name, mask,
	                   &attr, fop, &reg_op, &name_node);
	if (ret)
		return ret;
	ret = node_cache_add(&dir->node.node.sb->node_cache, &name_node->node);
	node_cache_unlock(&dir->node.node.sb->node_cache);
	if (ret)
	{
		node_free(&name_node->node);
		return ret;
	}
	*childp = &name_node->node;
	return 0;
}

static int tid_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **childp)
{
	struct procfs_dir *dir = (struct procfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_LOOKUP(node, dir->node.parent, name, name_len, childp);
	pid_t tid = TID_MASK(node->ino);
	struct thread *thread = getthread(tid);
	if (!thread)
		return -ENOENT;
	int ret;
	if (name_len == 4 && !memcmp(name, "name", name_len))
	{
		ret = fetch_reg(dir, "name", TID_INO(tid, TID_NAME), 0444,
		                 thread->proc->cred.euid, thread->proc->cred.egid,
		                 &tid_name_fop, childp);
		goto end;
	}
	if (name_len == 4 && !memcmp(name, "maps", name_len))
	{
		ret = fetch_reg(dir, "maps", TID_INO(tid, TID_MAPS), 0400,
		                thread->proc->cred.euid, thread->proc->cred.egid,
		                &tid_maps_fop, childp);
		goto end;
	}
	if (name_len == 2 && !memcmp(name, "vm", name_len))
	{
		ret = fetch_reg(dir, "maps", TID_INO(tid, TID_MAPS), 0400,
		                thread->proc->cred.euid, thread->proc->cred.egid,
		                &tid_vm_fop, childp);
		goto end;
	}
	ret = -ENOENT;

end:
	thread_free(thread);
	return ret;
}

static int tid_readdir(struct node *node, struct fs_readdir_ctx *ctx)
{
	int written = 0;
	struct procfs_dir *dir = (struct procfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_READDIR(node, dir->node.parent, ctx, written);
	pid_t tid = TID_MASK(node->ino);
	int res;
	if (ctx->off == 2)
	{
		res = ctx->fn(ctx, "name", 4, 2, TID_INO(tid, TID_NAME), DT_REG);
		if (res)
			return written;
		written++;
		ctx->off++;
	}
	if (ctx->off == 3)
	{
		res = ctx->fn(ctx, "maps", 4, 2, TID_INO(tid, TID_MAPS), DT_REG);
		if (res)
			return written;
		written++;
		ctx->off++;
	}
	return written;
}

static ssize_t tid_name_read(struct file *file, struct uio *uio)
{
	pid_t tid = TID_MASK(file->node->ino);
	struct thread *thread = getthread(tid);
	if (!thread)
		return -ENOENT;
	size_t count = uio->count;
	off_t off = uio->off;
	ssize_t ret = uprintf(uio, "%s", thread->proc->name);
	if (ret < 0)
		goto end;
	uio->off = off + count - uio->count;
	ret = count - uio->count;

end:
	thread_free(thread);
	return ret;
}

static ssize_t tid_maps_read(struct file *file, struct uio *uio)
{
	pid_t tid = TID_MASK(file->node->ino);
	struct thread *thread = getthread(tid);
	if (!thread)
		return -ENOENT;
	size_t count = uio->count;
	off_t off = uio->off;
	struct vm_zone *zone;
	ssize_t ret;
	TAILQ_FOREACH(zone, &thread->proc->vm_space->zones, chain)
	{
		ret = uprintf(uio, "%0*zx-%0*zx %c%c%c %0*" PRIx64 " %p\n",
		              (int)(sizeof(uintptr_t) * 2), zone->addr,
		              (int)(sizeof(uintptr_t) * 2), zone->addr + zone->size,
		              (zone->prot & VM_PROT_R) ? 'r' : '-',
		              (zone->prot & VM_PROT_W) ? 'w' : '-',
		              (zone->prot & VM_PROT_X) ? 'x' : '-',
		              (int)(sizeof(uintptr_t) * 2), zone->off,
		              zone->file);
		if (ret < 0)
			goto end;
	}
	uio->off = off + count - uio->count;
	ret = count - uio->count;

end:
	thread_free(thread);
	return ret;
}

static ssize_t tid_vm_read(struct file *file, struct uio *uio)
{
	pid_t tid = TID_MASK(file->node->ino);
	struct thread *thread = getthread(tid);
	if (!thread)
		return -ENOENT;
	size_t count = uio->count;
	off_t off = uio->off;
	vm_space_print(uio, thread->proc->vm_space);
	uio->off = off + count - uio->count;
	thread_free(thread);
	return count - uio->count;
}

int procfs_init(void)
{
	int ret = fs_sb_alloc(&g_procfs_type, &g_procfs.sb);
	if (ret)
		return ret;
	g_procfs.sb->private = &g_procfs;
	g_procfs.sb->flags |= ST_NOEXEC | ST_RDONLY;
	struct procfs_dir *root;
	fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	struct fs_attr attr;
	attr.uid = 0;
	attr.gid = 0;
	attr.mode = 0555;
	ret = fs_mkdir(&g_procfs, NULL, ++g_procfs.ino, "", mask, &attr,
	               &root_fop, &root_op, &root);
	if (ret)
	{
		fs_sb_free(g_procfs.sb);
		return ret;
	}
	g_procfs.sb->root = &root->node.node;
	mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	attr.uid = 0;
	attr.gid = 0;
	attr.mode = 0555;
	ret = fs_mknode(&g_procfs, root, ROOT_INO(ROOT_SELF), "self",
	                S_IFLNK, mask, &attr, &self_fop, &self_op,
	                sizeof(struct procfs_node), &g_procfs.self);
	if (ret)
	{
		fs_sb_free(g_procfs.sb);
		return ret;
	}
	return 0;
}

static int procfs_stat(struct fs_sb *sb, struct statvfs *st)
{
	st->f_bsize = PAGE_SIZE;
	st->f_frsize = PAGE_SIZE;
	st->f_blocks = 0;
	st->f_bfree = 0;
	st->f_bavail = 0;
	st->f_files = 0;
	st->f_ffree = 0;
	st->f_favail = 0;
	st->f_fsid = 0;
	st->f_flag = sb->flags;
	st->f_namemax = 255;
	st->f_magic = SYSFS_MAGIC;
	return 0;
}

static int procfs_mount(struct node *dir, struct node *proc,
                        unsigned long flags, const void *udata,
                        struct fs_sb **sbp)
{
	(void)proc;
	(void)flags;
	(void)udata;
	struct fs_sb *sb;
	int ret = fs_sb_alloc(&g_procfs_type, &sb);
	if (ret)
		return ret;
	struct procfs_sb *procsb = malloc(sizeof(*procsb), M_ZERO);
	if (!procsb)
	{
		fs_sb_free(sb);
		return -ENOMEM;
	}
	procsb->sb = sb;
	sb->private = procsb;
	sb->flags |= ST_NOEXEC | ST_RDONLY;
	g_procfs.sb->dir = dir;
	sb->dir = dir;
	node_ref(dir);
	sb->root = g_procfs.sb->root;
	dir->mount = sb;
	*sbp = sb;
	return 0;
}
