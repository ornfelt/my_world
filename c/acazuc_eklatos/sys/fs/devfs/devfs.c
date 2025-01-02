#include "devfs.h"

#include <ramfile.h>
#include <errno.h>
#include <queue.h>
#include <file.h>
#include <std.h>
#include <uio.h>
#include <mem.h>

struct devfs_sb
{
	struct fs_sb *sb;
	fsblkcnt_t blkcnt;
	fsfilcnt_t inocnt;
	ino_t ino;
};

struct devfs_dirent
{
	struct node *node;
	char *name;
	size_t name_len;
	LIST_ENTRY(devfs_dirent) chain;
};

struct devfs_dir
{
	struct node node;
	struct node *parent;
	LIST_HEAD(, devfs_dirent) nodes;
};

struct devfs_reg
{
	struct node node;
	struct ramfile ramfile;
};

struct devfs_lnk
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

static int reg_open(struct file *file, struct node *node);
static ssize_t reg_read(struct file *file, struct uio *uio);
static ssize_t reg_write(struct file *file, struct uio *uio);
static int reg_mmap(struct file *file, struct vm_zone *zone);
static int reg_release(struct node *node);
static int reg_fault(struct vm_zone *zone, off_t off, struct page **page);

static ssize_t lnk_readlink(struct node *node, struct uio *uio);
static int lnk_release(struct node *node);

static int devfs_node_release(struct node *node);
static int devfs_node_setattr(struct node *node, fs_attr_mask_t mask,
                              const struct fs_attr *attr);

static int devfs_mount(struct node *dir, struct node *dev,
                       unsigned long flags, const void *udata,
                       struct fs_sb **sb);
static int devfs_stat(struct fs_sb *sb, struct statvfs *st);

static struct devfs_sb g_devfs;

static const struct fs_type_op fs_type_op =
{
	.mount = devfs_mount,
	.stat = devfs_stat,
};

struct fs_type g_devfs_type =
{
	.op = &fs_type_op,
	.name = "devfs",
	.flags = 0,
};

static const struct node_op dir_op =
{
	.release = devfs_node_release,
	.lookup = dir_lookup,
	.readdir = dir_readdir,
	.mknode = dir_mknode,
	.symlink = dir_symlink,
	.getattr = vfs_common_getattr,
	.setattr = devfs_node_setattr,
};

static const struct file_op dir_fop =
{
};

static const struct node_op reg_op =
{
	.release = reg_release,
	.getattr = vfs_common_getattr,
	.setattr = devfs_node_setattr,
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
	.release = devfs_node_release,
	.getattr = vfs_common_getattr,
	.setattr = devfs_node_setattr,
};

static const struct node_op lnk_op =
{
	.readlink = lnk_readlink,
	.release = lnk_release,
	.getattr = vfs_common_getattr,
	.setattr = devfs_node_setattr,
};

static const struct file_op lnk_fop =
{
};

static const struct node_op cdev_op =
{
	.getattr = vfs_common_getattr,
	.setattr = devfs_node_setattr,
};

static const struct node_op bdev_op =
{
	.getattr = vfs_common_getattr,
	.setattr = devfs_node_setattr,
};

static const struct node_op sock_op =
{
	.release = devfs_node_release,
	.getattr = vfs_common_getattr,
	.setattr = devfs_node_setattr,
};

