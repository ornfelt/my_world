#include "tarfs.h"

#include <errno.h>
#include <queue.h>
#include <file.h>
#include <vfs.h>
#include <std.h>
#include <uio.h>
#include <mem.h>

#define BLOCK_SIZE 512

struct tarfs_sb
{
	struct fs_sb *sb;
	struct file *tar;
	ino_t ino;
	fsblkcnt_t blkcnt;
	fsfilcnt_t inocnt;
};

struct tarfs_dirent
{
	struct node *node;
	char *name;
	size_t name_len;
	LIST_ENTRY(tarfs_dirent) chain;
};

struct tarfs_reg
{
	struct node node;
	off_t off;
};

struct tarfs_dir
{
	struct node node;
	struct node *parent;
	LIST_HEAD(, tarfs_dirent) nodes;
};

struct tarfs_lnk
{
	struct node node;
	char *path;
};

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child);
static int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx);

static ssize_t reg_read(struct file *file, struct uio *uio);
static int reg_mmap(struct file *file, struct vm_zone *zone);
static int reg_fault(struct vm_zone *zone, off_t off, struct page **page);

static ssize_t lnk_readlink(struct node *node, struct uio *uio);
static int lnk_release(struct node *node);

static int tarfs_mount(struct node *dir, struct node *dev,
                       unsigned long flags, const void *udata,
                       struct fs_sb **sb);
static int tarfs_stat(struct fs_sb *sb, struct statvfs *st);

struct tar_hdr
{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char type;
	char linked[100];
	union
	{
		char padding[255];
		struct
		{
			char ustar[6];
			char version[2];
			char user[32];
			char group[32];
			char devmaj[8];
			char devmin[8];
			char prefix[155];
			char ustar_padding[12];
		} ustar;
	};
};

static const struct fs_type_op fs_type_op =
{
	.mount = tarfs_mount,
	.stat = tarfs_stat,
};

struct fs_type g_tarfs_type =
{
	.op = &fs_type_op,
	.name = "ramfs",
	.flags = 0,
};

static const struct node_op dir_op =
{
	.lookup = dir_lookup,
	.readdir = dir_readdir,
	.getattr = vfs_common_getattr,
};

static const struct file_op dir_fop =
{
};

static const struct node_op reg_op =
{
	.getattr = vfs_common_getattr,
};

static const struct file_op reg_fop =
{
	.read = reg_read,
	.mmap = reg_mmap,
	.seek = vfs_common_seek,
};

static const struct vm_zone_op reg_vm_op =
{
	.fault = reg_fault,
};

static const struct node_op fifo_op =
{
	.getattr = vfs_common_getattr,
};

static const struct node_op lnk_op =
{
	.release = lnk_release,
	.readlink = lnk_readlink,
	.getattr = vfs_common_getattr,
};

static const struct file_op lnk_fop =
{
};

static const struct node_op cdev_op =
{
	.getattr = vfs_common_getattr,
};

static const struct node_op bdev_op =
{
	.getattr = vfs_common_getattr,
};

static int fs_mknode(struct tarfs_sb *sb, mode_t type, fs_attr_mask_t mask,
                     struct fs_attr *attr, size_t st_size,
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
	if (mask & FS_ATTR_SIZE)
		node->attr.size = attr->size;
	node->blksize = BLOCK_SIZE;
	node->blocks = node->attr.size / BLOCK_SIZE;
	refcount_init(&node->refcount, 1);
	*nodep = node;
	return 0;
}

