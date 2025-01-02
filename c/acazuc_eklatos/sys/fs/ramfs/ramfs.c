#include "ramfs.h"

#include <ramfile.h>
#include <errno.h>
#include <queue.h>
#include <file.h>
#include <vfs.h>
#include <std.h>
#include <uio.h>
#include <mem.h>

struct ramfs_sb
{
	struct fs_sb *sb;
	fsblkcnt_t blkcnt;
	fsfilcnt_t inocnt;
	ino_t ino;
	/* XXX inode bitmap ? */
	/* XXX inode table ? */
};

struct ramfs_dirent
{
	struct node *node;
	char *name;
	size_t name_len;
	LIST_ENTRY(ramfs_dirent) chain;
};

struct ramfs_dir
{
	struct node node;
	struct node *parent;
	LIST_HEAD(, ramfs_dirent) nodes;
};

struct ramfs_reg
{
	struct node node;
	struct ramfile ramfile;
};

struct ramfs_lnk
{
	struct node node;
	char *path;
};

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child);
static int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx);
static int dir_mknode(struct node *node, const char *name, size_t name_len,
                      fs_attr_mask_t mask, const struct fs_attr *attr,
                      dev_t dev);
static int dir_symlink(struct node *node, const char *name, size_t name_len,
                       const char *target, fs_attr_mask_t mask,
                       const struct fs_attr *attr);
static int dir_rename(struct node *srcdir, const char *srcname,
                      struct node *dstdir, const char *dstname);
static int dir_rmdir(struct node *node, const char *name);
static int dir_unlink(struct node *node, const char *name);
static int dir_link(struct node *node, struct node *src, const char *path);

static int reg_open(struct file *file, struct node *node);
static ssize_t reg_read(struct file *file, struct uio *uio);
static ssize_t reg_write(struct file *file, struct uio *uio);
static int reg_mmap(struct file *file, struct vm_zone *zone);
static int reg_release(struct node *node);
static int reg_fault(struct vm_zone *zone, off_t off, struct page **page);

static ssize_t lnk_readlink(struct node *node, struct uio *uio);
static int lnk_release(struct node *node);

static int ramfs_node_release(struct node *node);
static int ramfs_node_setattr(struct node *node, fs_attr_mask_t mask,
                              const struct fs_attr *attr);

static int ramfs_mount(struct node *dir, struct node *dev,
                       unsigned long flags, const void *udata,
                       struct fs_sb **sb);
static int ramfs_stat(struct fs_sb *sb, struct statvfs *st);

static const struct fs_type_op fs_type_op =
{
	.mount = ramfs_mount,
	.stat = ramfs_stat,
};

struct fs_type g_ramfs_type =
{
	.op = &fs_type_op,
	.name = "ramfs",
	.flags = 0,
};

static const struct node_op dir_op =
{
	.release = ramfs_node_release,
	.lookup = dir_lookup,
	.readdir = dir_readdir,
	.mknode = dir_mknode,
	.symlink = dir_symlink,
	.rename = dir_rename,
	.rmdir = dir_rmdir,
	.unlink = dir_unlink,
	.link = dir_link,
	.getattr = vfs_common_getattr,
	.setattr = ramfs_node_setattr,
};

static const struct file_op dir_fop =
{
};

static const struct node_op reg_op =
{
	.release = reg_release,
	.getattr = vfs_common_getattr,
	.setattr = ramfs_node_setattr,
};

static const struct file_op reg_fop =
{
	.open = reg_open,
	.read = reg_read,
	.write = reg_write,
	.seek = vfs_common_seek,
	.mmap = reg_mmap,
};

static const struct vm_zone_op reg_vm_op =
{
	.fault = reg_fault,
};

static const struct node_op fifo_op =
{
	.release = ramfs_node_release,
	.getattr = vfs_common_getattr,
	.setattr = ramfs_node_setattr,
};

static const struct node_op lnk_op =
{
	.readlink = lnk_readlink,
	.release = lnk_release,
	.getattr = vfs_common_getattr,
	.setattr = ramfs_node_setattr,
};

static const struct file_op lnk_fop =
{
};

static const struct node_op cdev_op =
{
	.release = ramfs_node_release,
	.getattr = vfs_common_getattr,
	.setattr = ramfs_node_setattr,
};

static const struct node_op bdev_op =
{
	.release = ramfs_node_release,
	.getattr = vfs_common_getattr,
	.setattr = ramfs_node_setattr,
};

