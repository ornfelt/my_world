#include "procfs/procfs.h"
#include "devfs/devfs.h"
#include "ramfs/ramfs.h"
#include "tarfs/tarfs.h"
#include "sysfs/sysfs.h"

#include <net/local.h>

#include <errno.h>
#include <proc.h>
#include <file.h>
#include <pipe.h>
#include <uio.h>
#include <std.h>
#include <vfs.h>
#include <sma.h>
#include <cpu.h>

#define SYMLOOP_MAX 64 /* POSIX requires at least 8
                        * linux allows 40
                        * FreeBSD allows 32
                        */

#define TAR_SYMBOL(name, suffix) _binary____build_##name##_tar_##suffix

#define TAR_DEF(tar) \
extern uint8_t TAR_SYMBOL(tar, start); \
extern uint8_t TAR_SYMBOL(tar, end);

TAR_DEF(usr);
TAR_DEF(etc);

TAILQ_HEAD(, fs_sb) g_mounts = TAILQ_HEAD_INITIALIZER(g_mounts);
TAILQ_HEAD(, fs_type) g_types = TAILQ_HEAD_INITIALIZER(g_types);

static struct sma fs_sb_sma;

struct node *g_vfs_root;

struct node vfs_root =
{
	.refcount = REFCOUNT_INITIALIZER(1),
	.attr =
	{
		.mode = S_IFDIR,
	},
};

void vfs_init_sma(void)
{
	sma_init(&fs_sb_sma, sizeof(struct fs_sb), NULL, NULL, "fs_sb");
}

static int getnode(struct node *cwd, const char *path, int flags,
                   size_t recur_count, struct node **node,
                   struct node **dir, char **end_fn);

static int resolve_symlink(struct node **prv, struct node **cwd, int flags,
                           size_t recur_count)
{
	struct node *tar_node;
	struct node *tar_dir;
	ssize_t ret;
	if (!S_ISDIR((*prv)->attr.mode))
	{
		ret = -ENOTDIR;
		goto end;
	}
	char path[MAXPATHLEN];
	struct uio uio;
	struct iovec iov;
	uio_fromkbuf(&uio, &iov, path, sizeof(path), 0);
	ret = node_readlink(*cwd, &uio);
	if (ret < 0)
		goto end;
	if ((size_t)ret >= sizeof(path))
	{
		ret = -EINVAL;
		goto end;
	}
	path[ret] = '\0';
	ret = getnode(*prv, path, flags, recur_count + 1, &tar_node, &tar_dir,
	              NULL);

end:
	node_free(*cwd);
	node_free(*prv);
	*cwd = tar_node;
	*prv = tar_dir;
	return ret;
}

static void resolve_mount(struct node **cwd)
{
	if (!*cwd || !S_ISDIR((*cwd)->attr.mode) || !(*cwd)->mount)
		return;
	struct node *mnt = (*cwd)->mount->root;
	node_ref(mnt);
	node_free(*cwd);
	*cwd = mnt;
}

static int getnode(struct node *cwd, const char *path, int flags,
                   size_t recur_count, struct node **node,
                   struct node **dir, char **end_fn)
{
	if (recur_count >= SYMLOOP_MAX)
		return -ELOOP;
	struct thread *thread = curcpu()->thread;
	if (path && *path == '/')
		cwd = thread ? thread->proc->root : g_vfs_root;
	else if (!cwd)
		cwd = thread ? thread->proc->cwd : g_vfs_root;
	if (!cwd)
		return -ENOENT;
	if (path)
	{
		while (*path == '/')
			path++;
	}
	node_ref(cwd);
	resolve_mount(&cwd);
	node_ref(cwd);
	struct node *prv;
	struct node *nxt = cwd;
	const char *prv_path = path;
	const char *nxt_path = path;
	for (;;)
	{
		prv = cwd;
		cwd = nxt;
		prv_path = path;
		path = nxt_path;
		resolve_mount(&cwd);
		if (!path)
			break;
		while (*path == '/')
			path++;
		if (!*path)
			break;
		if (cwd && S_ISLNK(cwd->attr.mode))
		{
			int ret = resolve_symlink(&prv, &cwd, flags, recur_count);
			if (ret)
				return ret;
		}
		if (!cwd)
		{
			node_free(prv);
			return -ENOENT;
		}
		if (!S_ISDIR(cwd->attr.mode))
		{
			node_free(cwd);
			node_free(prv);
			return -ENOTDIR;
		}
		const char *next_path = strchrnul(path, '/');
		size_t path_len = next_path - path;
		/* XXX vfs_getperm(cwd, R_OK) */
		int ret = node_lookup(cwd, path, path_len, &nxt);
		if (ret)
		{
			if (ret != -ENOENT)
			{
				node_free(cwd);
				node_free(prv);
				return ret;
			}
			nxt = NULL;
		}
		nxt_path = next_path;
		node_free(prv);
	}
	if (end_fn)
		*end_fn = (char*)prv_path;
	if (!(flags & VFS_NOFOLLOW) && cwd && S_ISLNK(cwd->attr.mode))
	{
		int ret = resolve_symlink(&prv, &cwd, flags, recur_count);
		if (ret)
			return ret;
		/* XXX what about end_fn ? */
	}
	if (node)
		*node = cwd;
	else if (cwd)
		node_free(cwd);
	if (dir)
		*dir = prv;
	else
		node_free(prv);
	return 0;
}

