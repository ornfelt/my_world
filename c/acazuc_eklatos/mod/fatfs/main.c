#define ENABLE_TRACE

#include <ramfile.h>
#include <endian.h>
#include <errno.h>
#include <disk.h>
#include <file.h>
#include <kmod.h>
#include <std.h>
#include <vfs.h>
#include <mem.h>
#include <uio.h>

#define FAT_ATTR_READ_ONLY 0x01
#define FAT_ATTR_HIDDEN    0x02
#define FAT_ATTR_SYSTEM    0x04
#define FAT_ATTR_VOLUME_ID 0x08
#define FAT_ATTR_DIRECTORY 0x10
#define FAT_ATTR_ARCHIVE   0x20

struct fatbpb
{
	uint8_t jmp_boot[3];
	uint8_t oem_name[8];
	uint8_t byts_per_sec[2];
	uint8_t sec_per_clus;
	uint8_t rsvd_sec_cnt[2];
	uint8_t num_fats;
	uint8_t root_ent_cnt[2];
	uint8_t tot_sec16[2];
	uint8_t media;
	uint8_t fat_sz16[2];
	uint8_t sec_per_trk[2];
	uint8_t num_heads[2];
	uint8_t hidd_sec[4];
	uint8_t tot_sec32[4];
	union
	{
		struct
		{
			uint8_t drv_num;
			uint8_t reserved1;
			uint8_t boot_sig;
			uint8_t vol_id[4];
			uint8_t vol_lab[11];
			uint8_t fil_sys_type[8];
			uint8_t reserved[448];
			uint8_t signature_word[2];
		} ebpb12;
		struct
		{
			uint8_t fat_sz32[4];
			uint8_t ext_flags[2];
			uint8_t fs_ver[2];
			uint8_t root_clus[4];
			uint8_t fs_info[2];
			uint8_t bk_boot_sec[2];
			uint8_t reserved[12];
			uint8_t drv_num;
			uint8_t reserved1;
			uint8_t boot_sig;
			uint8_t vol_id[4];
			uint8_t vol_lab[11];
			uint8_t fil_sys_type[8];
			uint8_t reserved2[420];
			uint8_t signature_word[2];
		} ebpb32;
		struct
		{
			uint8_t reserved[474];
			uint8_t signature_word[2];
		} ebpb;
	};
};

struct fatfs_dirent
{
	uint8_t name[11];
	uint8_t attr;
	uint8_t reserved;
	uint8_t crt_time_tenth;
	uint8_t crt_time[2];
	uint8_t crt_date[2];
	uint8_t last_acc_date[2];
	uint8_t fst_clus_hi[2];
	uint8_t wrt_time[2];
	uint8_t wrt_date[2];
	uint8_t fst_clus_lo[2];
	uint8_t file_size[4];
};

struct fatfs_node
{
	struct node node;
	struct fatfs_dirent dirent;
	struct ramfile cache; /* XXX this must be somehow generalized to all the filesystems */
};

struct fatfs_sb
{
	struct fs_sb *sb;
	struct file *dev;
	struct fatbpb bpb;
	uint32_t fat_size;
	uint32_t clusters_count;
	uint32_t root_dir_sectors;
	uint32_t sector_size;
	uint8_t sector_shift;
	uint8_t sec_per_clus_shift;
	uint32_t data_sectors;
	uint32_t free_sectors;
	struct ramfile fat_cache; /* XXX really, a disk cache is a better way to do this kind of things */
	enum
	{
		FAT12,
		FAT16,
		FAT32,
	} fatsz;
};

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child);
static int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx);

static ssize_t reg_read(struct file *file, struct uio *uio);
static int reg_mmap(struct file *file, struct vm_zone *zone);
static int reg_fault(struct vm_zone *zone, off_t off, struct page **page);

static int fatfs_node_release(struct node *node);
static int fatfs_mount(struct node *dir, struct node *dev, unsigned long flags,
                        const void *udata, struct fs_sb **sb);
static int fatfs_stat(struct fs_sb *sb, struct statvfs *st);