static const struct node_op sock_op =
{
	.release = ramfs_node_release,
	.getattr = vfs_common_getattr,
	.setattr = ramfs_node_setattr,
};

static int fs_mknode(struct ramfs_sb *sb, mode_t type, fs_attr_mask_t mask,
                     const struct fs_attr *attr, size_t st_size,
                     struct node **nodep)
{
	struct node *node = malloc(st_size, M_ZERO);
	if (!node)
		return -ENOMEM;
	sb->inocnt++;
	node->sb = sb->sb;
	node->ino = ++sb->ino;
	node->attr.mode = type | (attr->mode & 07777);
	if (mask & FS_ATTR_UID)
		node->attr.uid = attr->uid;
	if (mask & FS_ATTR_GID)
		node->attr.gid = attr->gid;
	if (mask & FS_ATTR_ATIME)
		node->attr.atime = attr->atime;
	if (mask & FS_ATTR_CTIME)
		node->attr.ctime = attr->ctime;
	if (mask & FS_ATTR_MTIME)
		node->attr.mtime = attr->mtime;
	node->blksize = PAGE_SIZE;
	refcount_init(&node->refcount, 1);
	*nodep = node;
	return 0;
}

static int fs_mkdir(struct ramfs_sb *sb, struct ramfs_dir *parent,
                    fs_attr_mask_t mask, const struct fs_attr *attr,
                    struct ramfs_dir **dirp)
{
	struct ramfs_dir *dir;
	int ret = fs_mknode(sb, S_IFDIR, mask, attr, sizeof(*dir),
	                    (struct node**)&dir);
	if (ret)
		return ret;
	dir->parent = parent ? &parent->node : &dir->node;
	dir->node.fop = &dir_fop;
	dir->node.op = &dir_op;
	LIST_INIT(&dir->nodes);
	*dirp = dir;
	return 0;
}

static int fs_mkreg(struct ramfs_sb *sb, fs_attr_mask_t mask,
                    const struct fs_attr *attr, struct ramfs_reg **regp)
{
	struct ramfs_reg *reg;
	int ret = fs_mknode(sb, S_IFREG, mask, attr, sizeof(*reg),
	                    (struct node**)&reg);
	if (ret)
		return ret;
	reg->node.fop = &reg_fop;
	reg->node.op = &reg_op;
	ramfile_init(&reg->ramfile);
	*regp = reg;
	return 0;
}

static int fs_mkfifo(struct ramfs_sb *sb, fs_attr_mask_t mask,
                     const struct fs_attr *attr, struct node **fifop)
{
	struct node *fifo;
	int ret = fs_mknode(sb, S_IFIFO, mask, attr, sizeof(*fifo),
	                    (struct node**)&fifo);
	if (ret)
		return ret;
	fifo->op = &fifo_op;
	*fifop = fifo;
	return 0;
}

static int fs_mklnk(struct ramfs_sb *sb, const char *target,
                    fs_attr_mask_t mask, const struct fs_attr *attr,
                    struct ramfs_lnk **lnkp)
{
	size_t target_len = strlen(target);
	if (target_len >= MAXPATHLEN)
		return -EINVAL;
	struct ramfs_lnk *lnk;
	int ret = fs_mknode(sb, S_IFLNK, mask, attr, sizeof(*lnk),
	                    (struct node**)&lnk);
	if (ret)
		return ret;
	lnk->node.fop = &lnk_fop;
	lnk->node.op = &lnk_op;
	lnk->path = strdup(target);
	if (!lnk->path)
	{
		node_free(&lnk->node);
		return -ENOMEM;
	}
	lnk->node.attr.size = target_len;
	*lnkp = lnk;
	return 0;
}

static int fs_mkcdev(struct ramfs_sb *sb, fs_attr_mask_t mask,
                     const struct fs_attr *attr, dev_t rdev,
                     struct node **cdevp)
{
	struct node *cdev;
	int ret = fs_mknode(sb, S_IFCHR, mask, attr, sizeof(*cdev),
	                    (struct node**)&cdev);
	if (ret)
		return ret;
	cdev->rdev = rdev;
	cdev->op = &cdev_op;
	*cdevp = cdev;
	return 0;
}