int vfs_getnode(struct node *cwd, const char *path, int flags,
                struct node **node)
{
	int ret = getnode(cwd, path, flags, 0, node, NULL, NULL);
	if (ret)
		return ret;
	if (!*node)
		return -ENOENT;
	return 0;
}

int vfs_getdir(struct node *cwd, const char *path, int flags,
               struct node **node, char **end_fn)
{
	return getnode(cwd, path, flags, 0, NULL, node, end_fn);
}

void node_free(struct node *node)
{
	if (refcount_dec(&node->refcount))
		return;
	if (node->sb && (node->flags & NODE_CACHED))
	{
		node_cache_lock(&node->sb->node_cache);
		if (refcount_get(&node->refcount))
		{
			node_cache_unlock(&node->sb->node_cache);
			return;
		}
		node_cache_remove(&node->sb->node_cache, node->ino);
		node_cache_unlock(&node->sb->node_cache);
	}
	node_release(node);
	switch (node->attr.mode & S_IFMT)
	{
		case S_IFIFO:
			if (node->pipe)
				pipe_free(node->pipe);
			break;
		default:
			break;
	}
	free(node);
}

void node_ref(struct node *node)
{
	refcount_inc(&node->refcount);
}

static void unpack_tar(struct node *root, const char *name,
                       const uint8_t *sym_start, const uint8_t *sym_end)
{
	fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	struct fs_attr attr;
	attr.uid = 0;
	attr.gid = 0;
	attr.mode = S_IFREG | 0644;
	if (node_mknode(root, name, strlen(name), mask, &attr, 0))
		panic("failed to create %s\n", name);
	struct node *node;
	if (vfs_getnode(root, name, 0, &node))
		panic("can't find %s\n", name);
	struct file *f;
	if (file_fromnode(node, O_WRONLY, &f))
		panic("failed to create file from %s node\n", name);
	if (file_open(f, node))
		panic("failed to open file %s\n", name);
	size_t n = sym_end - sym_start;
	struct uio uio;
	struct iovec iov;
	uio_fromkbuf(&uio, &iov, (void*)sym_start, n, 0);
	if (file_write(f, &uio) != (ssize_t)n)
		panic("failed to write full file\n");
	file_free(f);
	node_free(node);
}