static const struct fs_type_op fs_type_op =
{
	.mount = fatfs_mount,
	.stat = fatfs_stat,
};

static struct fs_type fs_type =
{
	.op = &fs_type_op,
	.name = "fatfs",
	.flags = 0,
};

static const struct node_op dir_op =
{
	.release = fatfs_node_release,
	.lookup = dir_lookup,
	.readdir = dir_readdir,
	.getattr = vfs_common_getattr,
};

static const struct file_op dir_fop =
{
};

static const struct node_op reg_op =
{
	.release = fatfs_node_release,
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

static uint32_t get_ino(const struct fatfs_dirent *dirent)
{
	return ((uint32_t)le16dec(dirent->fst_clus_hi) << 16)
	     | ((uint32_t)le16dec(dirent->fst_clus_lo) <<  0);
}

static mode_t get_mode(const struct fatfs_dirent *dirent)
{
	mode_t mode;

	if (dirent->attr & FAT_ATTR_DIRECTORY)
		mode = S_IFDIR;
	else
		mode = S_IFREG;
	if (dirent->attr & FAT_ATTR_READ_ONLY)
		mode |= 0555;
	else
		mode |= 0777;
	return mode;
}

static time_t get_date(const uint8_t date[2])
{
	time_t yday = date[0] & 0x1F;
	if (yday < 1)
		yday = 1;
	else if (yday > 31)
		yday = 31;
	yday--;
	time_t month = ((date[0] & 0xE0) >> 5)
	             | ((date[1] & 0x01) << 3);
	if (month < 1)
		month = 1;
	else if (month > 12)
		month = 12;
	month--;
	time_t year = ((date[1] & 0xFE) >> 1) + 80;
	for (time_t i = 0; i < month; ++i)
	{
		int mdays;
		if (i == 1)
		{
			if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
				mdays = 29;
			else
				mdays = 28;
		}
		else
		{
			static const int g_mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
			mdays = g_mdays[i];
		}
		yday += mdays;
	}
	return yday * 86400
	     + (year - 70) * 31536000
	     + ((year - 69) / 4) * 86400
	     - ((year - 1) / 100) * 86400
	     + ((year + 299) / 400) * 86400;
}

static time_t get_time(const uint8_t time[2])
{
	time_t second = (time[0] & 0x1F) * 2;
	time_t minute = ((time[0] & 0xE0) >> 5)
	              | ((time[1] & 0x07) << 3);
	time_t hour = time[1] >> 3;
	return second
	     + minute * 60
	     + hour * 3600;
}

static void get_timespec(struct node *node, const struct fatfs_dirent *dirent)
{
	time_t acc_date = get_date(dirent->last_acc_date);
	time_t wrt_date = get_date(dirent->wrt_date);
	time_t wrt_time = get_time(dirent->wrt_time);
	node->attr.atime.tv_sec = acc_date;
	node->attr.mtime.tv_sec = wrt_date + wrt_time;
}

static int fs_mknode(struct fatfs_sb *fatsb, const struct fatfs_dirent *dirent,
                     struct fatfs_node **nodep)
{
	struct fatfs_node *node = malloc(sizeof(*node), M_ZERO);
	if (!node)
	{
		TRACE("node allocation failed");
		return -ENOMEM;
	}
	memcpy(&node->dirent, dirent, sizeof(*dirent));
	ramfile_init(&node->cache);
	node->node.sb = fatsb->sb;
	node->node.ino = get_ino(dirent);
	node->node.attr.mode = get_mode(dirent);
	if (dirent->attr & FAT_ATTR_DIRECTORY)
	{
		node->node.fop = &dir_fop;
		node->node.op = &dir_op;
	}
	else
	{
		node->node.fop = &reg_fop;
		node->node.op = &reg_op;
	}
	get_timespec(&node->node, dirent);
	node->node.attr.size = le32dec(dirent->file_size);
	node->node.nlink = 1;
	node->node.blksize = fatsb->sector_size;
	node->node.blocks = node->node.attr.size;
	node->node.blocks += fatsb->sector_size - 1;
	node->node.blocks >>= fatsb->sector_shift;
	refcount_init(&node->node.refcount, 1);
	*nodep = node;
	return 0;
}

static int fatfs_node_release(struct node *node)
{
	struct fatfs_node *fatnode = (struct fatfs_node*)node;
	ramfile_destroy(&fatnode->cache);
	return 0;
}

static int get_fat_cache_page(struct fatfs_sb *fatsb, uint64_t off,
                              uint8_t **blk)
{
	struct page *page = ramfile_getpage(&fatsb->fat_cache, off / PAGE_SIZE, 0);
	if (page)
	{
		*blk = vm_map(page, PAGE_SIZE, VM_PROT_RW);
		pm_free_page(page);
		if (!*blk)
		{
			TRACE("failed to map cache page");
			return -ENOMEM;
		}
		return 0;
	}
	page = ramfile_getpage(&fatsb->fat_cache, off / PAGE_SIZE, RAMFILE_ALLOC);
	if (!page)
	{
		TRACE("failed to allocate cache page");
		return -ENOMEM;
	}
	*blk = vm_map(page, PAGE_SIZE, VM_PROT_RW);
	pm_free_page(page);
	if (!*blk)
	{
		TRACE("failed to map cache page");
		ramfile_rmpage(&fatsb->fat_cache, off / PAGE_SIZE);
		return -ENOMEM;
	}
	ssize_t ret = file_readseq(fatsb->dev, *blk, PAGE_SIZE,
	                           (le16dec(fatsb->bpb.rsvd_sec_cnt)
	                         << fatsb->sector_shift)
	                         + off - off % PAGE_SIZE);
	if (ret < 0)
	{
		TRACE("failed to read directory block: %s",
		      strerror(ret));
		vm_unmap(*blk, PAGE_SIZE);
		ramfile_rmpage(&fatsb->fat_cache, off / PAGE_SIZE);
		return ret;
	}
	if (ret < PAGE_SIZE)
		memset(&(*blk)[ret], 0, PAGE_SIZE - ret);
	return 0;
}

static int get_sector_offset(struct fatfs_sb *fatsb,
                             struct fatfs_node *node,
                             uint32_t *cluster,
                             uint32_t *offset)
{
	uint8_t *data = NULL;
	uint32_t fat_offset;
	uint32_t fat_mask;
	uint32_t fat_num;
	uint32_t fat_off;
	uint32_t fat;
	ssize_t ret;

	if (&node->node == fatsb->sb->root)
	{
		if (fatsb->fatsz == FAT32)
			*offset = le32dec(fatsb->bpb.ebpb32.root_clus) * 4;
		else
			*offset = fatsb->bpb.num_fats * le16dec(fatsb->bpb.fat_sz16);
		*offset += le16dec(fatsb->bpb.rsvd_sec_cnt);
		*offset += *cluster << fatsb->sec_per_clus_shift;
		*offset <<= fatsb->sector_shift;
		(*cluster)++;
		return 0;
	}
	if (*cluster)
	{
		switch (fatsb->fatsz)
		{
			case FAT12:
				if (*cluster >= 0xFF8)
					return -ENOENT;
				break;
			case FAT16:
				if (*cluster >= 0xFFF8)
					return -ENOENT;
				break;
			case FAT32:
				if (*cluster >= 0xFFFFFFF8)
					return -ENOENT;
				break;
			default:
				panic("broken fat size\n");
		}
	}
	else
	{
		*cluster = ((uint32_t)le16dec(node->dirent.fst_clus_hi) << 16)
		         | ((uint32_t)le16dec(node->dirent.fst_clus_lo) <<  0);
		if (!*cluster)
			return -ENOENT;
		if (*cluster == 1)
			return -EINVAL;
		if (*cluster >= fatsb->clusters_count)
			return -ENOENT;
	}
	switch (fatsb->fatsz)
	{
		case FAT12:
			fat_offset = *cluster + *cluster / 2;
			fat_num = fat_offset / PAGE_SIZE;
			fat_off = fat_offset % PAGE_SIZE;
			ret = get_fat_cache_page(fatsb, fat_num, &data);
			if (ret < 0)
			{
				TRACE("failed to read fat sector");
				goto end;
			}
			switch (fat_off)
			{
				case PAGE_SIZE - 1:
					fat = data[fat_off];
					vm_unmap(data, PAGE_SIZE);
					ret = get_fat_cache_page(fatsb, fat_num + 1, &data);
					if (ret < 0)
					{
						TRACE("failed to read fat sector");
						goto end;
					}
					fat |= (uint16_t)data[0] << 8;
					break;
				default:
					fat = le16dec(&data[fat_off]);
					break;
			}
			if (*cluster & 1)
				fat >>= 4;
			else
				fat &= 0x0FFF;
			fat_mask = 0xFFF;
			break;
		case FAT16:
			fat_offset = *cluster * 2;
			fat_num = fat_offset / PAGE_SIZE;
			fat_off = fat_offset % PAGE_SIZE;
			ret = get_fat_cache_page(fatsb, fat_num, &data);
			if (ret < 0)
			{
				TRACE("failed to read fat sector");
				goto end;
			}
			fat = le16dec(&data[fat_off]);
			fat_mask = 0xFFFF;
			break;
		case FAT32:
			fat_offset = *cluster * 4;
			fat_num = fat_offset / PAGE_SIZE;
			fat_off = fat_offset % PAGE_SIZE;
			ret = get_fat_cache_page(fatsb, fat_num, &data);
			if (ret < 0)
			{
				TRACE("failed to read fat sector");
				goto end;
			}
			fat = le32dec(&data[fat_off]);
			fat_mask = 0x0FFFFFFF;
			break;
		default:
			panic("broken fat size\n");
	}
	if (!fat)
	{
		ret = -ENOENT;
		goto end;
	}
	if (fat == 1)
	{
		ret = -EINVAL;
		goto end;
	}
	if (fat >= fatsb->clusters_count
	 && fat < fat_mask - 7)
	{
		ret = -EINVAL;
		goto end;
	}
	*offset = (*cluster - 2) << fatsb->sec_per_clus_shift;
	*cluster = fat;
	*offset += fatsb->root_dir_sectors;
	*offset += fatsb->bpb.num_fats * fatsb->fat_size;
	*offset += le16dec(fatsb->bpb.rsvd_sec_cnt);
	*offset <<= fatsb->sector_shift;
	ret = 0;

end:
	if (data)
		vm_unmap(data, PAGE_SIZE);
	return ret;
}

static ssize_t node_read(struct fatfs_node *node, struct uio *uio)
{
	struct fatfs_sb *fatsb = node->node.sb->private;
	uint8_t data[4096];
	uint32_t cluster = 0;
	uint32_t offset = 0;
	ssize_t ret;
	ssize_t rd = 0;
	size_t len;

	while (1)
	{
		if (!uio->count)
			break;
		ret = get_sector_offset(fatsb, node, &cluster, &offset);
		if (ret == -ENOENT)
			break;
		if (ret)
			return ret;
		for (size_t sec = 0; sec < fatsb->bpb.sec_per_clus; ++sec)
		{
			if (!uio->count)
				break;
			if (uio->off >= fatsb->sector_size)
			{
				uio->off -= fatsb->sector_size;
				offset += fatsb->sector_size;
				continue;
			}
			ret = file_readseq(fatsb->dev, data,
			                   fatsb->sector_size,
			                   offset);
			if (ret < 0)
			{
				TRACE("failed to read sector");
				return ret;
			}
			if (ret != fatsb->sector_size)
			{
				TRACE("failed to read full sector");
				return -EINVAL;
			}
			len = fatsb->sector_size - uio->off;
			if (len > uio->count)
				len = uio->count;
			uio_copyin(uio, &data[uio->off], len);
			rd += len;
			offset += fatsb->sector_size;
		}
	}
	return rd;
}

static int get_node_cache_page(struct fatfs_node *node, uint64_t off,
                               uint8_t **blk)
{
	struct page *page = ramfile_getpage(&node->cache, off / PAGE_SIZE, 0);
	if (page)
	{
		*blk = vm_map(page, PAGE_SIZE, VM_PROT_RW);
		pm_free_page(page);
		if (!*blk)
		{
			TRACE("failed to map cache page");
			return -ENOMEM;
		}
		return 0;
	}
	page = ramfile_getpage(&node->cache, off / PAGE_SIZE, RAMFILE_ALLOC);
	if (!page)
	{
		TRACE("failed to allocate cache page");
		return -ENOMEM;
	}
	*blk = vm_map(page, PAGE_SIZE, VM_PROT_RW);
	pm_free_page(page);
	if (!*blk)
	{
		TRACE("failed to map cache page");
		ramfile_rmpage(&node->cache, off / PAGE_SIZE);
		return -ENOMEM;
	}
	struct uio uio;
	struct iovec iov;
	uio_fromkbuf(&uio, &iov, *blk, PAGE_SIZE, off);
	ssize_t ret = node_read(node, &uio);
	if (ret < 0)
	{
		TRACE("failed to read directory block: %s",
		      strerror(ret));
		vm_unmap(*blk, PAGE_SIZE);
		ramfile_rmpage(&node->cache, off / PAGE_SIZE);
		return ret;
	}
	if (ret < PAGE_SIZE)
		memset(&(*blk)[ret], 0, PAGE_SIZE - ret);
	return 0;
}

static int dir_iterate(struct fatfs_node *dir,
                       int (*cb)(const struct fatfs_dirent *dirent, void *userdata),
                       void *userdata)
{
	for (size_t off = 0; ; off += PAGE_SIZE)
	{
		uint8_t *blk;
		int ret = get_node_cache_page(dir, off, &blk);
		if (ret)
			return ret;
		for (size_t i = 0; i < PAGE_SIZE; i += sizeof(struct fatfs_dirent))
		{
			struct fatfs_dirent *dirent = (struct fatfs_dirent*)&blk[i];
			if (!dirent->name[0])
			{
				vm_unmap(blk, PAGE_SIZE);
				return 0;
			}
			if (cb(dirent, userdata))
			{
				vm_unmap(blk, PAGE_SIZE);
				return 0;
			}
		}
		vm_unmap(blk, PAGE_SIZE);
	}
	return 0;
}

static void get_fixed_name(char *name, size_t *name_len,
                           const struct fatfs_dirent *dirent)
{
	for (size_t i = 0; i < 8; ++i)
	{
		if (dirent->name[i] == ' ')
			break;
		name[(*name_len)++] = dirent->name[i];
	}
	if (dirent->name[8] != ' ')
	{
		name[(*name_len)++] = '.';
		for (size_t i = 8; i < 11; ++i)
		{
			if (dirent->name[i] == ' ')
				break;
			name[(*name_len)++] = dirent->name[i];
		}
	}
	name[(*name_len)] = '\0';
}

struct fatfs_lookup
{
	struct fatfs_sb *sb;
	struct node *node;
	const char *name;
	size_t name_len;
	struct node **child;
	size_t n;
	int ret;
};

static int lookup_cb(const struct fatfs_dirent *dirent, void *userdata)
{
	struct fatfs_lookup *ctx = userdata;
	char name[13];
	size_t name_len = 0;

	get_fixed_name(name, &name_len, dirent);
	ctx->n++;
	if (name_len != ctx->name_len
	 || memcmp(name, ctx->name, name_len))
		return 0;
	if (name_len == 2 && !strcmp(name, "..")
	 && ctx->node->ino == ctx->sb->sb->root->ino)
	{
		ctx->ret = node_lookup(ctx->sb->sb->dir, name, name_len, ctx->child);
		return 1;
	}
	node_cache_lock(&ctx->sb->sb->node_cache);
	{
		struct node *child = node_cache_find(&ctx->sb->sb->node_cache,
		                                     get_ino(dirent));
		if (child)
		{
			*ctx->child = child;
			ctx->ret = 0;
			node_cache_unlock(&ctx->sb->sb->node_cache);
			return 1;
		}
	}
	struct fatfs_node *child;
	ctx->ret = fs_mknode(ctx->sb, dirent, &child);
	if (!ctx->ret)
		*ctx->child = &child->node;
	node_cache_add(&ctx->sb->sb->node_cache, &child->node);
	node_cache_unlock(&ctx->sb->sb->node_cache);
	return 1;
}

static int dir_lookup(struct node *node, const char *name, size_t name_len,
                      struct node **child)
{
	struct fatfs_node *dir = (struct fatfs_node*)node;
	struct fatfs_sb *fatsb = node->sb->private;
	struct fatfs_lookup fatctx;
	int ret;

	fatctx.sb = fatsb;
	fatctx.node = node;
	fatctx.name = name;
	fatctx.name_len = name_len;
	fatctx.child = child;
	fatctx.n = 0;
	fatctx.ret = -ENOENT;
	ret = dir_iterate(dir, lookup_cb, &fatctx);
	if (ret)
		return ret;
	return fatctx.ret;
}

struct fatfs_readdir
{
	struct fs_readdir_ctx *ctx;
	size_t written;
	size_t n;
};

static int readdir_cb(const struct fatfs_dirent *dirent, void *data)
{
	struct fatfs_readdir *ctx = data;
	char name[13];
	size_t name_len = 0;
	int ret;

	if (ctx->n != (size_t)ctx->ctx->off)
	{
		ctx->n++;
		return 0;
	}
	get_fixed_name(name, &name_len, dirent);
	ret = ctx->ctx->fn(ctx->ctx, name, name_len, ctx->n,
	                   get_ino(dirent), get_mode(dirent));
	ctx->n++;
	if (ret)
		return 1;
	ctx->written++;
	ctx->ctx->off++;
	return 0;
}

static int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx)
{
	struct fatfs_node *dir = (struct fatfs_node*)node;
	struct fatfs_readdir fatctx;
	int ret;

	fatctx.ctx = ctx;
	fatctx.written = 0;
	fatctx.n = 0;
	ret = dir_iterate(dir, readdir_cb, &fatctx);
	if (ret)
		return ret;
	return fatctx.written;
}

