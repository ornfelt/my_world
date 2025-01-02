#include "ext2.h"

#include <errno.h>
#include <disk.h>
#include <file.h>
#include <kmod.h>
#include <std.h>
#include <uio.h>
#include <vfs.h>
#include <mem.h>

static int reg_open(struct file *file, struct node *node);
static ssize_t reg_read(struct file *file, struct uio *uio);
static ssize_t reg_write(struct file *file, struct uio *uio);
static int reg_mmap(struct file *file, struct vm_zone *zone);
static int reg_fault(struct vm_zone *zone, off_t off, struct page **page);

static ssize_t lnk_readlink(struct node *node, struct uio *uio);

static int ext2fs_node_setattr(struct node *node, fs_attr_mask_t mask,
                               const struct fs_attr *attr);

static int ext2fs_mount(struct node *dir, struct node *dev, unsigned long flags,
                        const void *udata, struct fs_sb **sb);
static int ext2fs_stat(struct fs_sb *sb, struct statvfs *st);

static const struct fs_type_op fs_type_op =
{
	.mount = ext2fs_mount,
	.stat = ext2fs_stat,
};

static struct fs_type fs_type =
{
	.op = &fs_type_op,
	.name = "ext2fs",
	.flags = 0,
};

static const struct node_op dir_op =
{
	.lookup = dir_lookup,
	.readdir = dir_readdir,
	.mknode = dir_mknode,
	.getattr = vfs_common_getattr,
	.setattr = ext2fs_node_setattr,
};

static const struct file_op dir_fop =
{
};