void vfs_init(void)
{
	g_vfs_root = &vfs_root;

	vfs_register_fs_type(&g_ramfs_type);
	vfs_register_fs_type(&g_devfs_type);
	vfs_register_fs_type(&g_procfs_type);
	vfs_register_fs_type(&g_sysfs_type);
	vfs_register_fs_type(&g_tarfs_type);

	struct fs_sb *ramfs;
	if (vfs_mount(g_vfs_root, NULL, &g_ramfs_type, 0, NULL, &ramfs))
		panic("failed to mount /\n");
	TAILQ_INSERT_TAIL(&g_mounts, ramfs, chain);

	struct node *root = ramfs->root;

	fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	struct fs_attr attr;
	attr.uid = 0;
	attr.gid = 0;
	attr.mode = S_IFDIR | 0755;
	if (node_mknode(root, "dev" , 3, mask, &attr, 0))
		panic("failed to create dev dir\n");
	if (node_mknode(root, "etc" , 3, mask, &attr, 0))
		panic("failed to create etc dir\n");
	if (node_mknode(root, "usr" , 3, mask, &attr, 0))
		panic("failed to create usr dir\n");

	fs_attr_mask_t sym_mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	struct fs_attr sym_attr;
	sym_attr.uid = 0;
	sym_attr.gid = 0;
	sym_attr.mode = S_IFLNK | 0777;
	if (node_symlink(root, "bin", 3, "usr/bin", sym_mask, &sym_attr))
		panic("failed to create bin symlink\n");
	if (node_symlink(root, "lib", 3, "usr/lib", sym_mask, &sym_attr))
		panic("failed to create lib symlink\n");

	unpack_tar(root, "usr.tar", &TAR_SYMBOL(usr, start),
	           &TAR_SYMBOL(etc, end));
	unpack_tar(root, "etc.tar", &TAR_SYMBOL(etc, start),
	           &TAR_SYMBOL(etc, end));

	struct node *devnode;
	if (vfs_getnode(ramfs->root, "dev", 0, &devnode))
		panic("can't find /dev\n");
	if (vfs_mount(devnode, NULL, &g_devfs_type, 0, NULL, NULL))
		panic("failed to mount /dev\n");

	struct node *usrnode;
	struct node *usrtar;
	if (vfs_getnode(ramfs->root, "usr.tar", 0, &usrtar))
		panic("can't find /usr.tar\n");
	if (vfs_getnode(ramfs->root, "usr", 0, &usrnode))
		panic("can't find /usr\n");
	if (vfs_mount(usrnode, usrtar, &g_tarfs_type, 0, NULL, NULL))
		panic("failed to mount /usr\n");

	struct node *etcnode;
	struct node *etctar;
	if (vfs_getnode(ramfs->root, "etc.tar", 0, &etctar))
		panic("can't find /etc.tar\n");
	if (vfs_getnode(ramfs->root, "etc", 0, &etcnode))
		panic("can't find /etc\n");
	if (vfs_mount(etcnode, etctar, &g_tarfs_type, 0, NULL, NULL))
		panic("failed to mount /etc\n");
}

int vfs_mount(struct node *dir, struct node *dev,
              const struct fs_type *type, unsigned long flags,
              const void *udata, struct fs_sb **sbp)
{
	if (!S_ISDIR(dir->attr.mode))
		return -ENOTDIR;
	if (!type->op || !type->op->mount)
		return -ENOSYS;
	struct fs_sb *sb;
	int ret = type->op->mount(dir, dev, flags, udata, &sb);
	if (ret)
		return ret;
	TAILQ_INSERT_TAIL(&g_mounts, sb, chain);
	if (sbp)
		*sbp = sb;
	return 0;
}

int vfs_register_fs_type(struct fs_type *fs_type)
{
	if (!fs_type || !fs_type->name)
		return -EINVAL;
	const struct fs_type *it;
	TAILQ_FOREACH(it, &g_types, chain)
	{
		if (!strcmp(it->name, fs_type->name))
			return -EEXIST;
	}
	TAILQ_INSERT_TAIL(&g_types, fs_type, chain);
	return 0;
}

const struct fs_type *vfs_get_fs_type(const char *name)
{
	const struct fs_type *fs_type;
	TAILQ_FOREACH(fs_type, &g_types, chain)
	{
		if (!strcmp(name, fs_type->name))
			return fs_type;
	}
	return NULL;
}

mode_t vfs_getperm(struct node *node, uid_t uid, gid_t gid)
{
	uint32_t fperms = node->attr.mode & 7;
	if (node->attr.uid == uid || !uid)
		fperms |= (node->attr.mode >> 6) & 7;
	if (node->attr.gid == gid)
		fperms |= (node->attr.mode >> 3) & 7;
	return fperms;
}

off_t vfs_common_seek(struct file *file, off_t off, int whence)
{
	switch (whence)
	{
		case SEEK_SET:
			if (off < 0)
				return -EINVAL;
			file->off = off;
			return file->off;
		case SEEK_CUR:
		{
			if (off < 0 && off < -file->off)
				return -EINVAL;
			off_t nxt;
			if (__builtin_add_overflow(file->off, off, &nxt))
				return -EINVAL;
			file->off = nxt;
			return file->off;
		}
		case SEEK_END:
		{
			if (off < -file->node->attr.size)
				return -EINVAL;
			off_t nxt;
			if (__builtin_add_overflow(file->node->attr.size, off,
			                           &nxt))
				return -EINVAL;
			file->off = nxt;
			return file->off;
		}
		default:
			return -EINVAL;
	}
}

