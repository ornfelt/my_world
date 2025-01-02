#include "sysfs.h"

#include <errno.h>
#include <file.h>
#include <std.h>

struct sysfs_sb
{
	struct fs_sb *sb;
	ino_t ino;
};

struct sysfs_node
{
	struct node node;
	char *name;
	LIST_ENTRY(sysfs_node) chain;
};

struct sysfs_dir
{
	struct sysfs_node node;
	struct node *parent;
	LIST_HEAD(, sysfs_node) nodes;
};

struct sysfs_reg
{
	struct sysfs_node node;
};

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child);
static int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx);

static int sysfs_node_release(struct node *node);

static int sysfs_mount(struct node *dir, struct node *dev,
                       unsigned long flags, const void *udata, struct fs_sb **sb);
static int sysfs_stat(struct fs_sb *sb, struct statvfs *st);

static struct sysfs_sb g_sysfs;

static const struct fs_type_op fs_type_op =
{
	.mount = sysfs_mount,
	.stat = sysfs_stat,
};

struct fs_type g_sysfs_type =
{
	.op = &fs_type_op,
	.name = "sysfs",
	.flags = 0,
};

static const struct file_op dir_fop =
{
};

static const struct node_op dir_op =
{
	.release = sysfs_node_release,
	.lookup = dir_lookup,
	.readdir = dir_readdir,
	.getattr = vfs_common_getattr,
};

static const struct node_op reg_op =
{
	.release = sysfs_node_release,
	.getattr = vfs_common_getattr,
};

static int fs_mknode(struct sysfs_sb *sb, struct sysfs_dir *parent,
                     const char *name, uint32_t name_len, uid_t uid, gid_t gid,
                     mode_t mode, size_t st_size, struct sysfs_node **nodep)
{
	if (memchr(name, '/', name_len))
		return -EINVAL;
	char *namedup = strndup(name, name_len);
	if (!namedup)
		return -ENOMEM;
	struct sysfs_node *node = malloc(st_size, M_ZERO);
	if (!node)
	{
		free(namedup);
		return -ENOMEM;
	}
	node->node.sb = sb->sb;
	node->node.ino = ++g_sysfs.ino;
	node->node.attr.uid = uid;
	node->node.attr.gid = gid;
	node->node.attr.mode = mode;
	refcount_init(&node->node.refcount, 1);
	node->name = namedup;
	if (parent)
		LIST_INSERT_HEAD(&parent->nodes, node, chain);
	*nodep = node;
	return 0;
}

static int fs_mkdir(struct sysfs_sb *sb, struct sysfs_dir *parent,
                    const char *name, uint32_t name_len, mode_t mode,
                    struct sysfs_dir **dirp)
{
	struct sysfs_dir *dir;
	int ret = fs_mknode(sb, parent, name, name_len, 0, 0,
	                    (mode & 07777) | S_IFDIR, sizeof(*dir),
	                    (struct sysfs_node**)&dir);
	if (ret)
		return ret;
	dir->parent = parent ? &parent->node.node : &dir->node.node;
	dir->node.node.fop = &dir_fop;
	dir->node.node.op = &dir_op;
	LIST_INIT(&dir->nodes);
	if (dirp)
	{
		node_ref(&dir->node.node);
		*dirp = dir;
	}
	return 0;
}

static int fs_mkreg(struct sysfs_sb *sb, struct sysfs_dir *parent,
                    const char *name, uint32_t name_len, mode_t mode,
                    const struct file_op *fop, struct sysfs_reg **regp)
{
	struct sysfs_reg *reg;
	int ret = fs_mknode(sb, parent, name, name_len, 0, 0,
	                    (mode & 07777) | S_IFREG, sizeof(*reg),
	                    (struct sysfs_node**)&reg);
	if (ret)
		return ret;
	reg->node.node.fop = fop;
	reg->node.node.op = &reg_op;
	if (regp)
	{
		node_ref(&reg->node.node);
		*regp = reg;
	}
	return 0;
}