static ssize_t reg_read(struct file *file, struct uio *uio)
{
	struct fatfs_node *node = (struct fatfs_node*)file->node;

	if (uio->off < 0)
		return -EINVAL;
	if (uio->off >= node->node.attr.size)
		return 0;
	size_t count = uio->count;
	size_t rem = node->node.attr.size - uio->off;
	if (count > rem)
		count = rem;
	size_t rd = 0;
	while (count)
	{
		size_t n = count;
		size_t pad = uio->off % PAGE_SIZE;
		if (n > PAGE_SIZE - pad)
			n = PAGE_SIZE - pad;
		uint8_t *blk;
		int ret = get_node_cache_page(node, uio->off - pad, &blk);
		if (ret)
			return ret;
		uio_copyin(uio, &blk[pad], n);
		vm_unmap(blk, PAGE_SIZE);
		uio->off += n;
		uio->count -= n;
		count -= n;
		rd += n;
	}
	return rd;
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

static int update_free_blocks(struct fatfs_sb *fatsb)
{
	uint8_t carry = 0;
	uint8_t ncarry = 0;
	uint32_t sectors = 0;
	for (size_t i = 0; i < (fatsb->fat_size << fatsb->sector_shift); i += PAGE_SIZE)
	{
		uint8_t *blk;
		int ret = get_fat_cache_page(fatsb, i, &blk);
		if (ret)
		{
			TRACE("failed to get fat cache");
			return ret;
		}
		switch (fatsb->fatsz)
		{
			case FAT12:
			{
				size_t n = 0;
				switch (ncarry)
				{
					case 0:
						break;
					case 1:
						if (!(carry | (blk[0] << 4)))
							fatsb->free_sectors++;
						if (++sectors == fatsb->clusters_count)
							return 0;
						n = 2;
						break;
					case 2:
						if (!(carry | ((blk[0] & 0x0F) << 8)))
							fatsb->free_sectors++;
						if (++sectors == fatsb->clusters_count)
							return 0;
						n = 1;
						break;
				}
				for (; n <= PAGE_SIZE * 2 - 3; n += 3)
				{
					uint16_t fat = le16dec(&blk[n / 2]);
					if (n & 1)
						fat >>= 4;
					else
						fat &= 0x0FFF;
					if (!fat)
						fatsb->free_sectors++;
					if (++sectors == fatsb->clusters_count)
						return 0;
				}
				switch (PAGE_SIZE * 2 - n)
				{
					case 0:
						ncarry = 0;
						break;
					case 1:
						ncarry = 1;
						carry = blk[PAGE_SIZE - 1] & 0xF0;
						break;
					case 2:
						ncarry = 2;
						carry = blk[PAGE_SIZE - 1];
						break;
				}
				break;
			}
			case FAT16:
				for (size_t n = 0; n < PAGE_SIZE; n += 2)
				{
					if (!le16dec(&blk[n]))
						fatsb->free_sectors++;
					if (++sectors == fatsb->clusters_count)
						return 0;
				}
				break;
			case FAT32:
				for (size_t n = 0; n < PAGE_SIZE; n += 4)
				{
					if (!le32dec(&blk[n]))
						fatsb->free_sectors++;
					if (++sectors == fatsb->clusters_count)
						return 0;
				}
				break;
		}
		vm_unmap(blk, PAGE_SIZE);
	}
	return 0;
}

static int fatfs_mount(struct node *dir, struct node *dev, unsigned long flags,
                        const void *udata, struct fs_sb **sbp)
{
	struct fatfs_dirent root_dirent;
	struct fatfs_node *root = NULL;
	struct fatfs_sb *fatsb = NULL;
	struct fs_sb *sb = NULL;
	ssize_t ret;

	(void)flags;
	(void)udata;
	if (!dev)
		return -EINVAL;
	node_ref(dev);
	ret = fs_sb_alloc(&fs_type, &sb);
	if (ret)
		goto err;
	fatsb = malloc(sizeof(*fatsb), M_ZERO);
	if (!fatsb)
	{
		TRACE("fatsb allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ramfile_init(&fatsb->fat_cache);
	fatsb->sb = sb;
	sb->private = fatsb;
	sb->flags |= ST_RDONLY;
	ret = file_fromnode(dev, O_RDONLY, &fatsb->dev);
	if (ret)
	{
		TRACE("failed to open dev");
		goto err;
	}
	ret = file_open(fatsb->dev, dev);
	if (ret)
	{
		TRACE("failed to open file");
		goto err;
	}
	ret = file_readseq(fatsb->dev, &fatsb->bpb, sizeof(fatsb->bpb), 0);
	if (ret < 0)
	{
		TRACE("failed to read from dev");
		goto err;
	}
	if (ret != sizeof(fatsb->bpb))
	{
		TRACE("failed to read full bpb");
		ret = -EINVAL;
		goto err;
	}
	if (fatsb->bpb.ebpb.signature_word[0] != 0x55
	 || fatsb->bpb.ebpb.signature_word[1] != 0xAA)
	{
		TRACE("invalid magic: 0x%02" PRIx8 " 0x%02 " PRIx8,
		      fatsb->bpb.ebpb.signature_word[0],
		      fatsb->bpb.ebpb.signature_word[1]);
		ret = -EINVAL;
		goto err;
	}
	switch (le16dec(fatsb->bpb.byts_per_sec))
	{
		case 512:
			fatsb->sector_shift = 9;
			break;
		case 1024:
			fatsb->sector_shift = 10;
			break;
		case 2048:
			fatsb->sector_shift = 11;
			break;
		case 4096:
			fatsb->sector_shift = 12;
			break;
		default:
			TRACE("invalid block size: %" PRIu16,
			      le16dec(fatsb->bpb.byts_per_sec));
			ret = -EINVAL;
			goto err;
	}
	fatsb->sector_size = 1 << fatsb->sector_shift;
	switch (fatsb->bpb.sec_per_clus)
	{
		case 1:
			fatsb->sec_per_clus_shift = 0;
			break;
		case 2:
			fatsb->sec_per_clus_shift = 1;
			break;
		case 4:
			fatsb->sec_per_clus_shift = 2;
			break;
		case 8:
			fatsb->sec_per_clus_shift = 3;
			break;
		case 16:
			fatsb->sec_per_clus_shift = 4;
			break;
		case 32:
			fatsb->sec_per_clus_shift = 5;
			break;
		case 64:
			fatsb->sec_per_clus_shift = 6;
			break;
		case 128:
			fatsb->sec_per_clus_shift = 7;
			break;
		default:
			TRACE("invalid sectors per cluster: %" PRIu8,
			      fatsb->bpb.sec_per_clus);
			ret = -EINVAL;
			goto err;
	}
	uint32_t total_sectors = le16dec(fatsb->bpb.tot_sec16);
	if (!total_sectors)
		total_sectors = le32dec(fatsb->bpb.tot_sec32);
	fatsb->fat_size = le16dec(fatsb->bpb.fat_sz16);
	if (!fatsb->fat_size)
		fatsb->fat_size = le32dec(fatsb->bpb.ebpb32.fat_sz32);
	fatsb->root_dir_sectors = le16dec(fatsb->bpb.root_ent_cnt) * sizeof(struct fatfs_dirent);
	fatsb->root_dir_sectors += fatsb->sector_size - 1;
	fatsb->root_dir_sectors >>= fatsb->sector_shift;
	fatsb->data_sectors = total_sectors - (le16dec(fatsb->bpb.rsvd_sec_cnt) + (fatsb->bpb.num_fats * fatsb->fat_size) + fatsb->root_dir_sectors);
	fatsb->clusters_count = fatsb->data_sectors >> fatsb->sec_per_clus_shift;
	if (fatsb->clusters_count < 4085)
		fatsb->fatsz = FAT12;
	else if (fatsb->clusters_count < 65525)
		fatsb->fatsz = FAT16;
	else
		fatsb->fatsz = FAT32;
	ret = update_free_blocks(fatsb);
	if (ret)
		goto err;
	memset(&root_dirent, 0, sizeof(root_dirent));
	root_dirent.name[0] = '/';
	root_dirent.attr = FAT_ATTR_DIRECTORY;
	ret = fs_mknode(fatsb, &root_dirent, &root);
	if (ret)
	{
		TRACE("failed to create root node");
		goto err;
	}
	sb->root = &root->node;
	sb->dir = dir;
	node_ref(dir);
	dir->mount = sb;
	*sbp = sb;
	return 0;

err:
	node_free(dev);
	if (fatsb)
	{
		ramfile_destroy(&fatsb->fat_cache);
		file_free(fatsb->dev);
		free(fatsb);
	}
	fs_sb_free(sb);
	free(root);
	return ret;
}

static int fatfs_stat(struct fs_sb *sb, struct statvfs *st)
{
	struct fatfs_sb *fatsb = sb->private;
	st->f_bsize = fatsb->sector_size;
	st->f_frsize = fatsb->sector_size;
	st->f_blocks = fatsb->data_sectors;
	st->f_bfree = fatsb->free_sectors;
	st->f_bavail = st->f_bfree;
	st->f_files = 0;
	st->f_ffree = st->f_bfree;
	st->f_favail = st->f_bavail;
	st->f_fsid = 0;
	st->f_flag = sb->flags;
	st->f_namemax = 255;
	st->f_magic = FATFS_MAGIC;
	return 0;
}

static int init(void)
{
	vfs_register_fs_type(&fs_type);
	return 0;
}

static void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "fatfs",
	.init = init,
	.fini = fini,
};