int vfs_common_getattr(struct node *node, fs_attr_mask_t mask,
                       struct fs_attr *attr)
{
	if (mask & FS_ATTR_ATIME)
		attr->atime = node->attr.atime;
	if (mask & FS_ATTR_MTIME)
		attr->mtime = node->attr.mtime;
	if (mask & FS_ATTR_CTIME)
		attr->ctime = node->attr.ctime;
	if (mask & FS_ATTR_UID)
		attr->uid = node->attr.uid;
	if (mask & FS_ATTR_GID)
		attr->gid = node->attr.gid;
	if (mask & FS_ATTR_MODE)
		attr->mode = node->attr.mode;
	if (mask & FS_ATTR_SIZE)
		attr->size = node->attr.size;
	return 0;
}

int vfs_common_setattr(struct node *node, fs_attr_mask_t mask,
                       const struct fs_attr *attr)
{
	if (mask & FS_ATTR_ATIME)
		node->attr.atime = attr->atime;
	if (mask & FS_ATTR_MTIME)
		node->attr.mtime = attr->mtime;
	if (mask & FS_ATTR_CTIME)
		node->attr.ctime = attr->ctime;
	if (mask & FS_ATTR_UID)
		node->attr.uid = attr->uid;
	if (mask & FS_ATTR_GID)
		node->attr.gid = attr->gid;
	if (mask & FS_ATTR_MODE)
		node->attr.mode = (node->attr.mode & ~07777) | (attr->mode & 07777);
	return 0;
}

int vfs_printmounts(struct uio *uio)
{
	struct fs_sb *sb;
	TAILQ_FOREACH(sb, &g_mounts, chain)
	{
		ssize_t ret = uprintf(uio, "%s\n", sb->type->name);
		if (ret < 0)
			return ret;
	}
	return 0;
}

int node_release(struct node *node)
{
	if (!node->op || !node->op->release)
		return 0;
	return node->op->release(node);
}

int node_lookup(struct node *node, const char *name, size_t name_len,
                struct node **child)
{
	if (!node->op || !node->op->lookup)
		return -ENOSYS;
	return node->op->lookup(node, name, name_len, child);
}

int node_readdir(struct node *node, struct fs_readdir_ctx *ctx)
{
	if (!node->op || !node->op->readdir)
		return -ENOSYS;
	return node->op->readdir(node, ctx);
}

int node_mknode(struct node *node, const char *name, size_t name_len,
                fs_attr_mask_t mask, struct fs_attr *attr, dev_t dev)
{
	if (!node->op || !node->op->mknode)
		return -ENOSYS;
	return node->op->mknode(node, name, name_len, mask, attr, dev);
}

int node_getattr(struct node *node, fs_attr_mask_t mask,
                 struct fs_attr *attr)
{
	if (!node->op || !node->op->getattr)
		return -ENOSYS;
	return node->op->getattr(node, mask, attr);
}

int node_setattr(struct node *node, fs_attr_mask_t mask,
                 struct fs_attr *attr)
{
	if (!node->op || !node->op->setattr)
		return -ENOSYS;
	return node->op->setattr(node, mask, attr);
}

int node_symlink(struct node *node, const char *name, size_t name_len,
                 const char *target, fs_attr_mask_t mask,
                 struct fs_attr *attr)
{
	if (!node->op || !node->op->symlink)
		return -ENOSYS;
	return node->op->symlink(node, name, name_len, target, mask, attr);
}

ssize_t node_readlink(struct node *node, struct uio *uio)
{
	if (!node->op || !node->op->readlink)
		return -ENOSYS;
	return node->op->readlink(node, uio);
}

int node_link(struct node *node, struct node *src, const char *name)
{
	if (!node->op || !node->op->link)
		return -ENOSYS;
	return node->op->link(node, src, name);
}

int node_unlink(struct node *node, const char *name)
{
	if (!node->op || !node->op->unlink)
		return -ENOSYS;
	return node->op->unlink(node, name);
}

int node_rmdir(struct node *node, const char *name)
{
	if (!node->op || !node->op->rmdir)
		return -ENOSYS;
	return node->op->rmdir(node, name);
}