static int fs_mkbdev(struct ramfs_sb *sb, fs_attr_mask_t mask,
                     const struct fs_attr *attr, dev_t rdev,
                     struct node **bdevp)
{
	struct node *bdev;
	int ret = fs_mknode(sb, S_IFBLK, mask, attr, sizeof(*bdev),
	                    (struct node**)&bdev);
	if (ret)
		return ret;
	bdev->rdev = rdev;
	bdev->op = &bdev_op;
	*bdevp = bdev;
	return 0;
}

static int fs_mksock(struct ramfs_sb *sb, fs_attr_mask_t mask,
                     const struct fs_attr *attr, struct node **sockp)
{
	struct node *sock;
	int ret = fs_mknode(sb, S_IFSOCK, mask, attr, sizeof(*sock),
	                    (struct node**)&sock);
	if (ret)
		return ret;
	sock->op = &sock_op;
	*sockp = sock;
	return 0;
}

static struct ramfs_dirent *mkdirent(const char *name, size_t name_len)
{
	struct ramfs_dirent *dirent = malloc(sizeof(*dirent), 0);
	if (!dirent)
		return NULL;
	dirent->name = malloc(name_len + 1, 0);
	if (!dirent->name)
	{
		free(dirent);
		return NULL;
	}
	memcpy(dirent->name, name, name_len);
	dirent->name[name_len] = '\0';
	dirent->name_len = name_len;
	return dirent;
}

static void dirent_free(struct ramfs_dirent *dirent)
{
	free(dirent->name);
	free(dirent);
}

static void reg_resize(struct ramfs_reg *reg, off_t size)
{
	if (size < reg->node.attr.size)
	{
		size_t blk_before = reg->ramfile.pages;
		ramfile_resize(&reg->ramfile, size / PAGE_SIZE);
		size_t blk_diff = reg->ramfile.pages - blk_before;
		struct ramfs_sb *ramsb = reg->node.sb->private;
		ramsb->blkcnt -= blk_diff;
		reg->node.blocks -= blk_diff * (PAGE_SIZE / 512);
	}
	reg->node.attr.size = size;
}

static int reg_open(struct file *file, struct node *node)
{
	if (file->flags & O_TRUNC)
	{
		struct ramfs_reg *reg = (struct ramfs_reg*)node;
		reg_resize(reg, 0);
	}
	return 0;
}

static ssize_t read_blk(struct ramfs_reg *reg, struct uio *uio, size_t n)
{
	if (n > PAGE_SIZE)
		n = PAGE_SIZE;
	size_t pad = uio->off % PAGE_SIZE;
	if (n > PAGE_SIZE - pad)
		n = PAGE_SIZE - pad;
	struct page *page = ramfile_getpage(&reg->ramfile, uio->off / PAGE_SIZE, 0);
	if (page)
	{
		void *ptr = vm_map(page, PAGE_SIZE, VM_PROT_R);
		if (!ptr)
		{
			pm_free_page(page);
			return -ENOMEM;
		}
		ssize_t ret = uio_copyin(uio, &((uint8_t*)ptr)[pad], n);
		vm_unmap(ptr, PAGE_SIZE);
		pm_free_page(page);
		return ret;
	}
	return uio_copyz(uio, n);
}

static ssize_t reg_read(struct file *file, struct uio *uio)
{
	struct ramfs_reg *reg = (struct ramfs_reg*)file->node;
	if (uio->off < 0)
		return -EINVAL;
	if (uio->off >= reg->node.attr.size)
		return 0;
	size_t n = reg->node.attr.size - uio->off;
	if (n > uio->count)
		n = uio->count;
	size_t rd = 0;
	while (n)
	{
		ssize_t ret = read_blk(reg, uio, n);
		if (ret < 0)
			return ret;
		rd += ret;
		n -= ret;
	}
	return rd;
}

static ssize_t write_page(struct ramfs_reg *reg, struct uio *uio)
{
	size_t n = PAGE_SIZE;
	size_t pad = uio->off % PAGE_SIZE;
	if (n > PAGE_SIZE - pad)
		n = PAGE_SIZE - pad;
	size_t blk_before = reg->ramfile.pages;
	struct page *page = ramfile_getpage(&reg->ramfile, uio->off / PAGE_SIZE,
	                                    RAMFILE_ALLOC | RAMFILE_ZERO); /* XXX don't zero */
	size_t blk_diff = reg->ramfile.pages - blk_before;
	reg->node.blocks += blk_diff * (PAGE_SIZE / 512);
	struct ramfs_sb *ramsb = reg->node.sb->private;
	ramsb->blkcnt += blk_diff;
	if (!page)
		return -ENOMEM;
	void *ptr = vm_map(page, PAGE_SIZE, VM_PROT_W);
	if (!ptr)
	{
		pm_free_page(page);
		return -ENOMEM;
	}
	ssize_t ret = uio_copyout(&((uint8_t*)ptr)[pad], uio, n);
	vm_unmap(ptr, PAGE_SIZE);
	pm_free_page(page);
	return ret;
}