static int sysfs_node_release(struct node *node)
{
	struct sysfs_node *sysfs_node = (struct sysfs_node*)node;
	free(sysfs_node->name);
	return 0;
}

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child)
{
	struct sysfs_dir *dir = (struct sysfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_LOOKUP(node, dir->parent, name, name_len, child);
	struct sysfs_node *tmp;
	LIST_FOREACH(tmp, &dir->nodes, chain)
	{
		if (strlen(tmp->name) != name_len
		 || strncmp(name, tmp->name, name_len))
			continue;
		node_ref(&tmp->node);
		*child = &tmp->node;
		return 0;
	}
	return -ENOENT;
}

static int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx)
{
	int written = 0;
	struct sysfs_dir *dir = (struct sysfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_READDIR(node, dir->parent, ctx, written);
	int res;
	size_t i = 2;
	struct sysfs_node *tmp;
	LIST_FOREACH(tmp, &dir->nodes, chain)
	{
		if (i == (size_t)ctx->off)
		{
			struct sysfs_node *dnode = tmp;
			res = ctx->fn(ctx, dnode->name, strlen(dnode->name), i,
			              dnode->node.ino, dnode->node.attr.mode >> 12);
			if (res)
				return written;
			written++;
			ctx->off++;
		}
		i++;
	}
	return written;
}

int sysfs_mknode(const char *name, uid_t uid, gid_t gid, mode_t mode,
                 const struct file_op *fop, struct node **nodep)
{
	struct sysfs_reg *reg;
	struct sysfs_dir *dir = (struct sysfs_dir*)g_sysfs.sb->root;
	node_ref(&dir->node.node);
	while (1)
	{
		if (!*name)
		{
			node_free(&dir->node.node);
			return -EINVAL;
		}
		char *slash = strchr(name, '/');
		if (!slash)
			break;
		if (slash == name)
		{
			name++;
			continue;
		}
		struct node *nxt;
		int ret = dir_lookup(&dir->node.node, name, slash - name, &nxt);
		if (ret == -ENOENT)
		{
			ret = fs_mkdir(dir->node.node.sb->private,
			               dir, name, slash - name, 0555,
			               (struct sysfs_dir**)&nxt);
		}
		node_free(&dir->node.node);
		if (ret)
			return ret;
		if (!S_ISDIR(nxt->attr.mode))
		{
			node_free(nxt);
			return -ENOTDIR;
		}
		dir = (struct sysfs_dir*)nxt;
		name = slash + 1;
	}
	size_t name_len = strlen(name);
	struct node *test;
	int ret = dir_lookup(&dir->node.node, name, name_len, &test);
	if (!ret)
	{
		node_free(test);
		node_free(&dir->node.node);
		return -EEXIST;
	}
	if (ret != -ENOENT)
	{
		node_free(&dir->node.node);
		return ret;
	}
	ret = fs_mkreg(&g_sysfs, dir, name, name_len, mode, fop, &reg);
	node_free(&dir->node.node);
	if (ret)
		return ret;
	reg->node.node.attr.uid = uid;
	reg->node.node.attr.gid = gid;
	if (nodep)
		*nodep = &reg->node.node;
	return 0;
}

int sysfs_init(void)
{
	int ret = fs_sb_alloc(&g_sysfs_type, &g_sysfs.sb);
	if (ret)
		return ret;
	g_sysfs.sb->private = &g_sysfs;
	g_sysfs.sb->flags |= ST_NOEXEC | ST_RDONLY;
	struct sysfs_dir *root;
	ret = fs_mkdir(&g_sysfs, NULL, "", 0, 0555, &root);
	if (ret)
	{
		fs_sb_free(g_sysfs.sb);
		return ret;
	}
	g_sysfs.sb->root = &root->node.node;
	return 0;
}

static int sysfs_stat(struct fs_sb *sb, struct statvfs *st)
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

static int sysfs_mount(struct node *dir, struct node *dev,
                       unsigned long flags, const void *udata,
                       struct fs_sb **sbp)
{
	(void)flags;
	(void)udata;
	(void)dev;
	struct fs_sb *sb;
	int ret = fs_sb_alloc(&g_sysfs_type, &sb);
	if (ret)
		return ret;
	struct sysfs_sb *syssb = malloc(sizeof(*syssb), M_ZERO);
	if (!syssb)
	{
		fs_sb_free(sb);
		return -ENOMEM;
	}
	sb->private = syssb;
	syssb->sb = sb;
	sb->flags |= ST_NOEXEC | ST_RDONLY;
	g_sysfs.sb->dir = dir;
	sb->dir = dir;
	node_ref(dir);
	sb->root = g_sysfs.sb->root;
	dir->mount = sb;
	*sbp = sb;
	return 0;
}