int node_rename(struct node *srcdir, const char *srcname,
                struct node *dstdir, const char *dstname)
{
	if (!srcdir->op || !srcdir->op->rename)
		return -ENOSYS;
	return srcdir->op->rename(srcdir, srcname, dstdir, dstname);
}

int node_cache_init(struct node_cache *cache)
{
	mutex_init(&cache->mutex, 0);
	cache->size = 0;
	cache->entries = NULL;
	return 0;
}

void node_cache_destroy(struct node_cache *cache)
{
	mutex_destroy(&cache->mutex);
	free(cache->entries);
}

void node_cache_lock(struct node_cache *cache)
{
	mutex_lock(&cache->mutex);
}

void node_cache_unlock(struct node_cache *cache)
{
	mutex_unlock(&cache->mutex);
}

static ino_t cache_hash(ino_t ino)
{
	ino = (ino ^ (ino >> 30)) * 0xBF58476D1CE4E5B9ULL;
	ino = (ino ^ (ino >> 27)) * 0x94D049BB133111EBULL;
	return ino ^ (ino >> 31);
}

struct node *node_cache_find(struct node_cache *cache, ino_t ino)
{
	if (!cache->size)
		return NULL;
	size_t idx = cache_hash(ino) & (cache->size - 1);
	struct node *node;
	TAILQ_FOREACH(node, &cache->entries[idx], cache_chain)
	{
		if (node->ino == ino)
		{
			node_ref(node);
			return node;
		}
	}
	return NULL;
}

int node_cache_add(struct node_cache *cache, struct node *node)
{
	if (cache->count + 1 >= cache->size)
	{
		size_t new_size = cache->size * 2;
		if (new_size < 32)
			new_size = 32;
		struct node_cache_head *entries = malloc(sizeof(*entries) * new_size, 0);
		if (!entries)
			return -ENOMEM;
		for (size_t i = 0; i < new_size; ++i)
			TAILQ_INIT(&entries[i]);
		for (size_t i = 0; i < cache->size; ++i)
		{
			struct node *it = TAILQ_FIRST(&cache->entries[i]);
			while (it)
			{
				size_t idx = cache_hash(node->ino) & (new_size - 1);
				TAILQ_REMOVE(&cache->entries[i], it, cache_chain);
				TAILQ_INSERT_TAIL(&entries[idx], it, cache_chain);
				it = TAILQ_FIRST(&cache->entries[i]);
			}
		}
		cache->entries = entries;
		cache->size = new_size;
	}
	size_t idx = cache_hash(node->ino) & (cache->size - 1);
	TAILQ_INSERT_TAIL(&cache->entries[idx], node, cache_chain);
	node->flags |= NODE_CACHED;
	return 0;
}

int node_cache_remove(struct node_cache *cache, ino_t ino)
{
	if (!cache->size)
		return -ENOENT;
	size_t idx = cache_hash(ino) % cache->size;
	struct node *node;
	TAILQ_FOREACH(node, &cache->entries[idx], cache_chain)
	{
		if (node->ino == ino)
		{
			node->flags &= ~NODE_CACHED;
			TAILQ_REMOVE(&cache->entries[idx], node, cache_chain);
			return 0;
		}
	}
	return -ENOENT;
}

int fs_sb_alloc(const struct fs_type *type, struct fs_sb **sbp)
{
	struct fs_sb *sb = sma_alloc(&fs_sb_sma, M_ZERO);
	if (!sb)
		return -ENOMEM;
	node_cache_init(&sb->node_cache);
	refcount_init(&sb->refcount, 1);
	sb->type = type;
	*sbp = sb;
	return 0;
}

void fs_sb_free(struct fs_sb *sb)
{
	if (!sb)
		return;
	if (!refcount_dec(&sb->refcount))
		return;
	node_cache_destroy(&sb->node_cache);
	sma_free(&fs_sb_sma, sb);
}

void fs_sb_ref(struct fs_sb *sb)
{
	refcount_inc(&sb->refcount);
}

static ssize_t mounts_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	int ret = vfs_printmounts(uio);
	if (ret < 0)
		return ret;
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op mounts_fop =
{
	.read = mounts_read,
};

int mounts_register_sysfs(void)
{
	return sysfs_mknode("mounts", 0, 0, 0444, &mounts_fop, NULL);
}