static ssize_t reg_write(struct file *file, struct uio *uio)
{
	struct ramfs_reg *reg = (struct ramfs_reg*)file->node;
	if (file->flags & O_APPEND)
		uio->off = reg->node.attr.size;
	if (uio->off < 0)
		return -EINVAL;
	size_t count = uio->count;
	off_t max;
	if (__builtin_add_overflow(uio->off, count, &max))
		return -EOVERFLOW;
	size_t wr = 0;
	while (uio->count)
	{
		ssize_t ret = write_page(reg, uio);
		/* XXX cleanup allocated block on error ? */
		if (ret < 0)
			return ret;
		if (uio->off > reg->node.attr.size)
			reg->node.attr.size = uio->off;
		wr += ret;
	}
	return wr;
}

static int reg_fault(struct vm_zone *zone, off_t off, struct page **page)
{
	ssize_t ret = pm_alloc_page(page);
	if (ret)
		return ret;
	void *ptr = vm_map(*page, PAGE_SIZE, VM_PROT_W);
	if (!ptr)
	{
		pm_free_page(*page);
		return -ENOMEM;
	}
	size_t size = PAGE_SIZE;
	if (size > zone->size - off)
		size = zone->size - off;
	struct uio uio;
	struct iovec iov;
	uio_fromkbuf(&uio, &iov, ptr, size, zone->off + off);
	ret = reg_read(zone->file, &uio);
	if (ret < 0)
	{
		vm_unmap(ptr, PAGE_SIZE);
		pm_free_page(*page);
		return ret;
	}
	if (ret < PAGE_SIZE)
		memset(&((uint8_t*)ptr)[ret], 0, PAGE_SIZE - ret);
	vm_unmap(ptr, PAGE_SIZE);
	return 0;
}

static int reg_mmap(struct file *file, struct vm_zone *zone)
{
	(void)file;
	zone->op = &reg_vm_op;
	return 0;
}