static int fs_mkdir(struct tarfs_sb *sb, struct tarfs_dir *parent,
                    fs_attr_mask_t mask, struct fs_attr *attr,
                    struct tarfs_dir **dirp)
{
	struct tarfs_dir *dir;
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

static int fs_mkreg(struct tarfs_sb *sb, fs_attr_mask_t mask,
                    struct fs_attr *attr, struct tarfs_reg **regp)
{
	struct tarfs_reg *reg;
	int ret = fs_mknode(sb, S_IFREG, mask, attr, sizeof(*reg),
	                    (struct node**)&reg);
	if (ret)
		return ret;
	reg->node.fop = &reg_fop;
	reg->node.op = &reg_op;
	*regp = reg;
	return 0;
}

static int fs_mkfifo(struct tarfs_sb *sb, fs_attr_mask_t mask,
                     struct fs_attr *attr, struct node **fifop)
{
	struct node *fifo;
	int ret = fs_mknode(sb, S_IFREG, mask, attr, sizeof(*fifo),
	                    (struct node**)&fifo);
	if (ret)
		return ret;
	fifo->op = &fifo_op;
	*fifop = fifo;
	return 0;
}

static int fs_mklnk(struct tarfs_sb *sb, const char *target, size_t target_len,
                    fs_attr_mask_t mask, struct fs_attr *attr,
                    struct tarfs_lnk **lnkp)
{
	if (target_len >= MAXPATHLEN)
		return -EINVAL;
	struct tarfs_lnk *lnk;
	int ret = fs_mknode(sb, S_IFLNK, mask, attr, sizeof(*lnk),
	                    (struct node**)&lnk);
	if (ret)
		return ret;
	lnk->node.fop = &lnk_fop;
	lnk->node.op = &lnk_op;
	lnk->path = malloc(target_len + 1, 0);
	if (!lnk->path)
	{
		node_free(&lnk->node);
		return -ENOMEM;
	}
	memcpy(lnk->path, target, target_len);
	lnk->path[target_len] = '\0';
	lnk->node.attr.size = strlen(lnk->path);
	*lnkp = lnk;
	return 0;
}

static int fs_mkcdev(struct tarfs_sb *sb, fs_attr_mask_t mask,
                     struct fs_attr *attr, dev_t rdev,
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

static int fs_mkbdev(struct tarfs_sb *sb, fs_attr_mask_t mask,
                     struct fs_attr *attr, dev_t rdev,
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

static struct tarfs_dirent *mkdirent(const char *name, size_t name_len)
{
	struct tarfs_dirent *dirent = malloc(sizeof(*dirent), 0);
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

static void dirent_free(struct tarfs_dirent *dirent)
{
	free(dirent->name);
	free(dirent);
}

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child)
{
	struct tarfs_dir *dir = (struct tarfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_LOOKUP(node, dir->parent, name, name_len, child);
	struct tarfs_dirent *dirent;
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
	struct tarfs_dir *dir = (struct tarfs_dir*)node;
	VFS_HANDLE_DOT_DOTDOT_READDIR(node, dir->parent, ctx, written);
	size_t i = 2;
	struct tarfs_dirent *dirent;
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

static ssize_t reg_read(struct file *file, struct uio *uio)
{
	struct tarfs_reg *reg = (struct tarfs_reg*)file->node;
	if (uio->off < 0)
		return -EINVAL;
	if (uio->off >= reg->node.attr.size)
		return 0;
	size_t count = uio->count;
	size_t rem = reg->node.attr.size - uio->off;
	if (count > rem)
		count = rem;
	struct tarfs_sb *tarsb = file->node->sb->private;
	off_t foff;
	if (__builtin_add_overflow(reg->off, uio->off, &foff))
		return -EOVERFLOW;
	off_t tmpoff = uio->off;
	size_t tmpcount = uio->count;
	uio->off = foff;
	uio->count = count;
	ssize_t ret = file_read(tarsb->tar, uio);
	uio->off = tmpoff + (uio->off - foff);
	uio->count = tmpcount - (count - uio->count);
	return ret;
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

static ssize_t lnk_readlink(struct node *node, struct uio *uio)
{
	struct tarfs_lnk *lnk = (struct tarfs_lnk*)node;
	return uio_copyin(uio, lnk->path, node->attr.size);
}

static int lnk_release(struct node *node)
{
	struct tarfs_lnk *lnk = (struct tarfs_lnk*)node;
	node_release(node);
	free(lnk->path);
	return 0;
}

static int is_octalstr(const char *s, size_t n)
{
	for (size_t i = 0; i < n - 1; ++i)
	{
		if (s[i] < '0' || s[i] > '7')
			return 0;
	}
	if (s[n - 1] != '\0')
		return 0;
	return 1;
}

static uint32_t get_octal(char *s)
{
	uint32_t ret = 0;
	for (size_t i = 0; s[i]; ++i)
	{
		ret <<= 3;
		ret |= s[i] - '0';
	}
	return ret;
}

static int find_hardlink(struct tarfs_dir *root, const char *name,
                         size_t name_len, struct node **node)
{
	struct tarfs_dir *parent = root;
	int ret;
	const char *name_ptr = name;
	while (1)
	{
		char *dirname = memchr(name_ptr, '/', name_len);
		if (!dirname)
			break;
		size_t dirname_len = dirname - name_ptr;
		if (dirname_len == 1 && name_ptr[0] == '.')
		{
			TRACE("invalid '.' hardlink directory name");
			return -EINVAL;
		}
		if (dirname_len == 2 && name_ptr[0] == '.' && name_ptr[1] == '.')
		{
			TRACE("invalid '..' hardlink directory name");
			return -EINVAL;
		}
		struct node *child;
		ret = dir_lookup(&parent->node, name_ptr, dirname_len, &child);
		if (ret)
		{
			TRACE("%.*s not found", (int)dirname_len, name_ptr);
			return ret;
		}
		if (!S_ISDIR(child->attr.mode))
		{
			TRACE("using non-directory as hardlink path");
			return -ENOTDIR;
		}
		parent = (struct tarfs_dir*)child;
		name_ptr = dirname + 1;
		name_len -= dirname_len + 1;
		while (*name_ptr == '/')
		{
			name_ptr++;
			name_len--;
		}
	}
	if (!name_len)
	{
		TRACE("hardlink path ended with '/'");
		return -ENOTDIR;
	}
	if (name_len == 1 && name_ptr[0] == '.')
	{
		TRACE("invalid '.' hardlink file name");
		return -EINVAL;
	}
	if (name_len == 2 && name_ptr[0] == '.' && name_ptr[1] == '.')
	{
		TRACE("invalid '..' hardlink file name");
		return -EINVAL;
	}
	return dir_lookup(&parent->node, name_ptr, name_len, node);
}

static int mknode(struct tarfs_sb *sb, struct tarfs_dir *root,
                  struct tar_hdr *hdr, struct node **node)
{
	struct tarfs_dir *parent = root;
	ssize_t ret;
	uint32_t checksum = 0;
	for (size_t i = 0; i < offsetof(struct tar_hdr, checksum); ++i)
		checksum += ((uint8_t*)hdr)[i];
	checksum += 8 * ' ';
	for (size_t i = offsetof(struct tar_hdr, type); i < sizeof(struct tar_hdr); ++i)
		checksum += ((uint8_t*)hdr)[i];
	if (checksum != get_octal(hdr->checksum))
	{
		TRACE("invalid checksum for %.*s", (int)sizeof(hdr->name),
		      hdr->name);
		return -EINVAL;
	}
	if (!is_octalstr(hdr->mode, sizeof(hdr->mode))
	 || !is_octalstr(hdr->uid, sizeof(hdr->uid))
	 || !is_octalstr(hdr->gid, sizeof(hdr->gid))
	 || !is_octalstr(hdr->size, sizeof(hdr->size))
	 || !is_octalstr(hdr->mtime, sizeof(hdr->mtime))
	 || !is_octalstr(hdr->checksum, sizeof(hdr->checksum) - 1)
	 || hdr->checksum[sizeof(hdr->checksum) - 1] != ' '
	 || !hdr->name[0])
	{
		TRACE("invalid header values");
		return -EINVAL;
	}
	uint16_t devmin = 0;
	uint16_t devmaj = 0;
	char name[256];
	int hardlink = 0;
	mode_t mode;
	if (!memcmp(hdr->ustar.ustar, "ustar", 6))
	{
		if (memcmp(hdr->ustar.version, "00", 2))
		{
			TRACE("invalid ustar version: %.2s", hdr->ustar.version);
			return -EINVAL;
		}
		switch (hdr->type)
		{
			case 0:
			case '0':
			case '7':
				mode = S_IFREG;
				break;
			case '1':
				mode = S_IFREG;
				hardlink = 1;
				break;
			case '2':
				mode = S_IFLNK;
				break;
			case '3':
				mode = S_IFCHR;
				break;
			case '4':
				mode = S_IFBLK;
				break;
			case '5':
				mode = S_IFDIR;
				break;
			case '6':
				mode = S_IFIFO;
				break;
			case 'g':
			case 'x':
			case 'A': case 'B': case 'C': case 'D':
			case 'E': case 'F': case 'G': case 'H':
			case 'I': case 'J': case 'K': case 'L':
			case 'M': case 'N': case 'O': case 'P':
			case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X':
			case 'Y': case 'Z':
				TRACE("invalid header file type: %c", hdr->type);
				return -EINVAL; /* XXX */
			default:
				TRACE("invalid header file type: %c", hdr->type);
				return -EINVAL; /* XXX */
		}
		if (hdr->type == '3' || hdr->type == '4')
		{
			if (!is_octalstr(hdr->ustar.devmaj,
			                 sizeof(hdr->ustar.devmaj))
			 || !is_octalstr(hdr->ustar.devmin,
			                 sizeof(hdr->ustar.devmin)))
			{
				TRACE("invalid device id");
				return -EINVAL;
			}
			devmin = get_octal(hdr->ustar.devmin);
			devmaj = get_octal(hdr->ustar.devmaj);
		}
		size_t name_len = strnlen(hdr->name, sizeof(hdr->name));
		size_t prefix_len = strnlen(hdr->ustar.prefix,
		                           sizeof(hdr->ustar.prefix));
		memcpy(name, hdr->name, name_len);
		memcpy(name + name_len, hdr->ustar.prefix, prefix_len);
		name[name_len + prefix_len] = '\0';
	}
	else
	{
		switch (hdr->type)
		{
			case 0:
			case '0':
				mode = S_IFREG;
				break;
			case '1':
				mode = S_IFREG;
				hardlink = 1;
				break;
			case '2':
				mode = S_IFLNK;
				break;
			default:
				return -EINVAL;
		}
		size_t name_len = strnlen(hdr->name, sizeof(hdr->name));
		memcpy(name, hdr->name, name_len);
		name[name_len] = '\0';
	}
	if (name[0] == '/')
	{
		TRACE("invalid name starting with /");
		return -EINVAL;
	}
	const char *nameptr = name;
	while (1)
	{
		char *dirname = strchr(nameptr, '/');
		if (!dirname)
			break;
		size_t name_len = dirname - nameptr;
		if (name_len == 1 && nameptr[0] == '.')
		{
			TRACE("invalid '.' directory name");
			return -EINVAL;
		}
		if (name_len == 2 && nameptr[0] == '.' && nameptr[1] == '.')
		{
			TRACE("invalid '..' directory name");
			return -EINVAL;
		}
		struct node *child;
		ret = dir_lookup(&parent->node, nameptr, name_len, &child);
		if (ret)
		{
			if (ret != -ENOENT)
				return ret;
			struct tarfs_dirent *dirent = mkdirent(nameptr, name_len);
			if (!dirent)
				return -ENOMEM;
			struct tarfs_dir *nextdir;
			fs_attr_mask_t mask = FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MODE;
			struct fs_attr attr;
			attr.uid = 0;
			attr.gid = 0;
			attr.mode = S_IFDIR | 0777;
			ret = fs_mkdir(sb, parent, mask, &attr, &nextdir);
			if (ret)
			{
				dirent_free(dirent);
				return ret;
			}
			dirent->node = &nextdir->node;
			dirent->node->nlink++;
			LIST_INSERT_HEAD(&parent->nodes, dirent, chain);
			parent = nextdir;
		}
		else
		{
			if (!S_ISDIR(child->attr.mode))
			{
				TRACE("using non-directory as path");
				return -ENOTDIR;
			}
			parent = (struct tarfs_dir*)child;
		}
		nameptr = dirname + 1;
		while (*nameptr == '/')
			nameptr++;
	}
	size_t name_len = strlen(nameptr);
	if (!name_len)
	{
		if (!S_ISDIR(parent->node.attr.mode))
		{
			TRACE("non-directory name ended with '/'");
			return -ENOTDIR;
		}
		*node = &parent->node;
		return 0;
	}
	if (name_len == 1 && nameptr[0] == '.')
	{
		TRACE("invalid '.' file name");
		return -EINVAL;
	}
	if (name_len == 2 && nameptr[0] == '.' && nameptr[1] == '.')
	{
		TRACE("invalid '..' file name");
		return -EINVAL;
	}
	struct fs_attr attr;
	fs_attr_mask_t mask = FS_ATTR_MODE | FS_ATTR_UID | FS_ATTR_GID | FS_ATTR_MTIME;
	attr.mode = (get_octal(hdr->mode) & 07777) | mode;
	attr.uid = get_octal(hdr->uid);
	attr.gid = get_octal(hdr->gid);
	attr.mtime.tv_sec = get_octal(hdr->mtime);
	attr.mtime.tv_nsec = 0;
	struct tarfs_dirent *dirent = mkdirent(nameptr, name_len);
	if (!dirent)
		return -ENOMEM;
	switch (mode & S_IFMT)
	{
		case S_IFREG:
		{
			if (hardlink)
			{
				ret = find_hardlink(root, hdr->linked,
				                    strnlen(hdr->linked,
				                            sizeof(hdr->linked)),
				                    node);
				if (ret)
				{
					dirent_free(dirent);
					return ret;
				}
				if (!S_ISREG((*node)->attr.mode))
				{
					TRACE("hardlink of non-regular file");
					return -EINVAL;
				}
				node_ref(*node);
				break;
			}
			mask |= FS_ATTR_SIZE;
			attr.size = get_octal(hdr->size);
			off_t off = file_seek(sb->tar, 0, SEEK_CUR);
			if (off < 0)
				return off;
			struct tarfs_reg *reg;
			ret = fs_mkreg(sb, mask, &attr, &reg);
			if (ret)
			{
				dirent_free(dirent);
				return ret;
			}
			reg->off = off;
			*node = &reg->node;
			break;
		}
		case S_IFLNK:
		{
			struct tarfs_lnk *lnk;
			ret = fs_mklnk(sb, hdr->linked,
			               strnlen(hdr->linked, sizeof(hdr->linked)),
			               mask, &attr, &lnk);
			if (ret)
			{
				dirent_free(dirent);
				return ret;
			}
			*node = &lnk->node;
			break;
		}
		case S_IFBLK:
		{
			ret = fs_mkbdev(sb, mask, &attr,
			                makedev(devmaj, devmin), node);
			if (ret)
			{
				dirent_free(dirent);
				return ret;
			}
			break;
		}
		case S_IFCHR:
		{
			ret = fs_mkcdev(sb, mask, &attr,
			                makedev(devmaj, devmin), node);
			if (ret)
			{
				dirent_free(dirent);
				return ret;
			}
			break;
		}
		case S_IFDIR:
		{
			struct tarfs_dir *dir;
			ret = fs_mkdir(sb, parent, mask, &attr, &dir);
			if (ret)
			{
				dirent_free(dirent);
				return ret;
			}
			*node = &dir->node;
			break;
		}
		case S_IFIFO:
		{
			ret = fs_mkfifo(sb, mask, &attr, node);
			if (ret)
			{
				dirent_free(dirent);
				return ret;
			}
			break;
		}
		default:
			TRACE("unsupported file type: %x", (unsigned)mode);
			return -EINVAL;
	}
	dirent->node = *node;
	dirent->node->nlink++;
	LIST_INSERT_HEAD(&parent->nodes, dirent, chain);
	return 0;
}

static int tarfs_stat(struct fs_sb *sb, struct statvfs *st)
{
	struct tarfs_sb *tarsb = sb->private;
	st->f_bsize = BLOCK_SIZE;
	st->f_frsize = BLOCK_SIZE;
	st->f_blocks = tarsb->blkcnt;
	st->f_bfree = 0;
	st->f_bavail = 0;
	st->f_files = tarsb->inocnt;
	st->f_ffree = 0;
	st->f_favail = 0;
	st->f_fsid = 0;
	st->f_flag = sb->flags;
	st->f_namemax = 255;
	st->f_magic = TARFS_MAGIC;
	return 0;
}

static int tarfs_mount(struct node *dir, struct node *dev,
                       unsigned long flags, const void *udata,
                       struct fs_sb **sbp)
{
	struct tarfs_dir *root = NULL;
	struct tarfs_sb *tarsb = NULL;
	struct fs_sb *sb = NULL;
	ssize_t ret;

	(void)flags;
	(void)udata;
	if (!dev)
		return -EINVAL;
	node_ref(dev);
	ret = fs_sb_alloc(&g_tarfs_type, &sb);
	if (ret)
		goto err;
	tarsb = malloc(sizeof(*tarsb), M_ZERO);
	if (!tarsb)
	{
		TRACE("failed to malloc tarfs sb");
		ret = -ENOMEM;
		goto err;
	}
	sb->private = tarsb;
	tarsb->sb = sb;
	sb->flags |= ST_RDONLY;
	fs_attr_mask_t root_mask = FS_ATTR_MODE;
	struct fs_attr root_attr;
	root_attr.mode = S_IFDIR | 0777;
	ret = fs_mkdir(tarsb, NULL, root_mask, &root_attr, &root);
	if (ret)
	{
		TRACE("failed to create tarfs root");
		goto err;
	}
	ret = file_fromnode(dev, O_RDONLY, &tarsb->tar);
	if (ret)
	{
		TRACE("failed to create file from node");
		goto err;
	}
	ret = file_open(tarsb->tar, dev);
	if (ret)
	{
		TRACE("failed to open file");
		goto err;
	}
	while (1)
	{
		struct tar_hdr hdr;
		ret = file_readseq(tarsb->tar, &hdr, sizeof(hdr), tarsb->tar->off);
		if (ret < 0)
		{
			TRACE("failed to readseq: %s", strerror(ret));
			goto err;
		}
		if (!ret)
			break;
		if (ret != sizeof(hdr))
		{
			TRACE("failed to read tar header: %u", (unsigned)ret);
			ret = -EINVAL;
			goto err;
		}
		tarsb->tar->off += ret;
		int is_end = 1;
		for (size_t i = 0; i < sizeof(hdr); ++i)
		{
			if (((uint8_t*)&hdr)[i])
			{
				is_end = 0;
				break;
			}
		}
		if (is_end)
			break;
		struct node *node;
		ret = mknode(tarsb, root, &hdr, &node);
		if (ret)
		{
			TRACE("failed to create node: %d", (int)ret);
			goto err;
		}
		tarsb->blkcnt++;
		if (S_ISREG(node->attr.mode) && node->nlink == 1)
		{
			uint32_t next = node->attr.size;
			if (next & 0x1FF)
				next += BLOCK_SIZE - next % BLOCK_SIZE;
			tarsb->blkcnt += next / BLOCK_SIZE;
			if (file_seek(tarsb->tar, next, SEEK_CUR) < 0)
			{
				TRACE("failed to seek to next header");
				ret = -EINVAL;
				goto err;
			}
		}
	}
	sb->dir = dir;
	node_ref(dir);
	sb->root = &root->node;
	dir->mount = sb;
	*sbp = sb;
	return 0;

err:
	node_free(dev);
	if (tarsb)
	{
		if (tarsb->tar)
			file_free(tarsb->tar);
		free(tarsb);
	}
	if (sb)
		fs_sb_free(sb);
	free(root);
	return ret;
}