static int fs_mknode(struct devfs_sb *devsb, mode_t type, fs_attr_mask_t mask,
                     const struct fs_attr *attr, size_t st_size,
                     struct node **nodep)
{
	struct node *node = malloc(st_size, M_ZERO);
	if (!node)
		return -ENOMEM;
	devsb->inocnt++;
	node->sb = devsb->sb;
	node->ino = ++devsb->ino;
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

static int fs_mkdir(struct devfs_sb *sb, struct devfs_dir *parent,
                    fs_attr_mask_t mask, const struct fs_attr *attr,
                    struct devfs_dir **dirp)
{
	struct devfs_dir *dir;
	int ret = fs_mknode(sb, S_IFDIR, mask, attr, sizeof(*dir),
	                    (struct node**)&dir);
	if (ret)
		return ret;
	dir->parent = parent ? &parent->node : &dir->node;
	dir->node.fop = &dir_fop;
	dir->node.op = &dir_op;
	LIST_INIT(&dir->nodes);
	if (dirp)
	{
		node_ref(&dir->node);
		*dirp = dir;
	}
	return 0;
}

static int fs_mkreg(struct devfs_sb *sb, fs_attr_mask_t mask,
                    const struct fs_attr *attr, struct devfs_reg **regp)
{
	struct devfs_reg *reg;
	int ret = fs_mknode(sb, S_IFREG, mask, attr, sizeof(*reg),
	                    (struct node**)&reg);
	if (ret)
		return ret;
	reg->node.fop = &reg_fop;
	reg->node.op = &reg_op;
	ramfile_init(&reg->ramfile);
	if (regp)
	{
		node_ref(&reg->node);
		*regp = reg;
	}
	return 0;
}

static int fs_mkfifo(struct devfs_sb *sb, fs_attr_mask_t mask,
                     const struct fs_attr *attr, struct node **fifop)
{
	struct node *fifo;
	int ret = fs_mknode(sb, S_IFIFO, mask, attr, sizeof(*fifo),
	                    (struct node**)&fifo);
	if (ret)
		return ret;
	fifo->op = &fifo_op;
	if (fifop)
	{
		node_ref(fifo);
		*fifop = fifo;
	}
	return 0;
}

static int fs_mklnk(struct devfs_sb *sb, const char *target,
                    fs_attr_mask_t mask, const struct fs_attr *attr,
                    struct devfs_lnk **lnkp)
{
	size_t target_len = strlen(target);
	if (target_len >= MAXPATHLEN)
		return -EINVAL;
	struct devfs_lnk *lnk;
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
	if (lnkp)
	{
		node_ref(&lnk->node);
		*lnkp = lnk;
	}
	return 0;
}

static int fs_mkcdev(struct devfs_sb *sb, fs_attr_mask_t mask,
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
	if (cdevp)
	{
		node_ref(cdev);
		*cdevp = cdev;
	}
	return 0;
}

static int fs_mkbdev(struct devfs_sb *sb, fs_attr_mask_t mask,
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
	if (bdev)
	{
		node_ref(bdev);
		*bdevp = bdev;
	}
	return 0;
}

static int fs_mksock(struct devfs_sb *sb, fs_attr_mask_t mask,
                     const struct fs_attr *attr, struct node **sockp)
{
	struct node *sock;
	int ret = fs_mknode(sb, S_IFSOCK, mask, attr, sizeof(*sock),
	                    (struct node**)&sock);
	if (ret)
		return ret;
	sock->op = &sock_op;
	if (sockp)
	{
		node_ref(sock);
		*sockp = sock;
	}
	return 0;
}

static struct devfs_dirent *dirent_alloc(const char *name, size_t name_len)
{
	struct devfs_dirent *dirent = malloc(sizeof(*dirent), 0);
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

static void dirent_free(struct devfs_dirent *dirent)
{
	free(dirent->name);
	free(dirent);
}

static int devfs_node_release(struct node *node)
{
	struct devfs_sb *devsb = node->sb->private;
	devsb->inocnt--;
	return 0;
}

static void reg_resize(struct devfs_reg *reg, off_t size)
{
	if (size < reg->node.attr.size)
	{
		size_t blk_before = reg->ramfile.pages;
		ramfile_resize(&reg->ramfile, size / PAGE_SIZE);
		size_t blk_diff = reg->ramfile.pages - blk_before;
		struct devfs_sb *devsb = reg->node.sb->private;
		devsb->blkcnt -= blk_diff;
		reg->node.blocks -= blk_diff * (PAGE_SIZE / 512);
	}
	reg->node.attr.size = size;
}

static int reg_open(struct file *file, struct node *node)
{
	if (file->flags & O_TRUNC)
	{
		struct devfs_reg *reg = (struct devfs_reg*)node;
		reg_resize(reg, 0);
	}
	return 0;
}

static ssize_t read_blk(struct devfs_reg *reg, struct uio *uio, size_t n)
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
	struct devfs_reg *reg = (struct devfs_reg*)file->node;
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

static ssize_t write_page(struct devfs_reg *reg, struct uio *uio)
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
	struct devfs_sb *devsb = reg->node.sb->private;
	devsb->blkcnt += blk_diff;
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
	struct devfs_reg *reg = (struct devfs_reg*)file->node;
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
	struct devfs_reg *reg = (struct devfs_reg*)node;
	ramfile_destroy(&reg->ramfile);
	node_release(node);
	return 0;
}

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child)
{
	struct devfs_dir *dir = (struct devfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_LOOKUP(node, dir->parent, name, name_len, child);
	struct devfs_dirent *dirent;
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
	struct devfs_dir *dir = (struct devfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_READDIR(node, dir->parent, ctx, written);
	int res;
	size_t i = 2;
	struct devfs_dirent *dirent;
	LIST_FOREACH(dirent, &dir->nodes, chain)
	{
		if (i == (size_t)ctx->off)
		{
			res = ctx->fn(ctx, dirent->name, dirent->name_len,
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
	struct devfs_dir *dir = (struct devfs_dir*)node;
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
	struct devfs_dirent *dirent = dirent_alloc(name, name_len);
	if (!dirent)
		return -ENOMEM;
	switch (attr->mode & S_IFMT)
	{
		case 0:
		case S_IFREG:
			ret = fs_mkreg(node->sb->private, mask, attr,
			               (struct devfs_reg**)&dirent->node);
			break;
		case S_IFDIR:
			ret = fs_mkdir(node->sb->private, dir, mask,
			               attr, (struct devfs_dir**)&dirent->node);
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
	struct devfs_dir *dir = (struct devfs_dir*)node;
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
	struct devfs_dirent *dirent = dirent_alloc(name, name_len);
	if (!dirent)
		return -ENOMEM;
	struct devfs_sb *devsb = node->sb->private;
	ret = fs_mklnk(devsb, target, mask, attr,
	               (struct devfs_lnk**)&dirent->node);
	if (ret)
	{
		dirent_free(dirent);
		return ret;
	}
	dirent->node->nlink++;
	LIST_INSERT_HEAD(&dir->nodes, dirent, chain);
	return 0;
}

static ssize_t lnk_readlink(struct node *node, struct uio *uio)
{
	struct devfs_lnk *lnk = (struct devfs_lnk*)node;
	return uio_copyin(uio, lnk->path, lnk->node.attr.size);
}

static int lnk_release(struct node *node)
{
	struct devfs_lnk *lnk = (struct devfs_lnk*)node;
	free(lnk->path);
	node_release(node);
	return 0;
}

static int devfs_node_setattr(struct node *node, fs_attr_mask_t mask,
                              const struct fs_attr *attr)
{
	if (mask & FS_ATTR_SIZE)
	{
		if (S_ISDIR(node->attr.mode))
			return -EISDIR;
	}
	return vfs_common_setattr(node, mask, attr);
}

static int get_dir(const char **name, struct devfs_dir **dir)
{
	*dir = (struct devfs_dir*)g_devfs.sb->root;
	node_ref(&(*dir)->node);
	while (1)
	{
		if (!**name)
			return -EINVAL;
		char *slash = strchr(*name, '/');
		if (!slash)
			break;
		if (slash == *name)
		{
			(*name)++;
			continue;
		}
		struct node *nxt;
		int ret = dir_lookup(&(*dir)->node, *name, slash - *name,
		                     &nxt);
		if (ret == -ENOENT)
		{
			struct devfs_dirent *dirent = dirent_alloc(*name,
			                                           slash - *name);
			if (!dirent)
				return -ENOMEM;
			struct fs_attr attr;
			attr.mode = S_IFDIR | 0777;
			ret = fs_mkdir((*dir)->node.sb->private,
			               *dir, FS_ATTR_MODE, &attr,
			               (struct devfs_dir**)&nxt);
			if (ret)
			{
				node_free(&(*dir)->node);
				dirent_free(dirent);
				return ret;
			}
			dirent->node = nxt;
			dirent->node->nlink++;
			LIST_INSERT_HEAD(&(*dir)->nodes, dirent, chain);
		}
		node_free(&(*dir)->node);
		if (ret)
			return ret;
		if (!S_ISDIR(nxt->attr.mode))
		{
			node_free(nxt);
			return -ENOTDIR;
		}
		*dir = (struct devfs_dir*)nxt;
		*name = slash + 1;
	}
	return 0;
}

int devfs_mkcdev(const char *name, uid_t uid, gid_t gid, mode_t mode,
                 dev_t rdev, struct cdev *cdev)
{
	struct devfs_dir *dir;
	int ret = get_dir(&name, &dir);
	if (ret)
		return ret;
	size_t name_len = strlen(name);
	struct node *test;
	ret = dir_lookup(&dir->node, name, name_len, &test);
	if (!ret)
	{
		node_free(test);
		node_free(&dir->node);
		return -EEXIST;
	}
	if (ret != -ENOENT)
	{
		node_free(&dir->node);
		return ret;
	}
	struct devfs_dirent *dirent = dirent_alloc(name, name_len);
	if (!dirent)
	{
		node_free(&dir->node);
		return -ENOMEM;
	}
	fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	struct fs_attr attr;
	attr.uid = uid;
	attr.gid = gid;
	attr.mode = S_IFCHR | (mode & 07777);
	ret = fs_mkcdev(&g_devfs, mask, &attr, rdev, &dirent->node);
	node_free(&dir->node);
	if (ret)
	{
		dirent_free(dirent);
		return ret;
	}
	dirent->node->nlink++;
	dirent->node->cdev = cdev;
	LIST_INSERT_HEAD(&dir->nodes, dirent, chain);
	return 0;
}

int devfs_mkbdev(const char *name, uid_t uid, gid_t gid, mode_t mode, dev_t rdev,
                 struct bdev *bdev)
{
	struct devfs_dir *dir;
	int ret = get_dir(&name, &dir);
	if (ret)
		return ret;
	size_t name_len = strlen(name);
	struct node *test;
	ret = dir_lookup(&dir->node, name, name_len, &test);
	if (!ret)
	{
		node_free(test);
		node_free(&dir->node);
		return -EEXIST;
	}
	if (ret != -ENOENT)
	{
		node_free(&dir->node);
		return ret;
	}
	struct devfs_dirent *dirent = dirent_alloc(name, name_len);
	if (!dirent)
	{
		node_free(&dir->node);
		return -ENOMEM;
	}
	fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
	struct fs_attr attr;
	attr.uid = uid;
	attr.gid = gid;
	attr.mode = S_IFBLK | (mode & 07777);
	ret = fs_mkbdev(&g_devfs, mask, &attr, rdev, &dirent->node);
	node_free(&dir->node);
	if (ret)
	{
		dirent_free(dirent);
		return ret;
	}
	dirent->node->nlink++;
	dirent->node->bdev = bdev;
	LIST_INSERT_HEAD(&dir->nodes, dirent, chain);
	return 0;
}

int devfs_init(void)
{
	int ret = fs_sb_alloc(&g_devfs_type, &g_devfs.sb);
	if (ret)
		return ret;
	g_devfs.sb->private = &g_devfs;
	g_devfs.sb->flags |= ST_NOEXEC;
	struct devfs_dir *root;
	struct fs_attr attr;
	attr.mode = S_IFDIR | 0755;
	ret = fs_mkdir(&g_devfs, NULL, FS_ATTR_MODE, &attr, &root);
	if (ret)
	{
		fs_sb_free(g_devfs.sb);
		return ret;
	}
	g_devfs.sb->root = &root->node;
	return 0;
}

static int devfs_stat(struct fs_sb *sb, struct statvfs *st)
{
	struct devfs_sb *devsb = sb->private;
	st->f_bsize = PAGE_SIZE;
	st->f_frsize = PAGE_SIZE;
	st->f_blocks = devsb->blkcnt;
	st->f_bfree = -1;
	st->f_bavail = -1;
	st->f_files = devsb->inocnt;
	st->f_ffree = -1;
	st->f_favail = -1;
	st->f_fsid = 0;
	st->f_flag = sb->flags;
	st->f_namemax = 255;
	st->f_magic = DEVFS_MAGIC;
	return 0;
}

static int devfs_mount(struct node *dir, struct node *dev,
                       unsigned long flags, const void *udata,
                       struct fs_sb **sbp)
{
	(void)dev;
	(void)flags;
	(void)udata;
	struct fs_sb *sb;
	int ret = fs_sb_alloc(&g_devfs_type, &sb);
	if (ret)
		return ret;
	struct devfs_sb *devsb = malloc(sizeof(*devsb), M_ZERO);
	if (!devsb)
		return -ENOMEM;
	devsb->sb = sb;
	sb->private = devsb;
	sb->flags |= ST_NOEXEC;
	g_devfs.sb->dir = dir;
	sb->dir = dir;
	node_ref(dir);
	sb->root = g_devfs.sb->root;
	dir->mount = sb;
	*sbp = sb;
	return 0;
}