static int reg_release(struct node *node)
{
	struct ramfs_reg *reg = (struct ramfs_reg*)node;
	ramfile_destroy(&reg->ramfile);
	ramfs_node_release(node);
	return 0;
}

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child)
{
	struct ramfs_dir *dir = (struct ramfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_LOOKUP(node, dir->parent, name, name_len, child);
	struct ramfs_dirent *dirent;
	LIST_FOREACH(dirent, &dir->nodes, chain)
	{
		if (dirent->name_len != name_len
		 || memcmp(name, dirent->name, name_len))
			continue;
		node_ref(dirent->node);
		*child = dirent->node;
		return 0;
	}
	return -ENOENT;
}

static int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx)
{
	int written = 0;
	struct ramfs_dir *dir = (struct ramfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_READDIR(node, dir->parent, ctx, written);
	size_t i = 2;
	struct ramfs_dirent *dirent;
	LIST_FOREACH(dirent, &dir->nodes, chain)
	{
		if (i == (size_t)ctx->off)
		{
			int res = ctx->fn(ctx, dirent->name, dirent->name_len,
			                  i, dirent->node->ino,
			                  dirent->node->attr.mode >> 12);
			if (res)
				return written;
			written++;
			ctx->off++;
		}
		i++;
	}
	return written;
}

static int dir_mknode(struct node *node, const char *name, size_t name_len,
                      fs_attr_mask_t mask, const struct fs_attr *attr,
                      dev_t rdev)
{
	struct ramfs_dir *dir = (struct ramfs_dir*)node;
	struct node *child;
	if (!(mask & FS_ATTR_MODE))
		return -EINVAL;
	int ret = node_lookup(node, name, name_len, &child);
	if (!ret)
	{
		node_free(child);
		return -EEXIST;
	}
	if (ret != -ENOENT)
		return ret;
	struct ramfs_dirent *dirent = mkdirent(name, name_len);
	if (!dirent)
		return -ENOMEM;
	switch (attr->mode & S_IFMT)
	{
		case 0:
		case S_IFREG:
			ret = fs_mkreg(node->sb->private, mask, attr,
			               (struct ramfs_reg**)&dirent->node);
			break;
		case S_IFDIR:
			ret = fs_mkdir(node->sb->private, dir, mask,
			               attr, (struct ramfs_dir**)&dirent->node);
			break;
		case S_IFIFO:
			ret = fs_mkfifo(node->sb->private, mask, attr,
			                &dirent->node);
			break;
		case S_IFCHR:
			ret = fs_mkcdev(node->sb->private, mask, attr,
			                rdev, &dirent->node);
			break;
		case S_IFBLK:
			ret = fs_mkbdev(node->sb->private, mask, attr,
			                rdev, &dirent->node);
			break;
		case S_IFSOCK:
			ret = fs_mksock(node->sb->private, mask, attr,
			                &dirent->node);
			break;
		default:
			ret = -EINVAL;
			break;
	}
	if (ret)
	{
		dirent_free(dirent);
		return ret;
	}
	dirent->node->nlink++;
	LIST_INSERT_HEAD(&dir->nodes, dirent, chain);
	return 0;
}

static int dir_symlink(struct node *node, const char *name, size_t name_len,
                       const char *target, fs_attr_mask_t mask,
                       const struct fs_attr *attr)
{
	struct ramfs_dir *dir = (struct ramfs_dir*)node;
	struct node *child;
	if (!(mask & FS_ATTR_MODE))
		return -EINVAL;
	int ret = node_lookup(node, name, name_len, &child);
	if (!ret)
	{
		node_free(child);
		return -EEXIST;
	}
	if (ret != -ENOENT)
		return ret;
	struct ramfs_dirent *dirent = mkdirent(name, name_len);
	if (!dirent)
		return -ENOMEM;
	struct ramfs_sb *ramsb = node->sb->private;
	ret = fs_mklnk(ramsb, target, mask, attr,
	               (struct ramfs_lnk**)&dirent->node);
	if (ret)
	{
		dirent_free(dirent);
		return ret;
	}
	dirent->node->nlink++;
	LIST_INSERT_HEAD(&dir->nodes, dirent, chain);
	return 0;
}

static int dir_rename(struct node *srcdir, const char *srcname,
                      struct node *dstdir, const char *dstname)
{
	if (srcdir->sb != dstdir->sb)
		return -EXDEV;
	struct ramfs_dirent *srcdirent;
	struct ramfs_dirent *dstdirent;
	int ret;
	LIST_FOREACH(srcdirent, &((struct ramfs_dir*)srcdir)->nodes, chain)
	{
		if (!strcmp(srcdirent->name, srcname))
			break;
	}
	if (!srcdirent)
		return -ENOENT;
	LIST_FOREACH(dstdirent, &((struct ramfs_dir*)dstdir)->nodes, chain)
	{
		if (!strcmp(dstdirent->name, dstname))
			break;
	}
	/* XXX check for recursion (i.e mv dir into sub-dir) */
	char *newname = strdup(dstname);
	if (!newname)
		return -ENOMEM;
	if (dstdirent)
	{
		if (S_ISDIR(srcdirent->node->attr.mode))
		{
			if (!S_ISDIR(dstdirent->node->attr.mode))
			{
				ret = -ENOTDIR;
				goto end;
			}
			if (!LIST_EMPTY(&((struct ramfs_dir*)dstdirent->node)->nodes))
			{
				ret = -ENOTEMPTY;
				goto end;
			}
		}
		else if (S_ISDIR(dstdirent->node->attr.mode))
		{
			ret = -EISDIR;
			goto end;
		}
		LIST_REMOVE(dstdirent, chain);
		dstdirent->node->nlink--;
		node_free(dstdirent->node);
		/* XXX dec nlinks */
		free(dstdirent->name);
		free(dstdirent);
	}
	LIST_REMOVE(srcdirent, chain);
	LIST_INSERT_HEAD(&((struct ramfs_dir*)dstdir)->nodes,
	                 srcdirent, chain);
	if (S_ISDIR(srcdirent->node->attr.mode))
		((struct ramfs_dir*)srcdirent->node)->parent = dstdir;
	free(srcdirent->name);
	srcdirent->name = newname;
	srcdirent->name_len = strlen(newname);
	return 0;

end:
	free(newname);
	return ret;
}

static int dir_rmdir(struct node *node, const char *name)
{
	struct ramfs_dirent *dirent;
	LIST_FOREACH(dirent, &((struct ramfs_dir*)node)->nodes, chain)
	{
		if (!strcmp(dirent->name, name))
			break;
	}
	if (!dirent)
		return -ENOENT;
	if (!S_ISDIR(dirent->node->attr.mode))
		return -ENOTDIR;
	if (dirent->node->mount)
		return -EBUSY;
	if (!LIST_EMPTY(&((struct ramfs_dir*)dirent->node)->nodes))
		return -ENOTEMPTY;
	dirent->node->nlink--;
	node_free(dirent->node);
	LIST_REMOVE(dirent, chain);
	dirent_free(dirent);
	return 0;
}

static int dir_unlink(struct node *node, const char *name)
{
	struct ramfs_dirent *dirent;
	LIST_FOREACH(dirent, &((struct ramfs_dir*)node)->nodes, chain)
	{
		if (!strcmp(dirent->name, name))
			break;
	}
	if (!dirent)
		return -ENOENT;
	if (S_ISDIR(dirent->node->attr.mode))
		return -EISDIR;
	dirent->node->nlink--;
	node_free(dirent->node);
	LIST_REMOVE(dirent, chain);
	dirent_free(dirent);
	return 0;
}

static int dir_link(struct node *node, struct node *src, const char *name)
{
	struct ramfs_dirent *dirent;
	LIST_FOREACH(dirent, &((struct ramfs_dir*)node)->nodes, chain)
	{
		if (!strcmp(dirent->name, name))
			return -EEXIST;
	}
	dirent = mkdirent(name, strlen(name));
	if (!dirent)
		return -ENOMEM;
	dirent->node = src;
	dirent->node->nlink++;
	node_ref(dirent->node);
	LIST_INSERT_HEAD(&((struct ramfs_dir*)node)->nodes, dirent, chain);
	return 0;
}

static ssize_t lnk_readlink(struct node *node, struct uio *uio)
{
	struct ramfs_lnk *lnk = (struct ramfs_lnk*)node;
	return uio_copyin(uio, lnk->path, lnk->node.attr.size);
}

static int lnk_release(struct node *node)
{
	struct ramfs_lnk *lnk = (struct ramfs_lnk*)node;
	free(lnk->path);
	ramfs_node_release(node);
	return 0;
}

static int ramfs_node_release(struct node *node)
{
	struct ramfs_sb *ramsb = node->sb->private;
	ramsb->inocnt--;
	return 0;
}

static int ramfs_node_setattr(struct node *node, fs_attr_mask_t mask,
                              const struct fs_attr *attr)
{
	if (mask & FS_ATTR_SIZE)
	{
		if (S_ISDIR(node->attr.mode))
			return -EISDIR;
		if (S_ISREG(node->attr.mode))
			reg_resize((struct ramfs_reg*)node, attr->size);
	}
	return vfs_common_setattr(node, mask, attr);
}

static int ramfs_stat(struct fs_sb *sb, struct statvfs *st)
{
	struct ramfs_sb *ramsb = sb->private;
	st->f_bsize = PAGE_SIZE;
	st->f_frsize = PAGE_SIZE;
	st->f_blocks = ramsb->blkcnt;
	st->f_bfree = -1;
	st->f_bavail = -1;
	st->f_files = ramsb->inocnt;
	st->f_ffree = -1;
	st->f_favail = -1;
	st->f_fsid = 0;
	st->f_flag = sb->flags;
	st->f_namemax = 255;
	st->f_magic = RAMFS_MAGIC;
	return 0;
}

static int ramfs_mount(struct node *dir, struct node *dev, 
                       unsigned long flags, const void *udata,
                       struct fs_sb **sbp)
{
	(void)flags;
	(void)udata;
	(void)dev;
	struct fs_sb *sb;
	int ret = fs_sb_alloc(&g_ramfs_type, &sb);
	if (ret)
		return ret;
	struct ramfs_sb *ramsb = malloc(sizeof(*ramsb), M_ZERO);
	if (!ramsb)
	{
		fs_sb_free(sb);
		return -ENOMEM;
	}
	ramsb->sb = sb;
	sb->private = ramsb;
	struct ramfs_dir *root;
	struct fs_attr attr;
	attr.mode = S_IFDIR | 0755;
	ret = fs_mkdir(ramsb, NULL, FS_ATTR_MODE, &attr, &root);
	if (ret)
	{
		free(ramsb);
		return ret;
	}
	sb->dir = dir;
	node_ref(dir);
	sb->root = &root->node;
	dir->mount = sb;
	*sbp = sb;
	return 0;
}