static const struct node_op reg_op =
{
	.getattr = vfs_common_getattr,
	.setattr = ext2fs_node_setattr,
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

static const struct node_op lnk_op =
{
	.readlink = lnk_readlink,
	.getattr = vfs_common_getattr,
	.setattr = ext2fs_node_setattr,
};

static const struct file_op lnk_fop =
{
};

static const struct node_op bdev_op =
{
	.getattr = vfs_common_getattr,
	.setattr = ext2fs_node_setattr,
};

static const struct node_op cdev_op =
{
	.getattr = vfs_common_getattr,
	.setattr = ext2fs_node_setattr,
};

static const struct node_op fifo_op =
{
	.getattr = vfs_common_getattr,
	.setattr = ext2fs_node_setattr,
};

static const struct node_op sock_op =
{
	.getattr = vfs_common_getattr,
	.setattr = ext2fs_node_setattr,
};

static inline void print_sb(struct ext2_sb *sb)
{
#define PRINT_SB_FIELD(type, name) printf(#name ": %" type "\n", sb->name)
	PRINT_SB_FIELD(PRIu32, inodes_count);
	PRINT_SB_FIELD(PRIu32, blocks_count);
	PRINT_SB_FIELD(PRIu32, r_blocks_count);
	PRINT_SB_FIELD(PRIu32, free_blocks_count);
	PRINT_SB_FIELD(PRIu32, free_inodes_count);
	PRINT_SB_FIELD(PRIx16, magic);
	PRINT_SB_FIELD(PRIu16, minor_rev_level);
	PRINT_SB_FIELD(PRIu32, log_block_size);
	PRINT_SB_FIELD(PRIu32, log_frag_size);
	PRINT_SB_FIELD(PRIu32, blocks_per_group);
	PRINT_SB_FIELD(PRIu32, inodes_per_group);
	PRINT_SB_FIELD(PRIu32, first_data_block);
#undef PRINT_SB_FIELD
}

static inline void print_group_desc(struct ext2_group_desc *group_desc)
{
#define PRINT_GD_FIELD(type, name) printf(#name ": %" type "\n", group_desc->name)
	PRINT_GD_FIELD(PRIu32, block_bitmap);
	PRINT_GD_FIELD(PRIu32, inode_bitmap);
	PRINT_GD_FIELD(PRIu32, inode_table);
	PRINT_GD_FIELD(PRIu16, free_blocks_count);
	PRINT_GD_FIELD(PRIu16, free_inodes_count);
	PRINT_GD_FIELD(PRIu16, used_dirs_count);
#undef PRINT_GD_FIELD
}

static inline void print_inode(struct ext2_inode *inode)
{
#define PRINT_INODE_FIELD(type, name) printf(#name ": %" type "\n", inode->name)
	PRINT_INODE_FIELD(PRIu16, mode);
	PRINT_INODE_FIELD(PRIu16, uid);
	PRINT_INODE_FIELD(PRIu16, gid);
	PRINT_INODE_FIELD(PRIu32, size);
	PRINT_INODE_FIELD(PRIu32, atime);
	PRINT_INODE_FIELD(PRIu32, ctime);
	PRINT_INODE_FIELD(PRIu32, mtime);
	PRINT_INODE_FIELD(PRIu32, dtime);
#undef PRINT_INODE_FIELD
}

static inline void print_dirent(struct ext2_dirent *dirent)
{
#define PRINT_DIRENT_FIELD(type, name) printf(#name ": %" type "\n", dirent->name)
	PRINT_DIRENT_FIELD(PRIu32, inode);
	PRINT_DIRENT_FIELD(PRIu16, rec_len);
	PRINT_DIRENT_FIELD(PRIu8, name_len);
	PRINT_DIRENT_FIELD(PRIu8, file_type);
	PRINT_DIRENT_FIELD("s", name);
#undef PRINT_DIRENT_FIELD
}

static int read_disk_blocks(struct ext2_fs *fs, void *data, size_t size,
                            off_t off)
{
#if 0
	printf("reading blocks of 0x%lx bytes at 0x%lx to %p\n", size, off, data);
#endif
	struct iovec iov;
	struct uio uio;
	uio_fromkbuf(&uio, &iov, data, size, off);
	ssize_t ret = file_read(fs->file, &uio);
	if (ret < 0)
		return 0;
	if (ret != (ssize_t)size)
		return -ENXIO;
	return 0;
}

static int write_disk_blocks(struct ext2_fs *fs, const void *data, size_t size,
                             off_t off)
{
#if 0
	printf("writing blocks of 0x%lx bytes at 0x%lx to %p\n", size, off, data);
#endif
	struct iovec iov;
	struct uio uio;
	uio_fromkbuf(&uio, &iov, (void*)data, size, off);
	ssize_t ret = file_write(fs->file, &uio);
	if (ret < 0)
		return 0;
	if (ret != (ssize_t)size)
		return -ENXIO;
	return 0;
}

int read_disk_data(struct ext2_fs *fs, void *data, size_t count, off_t off)
{
#if 0
	printf("reading 0x%lx bytes at 0x%lx to %p\n", count, off, data);
#endif
	return read_disk_blocks(fs, data, count, off);
}

int write_disk_data(struct ext2_fs *fs, void *data, size_t count, off_t off)
{
#if 0
	printf("writing 0x%lx bytes at 0x%lx to %p\n", count, off, data);
#endif
	return write_disk_blocks(fs, data, count, off);
}

/* XXX should return a pointer to allow disk cache */
int read_block(struct ext2_fs *fs, void *data, uint32_t id)
{
	return read_disk_blocks(fs, data, fs->blksz, (off_t)id * fs->blksz);
}

int write_block(struct ext2_fs *fs, const void *data, uint32_t id)
{
	return write_disk_blocks(fs, data, fs->blksz, (off_t)id * fs->blksz);
}

int read_group_desc(struct ext2_fs *fs, uint32_t id,
                    struct ext2_group_desc *group_desc)
{
	return read_disk_data(fs, group_desc, sizeof(*group_desc),
	                      fs->bgdt_off + id * sizeof(*group_desc));
}

int write_group_desc(struct ext2_fs *fs, uint32_t id,
                     struct ext2_group_desc *group_desc)
{
	return write_disk_data(fs, group_desc, sizeof(*group_desc),
	                       fs->bgdt_off + id * sizeof(*group_desc));
}

static int bitmap_find_free(uint8_t *blk, uint32_t size, uint32_t *found)
{
	for (uint32_t i = 0; i < size / 8; i++)
	{
		if (blk[i] == 0xFF)
			continue;
		for (uint32_t j = 0; j < 8 && i * 8 + j < size; ++j)
		{
			if ((blk[i] & (1 << j)))
				continue;
			*found = i * 8 + j;
			return 0;
		}
	}
	return -ENOENT;
}


int write_sb(struct ext2_fs *fs)
{
	return write_disk_blocks(fs, &fs->ext2sb, sizeof(fs->ext2sb), 1024);
}

static int group_alloc_block(struct ext2_fs *fs, uint32_t grpid,
                             uint32_t *blkid)
{
	struct ext2_group_desc group_desc;
	int ret = read_group_desc(fs, grpid, &group_desc);
	if (ret)
		return ret;
	if (!group_desc.free_blocks_count)
	{
		*blkid = 0;
		return 0;
	}
	uint8_t blk[EXT2_MAXBLKSZ_U8];
	ret = read_block(fs, blk, group_desc.block_bitmap);
	if (ret)
		return ret;
	uint32_t found;
	ret = bitmap_find_free(blk, fs->ext2sb.blocks_per_group, &found);
	if (ret == -ENOENT)
	{
		*blkid = 0;
		return 0;
	}
	*blkid = fs->ext2sb.blocks_per_group * grpid + found;
	blk[found / 8] |= 1 << (found % 8);
	ret = write_block(fs, blk, group_desc.block_bitmap);
	if (ret)
		return ret;
	group_desc.free_blocks_count--;
	ret = write_group_desc(fs, grpid, &group_desc);
	if (ret)
		panic("failed to write group desc\n"); /* XXX */
	return 0;
}

int alloc_block(struct ext2_fs *fs, uint32_t *blkid)
{
	if (!fs->ext2sb.free_blocks_count)
		return -ENOMEM;
	for (size_t i = 0; i < fs->groups_count; ++i)
	{
		int ret = group_alloc_block(fs, i, blkid);
		if (ret)
			return ret;
		if (!*blkid)
			continue;
		fs->ext2sb.free_blocks_count--;
		ret = write_sb(fs);
		if (ret)
			panic("failed to write ext2 sb\n"); /* XXX */
		return 0;
	}
	return -ENOMEM;
}

int free_block(struct ext2_fs *fs, uint32_t blkid)
{
	uint32_t grpid = blkid / fs->ext2sb.blocks_per_group;
	struct ext2_group_desc group_desc;
	int ret = read_group_desc(fs, grpid, &group_desc);
	if (ret)
		return ret;
	if (group_desc.free_blocks_count >= fs->ext2sb.blocks_per_group)
		return -EINVAL; /* XXX assert */
	uint8_t blk[EXT2_MAXBLKSZ_U8];
	ret = read_block(fs, blk, group_desc.block_bitmap);
	if (ret)
		return ret;
	uint32_t idx = blkid % fs->ext2sb.blocks_per_group;
	if (!(blk[idx / 8] & (1 << (idx % 8))))
		return -EINVAL; /* XXX assert */
	blk[idx / 8] &= ~(1 << (idx % 8));
	ret = write_block(fs, blk, group_desc.block_bitmap);
	if (ret)
		return ret;
	group_desc.free_blocks_count++;
	ret = write_group_desc(fs, grpid, &group_desc);
	if (ret)
		panic("failed to write group desc\n"); /* XXX */
	return 0;
}

int alloc_block_zero(struct ext2_fs *fs, uint32_t *blkid)
{
	int ret = alloc_block(fs, blkid);
	if (ret)
		return ret;
	static const uint8_t zeros[EXT2_MAXBLKSZ_U8];
	ret = write_block(fs, zeros, *blkid);
	if (ret)
	{
		free_block(fs, *blkid);
		return ret;
	}
	return 0;
}

int group_alloc_inode(struct ext2_fs *fs, uint32_t grpid, ino_t *ino)
{
	struct ext2_group_desc group_desc;
	int ret = read_group_desc(fs, grpid, &group_desc);
	if (ret)
		return ret;
	if (!group_desc.free_inodes_count)
	{
		*ino = 0;
		return 0;
	}
	uint8_t blk[EXT2_MAXBLKSZ_U8];
	ret = read_block(fs, blk, group_desc.inode_bitmap);
	if (ret)
		return ret;
	uint32_t found;
	ret = bitmap_find_free(blk, fs->ext2sb.inodes_per_group, &found);
	if (ret == -ENOENT)
	{
		*ino = 0;
		return 0;
	}
	*ino = fs->ext2sb.inodes_per_group * grpid + found;
	blk[found / 8] |= 1 << (found % 8);
	ret = write_block(fs, blk, group_desc.inode_bitmap);
	if (ret)
		return ret;
	group_desc.free_inodes_count--;
	ret = write_group_desc(fs, grpid, &group_desc);
	if (ret)
		panic("failed to write group desc\n"); /* XXX */
	return 0;
}

static int node_attr_from_inode(struct ext2_fs *fs, struct ext2_node *node)
{
	struct ext2_inode *inode = &node->inode;
	switch (node->inode.mode & S_IFMT)
	{
		case S_IFREG:
			node->node.fop = &reg_fop;
			node->node.op = &reg_op;
			break;
		case S_IFDIR:
			node->node.fop = &dir_fop;
			node->node.op = &dir_op;
			break;
		case S_IFLNK:
			node->node.fop = &lnk_fop;
			node->node.op = &lnk_op;
			break;
		case S_IFCHR:
			node->node.op = &cdev_op;
			break;
		case S_IFBLK:
			node->node.op = &bdev_op;
			break;
		case S_IFIFO:
			node->node.op = &fifo_op;
			break;
		case S_IFSOCK:
			node->node.op = &sock_op;
			break;
		default:
			return -EINVAL;
	}
	node->node.attr.mode = inode->mode;
	node->node.attr.uid = inode->uid;
	node->node.attr.gid = inode->gid;
	node->node.nlink = inode->links_count;
	node->node.blksize = fs->blksz;
	node->node.blocks = inode->blocks;
	node->node.attr.size = inode->size;
	node->node.attr.atime.tv_sec = inode->atime;
	node->node.attr.atime.tv_nsec = 0;
	node->node.attr.ctime.tv_sec = inode->ctime;
	node->node.attr.ctime.tv_nsec = 0;
	node->node.attr.mtime.tv_sec = inode->mtime;
	node->node.attr.mtime.tv_nsec = 0;
	if (S_ISCHR(node->inode.mode) || S_ISBLK(node->inode.mode))
	{
		if (inode->block[0])
		{
			node->node.rdev = makedev((inode->block[0] >> 8) & 0xFF,
			                          (inode->block[0] >> 0) & 0xFF);
		}
		else if (inode->block[1])
		{
			node->node.rdev = makedev((inode->block[1] >> 8) & 0xFF,
			                          ((inode->block[1] >> 0) & 0xFF)
			                           | ((inode->block[1] >> 12) & ~0xFF));
		}
		else
		{
			node->node.rdev = 0;
		}
	}
	return 0;
}

static int read_node(struct ext2_fs *fs, struct ext2_node *node)
{
	struct ext2_inode *inode = &node->inode;
	int ret = read_inode(fs, node->node.ino, inode);
	if (ret)
		return ret;
	return node_attr_from_inode(fs, node);
}

int get_node(struct ext2_fs *fs, uint32_t ino, struct ext2_node **nodep)
{
	node_cache_lock(&fs->sb->node_cache);
	{
		struct node *child = node_cache_find(&fs->sb->node_cache, ino);
		if (child)
		{
			*nodep = (struct ext2_node*)child;
			node_cache_unlock(&fs->sb->node_cache);
			return 0;
		}
	}
	struct ext2_node *node = malloc(sizeof(*node), M_ZERO);
	if (!node)
	{
		node_cache_unlock(&fs->sb->node_cache);
		return -ENOMEM;
	}
	node->node.ino = ino;
	node->node.sb = fs->sb;
	refcount_init(&node->node.refcount, 1);
	int ret = read_node(fs, node);
	if (ret)
	{
		free(node);
		return ret;
	}
	*nodep = node;
	node_cache_add(&fs->sb->node_cache, &node->node);
	node_cache_unlock(&fs->sb->node_cache);
	return 0;
}

int fs_mknode(struct ext2_fs *fs, ino_t ino, fs_attr_mask_t mask,
              const struct fs_attr *attr, dev_t rdev,
              struct ext2_node **nodep)
{
	struct ext2_node *node = malloc(sizeof(*node), M_ZERO);
	if (!node)
		return -ENOMEM;
	node->node.ino = ino;
	node->node.sb = fs->sb;
	refcount_init(&node->node.refcount, 1);
	node->inode.mode = attr->mode;
	if (!(node->inode.mode & S_IFMT))
		node->inode.mode |= S_IFREG;
	node->inode.uid = (mask & FS_ATTR_UID) ? attr->uid : 0;
	node->inode.gid = (mask & FS_ATTR_GID) ? attr->gid : 0;
	node->inode.ctime = realtime_seconds();
	node->inode.mtime = realtime_seconds();
	node->inode.links_count = 1;
	if (S_ISCHR(attr->mode) || S_ISBLK(attr->mode))
	{
		uint16_t maj = major(rdev);
		uint16_t min = minor(rdev);
		node->inode.block[1] = (min & 0xFF)
		                     | ((maj & 0xFFF) << 8)
		                     | ((min & ~0xFF) << 12);
	}
	int ret = node_attr_from_inode(fs, node);
	if (ret)
	{
		free(node);
		return ret;
	}
	*nodep = node;
	return 0;
}

static int reg_open(struct file *file, struct node *node)
{
	if (file->flags & O_TRUNC)
	{
		struct ext2_node *reg = (struct ext2_node*)node;
		return node_truncate(reg, 0);
	}
	return 0;
}

static ssize_t reg_read(struct file *file, struct uio *uio)
{
	struct ext2_node *reg = (struct ext2_node*)file->node;
	return node_read(reg, uio);
}

static ssize_t reg_write(struct file *file, struct uio *uio)
{
	struct ext2_node *reg = (struct ext2_node*)file->node;
	if (file->flags & O_APPEND)
		uio->off = reg->node.attr.size;
	if (uio->off < 0)
		return -EINVAL;
	off_t max;
	if (__builtin_add_overflow(uio->off, uio->count, &max))
		return -EOVERFLOW;
	return node_write(reg, uio);
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
	struct iovec iov;
	struct uio uio;
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
	struct ext2_node *lnk = (struct ext2_node*)node;
	if (lnk->node.attr.size < 60)
		return uio_copyin(uio, lnk->inode.block, lnk->node.attr.size);
	return node_read(lnk, uio);
}

static int ext2fs_node_setattr(struct node *node, fs_attr_mask_t mask,
                                const struct fs_attr *attr)
{
	if (mask & FS_ATTR_SIZE)
	{
		/* XXX avoid double inode write */
		int ret = node_truncate((struct ext2_node*)node, attr->size);
		if (ret)
			return ret;
	}
	if (mask)
	{
		int ret = update_node_inode((struct ext2_node*)node);
		if (ret)
			return ret;
	}
	return vfs_common_setattr(node, mask, attr);
}

static int ext2fs_stat(struct fs_sb *sb, struct statvfs *st)
{
	struct ext2_fs *fs = sb->private;
	st->f_bsize = 1024 << fs->ext2sb.log_block_size;
	st->f_frsize = 1024 << fs->ext2sb.log_frag_size;
	st->f_blocks = fs->ext2sb.blocks_count;
	st->f_bfree = fs->ext2sb.free_blocks_count;
	st->f_bavail = fs->ext2sb.free_blocks_count - fs->ext2sb.r_blocks_count;
	st->f_files = fs->ext2sb.inodes_count;
	st->f_ffree = fs->ext2sb.free_inodes_count;
	st->f_favail = fs->ext2sb.free_inodes_count;
	st->f_fsid = 0;
	st->f_flag = sb->flags;
	st->f_namemax = 255;
	st->f_magic = EXT2FS_MAGIC;
	return 0;
}

static int ext2fs_mount(struct node *dir, struct node *dev,
                        unsigned long flags, const void *udata,
                        struct fs_sb **sbp)
{
	struct ext2_fs *fs = NULL;
	struct fs_sb *sb = NULL;
	ssize_t ret;

	(void)flags;
	(void)udata;
	if (!dev)
		return -EINVAL;
	ret = fs_sb_alloc(&fs_type, &sb);
	if (ret)
		return ret;
	fs = malloc(sizeof(*fs), M_ZERO);
	if (!fs)
	{
		ret = -ENOMEM;
		goto err;
	}
	fs->sb = sb;
	sb->private = fs;
	ret = file_fromnode(dev, O_RDONLY, &fs->file);
	if (ret)
		goto err;
	ret = file_open(fs->file, dev);
	if (ret)
		goto err;
	ret = read_disk_blocks(fs, &fs->ext2sb, sizeof(fs->ext2sb), 1024);
	if (ret < 0)
		goto err;
	if (fs->ext2sb.magic != 0xEF53)
	{
		printf("invalid magic\n");
		ret = -EINVAL;
		goto err;
	}
	if (fs->ext2sb.log_block_size > 3)
	{
		printf("unsupported block size: %" PRIu32 "\n",
		       fs->ext2sb.log_block_size);
		ret = -EINVAL;
		goto err;
	}
	if (!fs->ext2sb.blocks_per_group
	 || !fs->ext2sb.inodes_per_group)
	{
		printf("null blocks per group or inodes per group\n");
		ret = -EINVAL;
		goto err;
	}
	fs->blksz = 1024 << fs->ext2sb.log_block_size;
	if (fs->blksz > EXT2_MAXBLKSZ_U8)
	{
		printf("block size too big: 0x%" PRIx32 "\n", fs->blksz);
		ret = -EINVAL;
		goto err;
	}
	fs->bgdt_off = (!fs->ext2sb.log_block_size) ? 2048 : fs->blksz;
	fs->blk_per_blk = fs->blksz / sizeof(uint32_t);
	uint32_t groups_nb_blocks = (fs->ext2sb.blocks_count
	                             + fs->ext2sb.blocks_per_group - 1)
	                           / fs->ext2sb.blocks_per_group;
	uint32_t groups_nb_inodes = (fs->ext2sb.inodes_count
	                             + fs->ext2sb.inodes_per_group - 1)
	                           / fs->ext2sb.inodes_per_group;
	if (groups_nb_blocks != groups_nb_inodes)
	{
		printf("invalid groups nb: %" PRIu32 " / %" PRIu32 "\n",
		       groups_nb_blocks, groups_nb_inodes);
		ret = -EINVAL;
		goto err;
	}
	if (fs->ext2sb.blocks_per_group > fs->blksz * 8)
	{
		printf("blocks_per_group too big: 0x%" PRIx32 " / 0x%" PRIx32 "\n",
		       fs->ext2sb.blocks_per_group, fs->blksz * 8);
		ret = -EINVAL;
		goto err;
	}
	if (fs->ext2sb.inodes_per_group > fs->blksz * 8)
	{
		printf("inodes_per_group too big: 0x%" PRIx32 " / 0x%" PRIx32 "\n",
		       fs->ext2sb.inodes_per_group, fs->blksz * 8);
		ret = -EINVAL;
		goto err;
	}
	fs->groups_count = groups_nb_blocks;
	struct ext2_node *root;
	ret = get_node(fs, EXT2_ROOT_INO, &root);
	if (ret)
		goto err;

	sb->dir = dir;
	node_ref(dir);
	sb->root = &root->node;
	dir->mount = sb;
	*sbp = sb;
	return 0;

err:
	if (sb)
		fs_sb_free(sb);
	if (fs)
	{
		file_free(fs->file);
		free(fs);
	}
	return ret;
}

int init(void)
{
	vfs_register_fs_type(&fs_type);
	return 0;
}

void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "ext2fs",
	.init = init,
	.fini = fini,
};
