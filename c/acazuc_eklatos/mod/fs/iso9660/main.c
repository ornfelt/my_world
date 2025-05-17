#define ENABLE_TRACE

#include <ramfile.h>
#include <endian.h>
#include <file.h>
#include <kmod.h>
#include <vfs.h>
#include <uio.h>
#include <std.h>
#include <mem.h>

#define BLOCK_SIZE 2048

#define DIRENT_FLAG_HIDDEN     (1 << 0)
#define DIRENT_FLAG_DIRECTORY  (1 << 1)
#define DIRENT_FLAG_ASSOCIATED (1 << 2)
#define DIRENT_FLAG_EXT_ATTR   (1 << 3)
#define DIRENT_FLAG_NOT_FINAL  (1 << 7)

struct datetime_17
{
	uint8_t year[4];
	uint8_t month[2];
	uint8_t day[2];
	uint8_t hour[2];
	uint8_t minute[2];
	uint8_t second[2];
	uint8_t hundredth[2];
	int8_t tz;
} __attribute__ ((packed));

struct datetime_7
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	int8_t tz;
} __attribute__ ((packed));

struct iso9660_dirent
{
	uint8_t length;
	uint8_t ext_attr_length;
	uint32_t lba_lsb;
	uint32_t lba_msb;
	uint32_t size_lsb;
	uint32_t size_msb;
	struct datetime_7 date;
	uint8_t flags;
	uint8_t interleaved_size;
	uint8_t interleaved_gap;
	uint16_t sequence_lsb;
	uint16_t sequence_msb;
	uint8_t name_len;
	char name[];
} __attribute__ ((packed));

struct volume_descriptor
{
	uint8_t type;
	uint8_t identifier[5];
	uint8_t version;
} __attribute__ ((packed));

struct boot_record
{
	uint8_t type;
	uint8_t identifier[5];
	uint8_t version;
	uint8_t boot_system_identifier[32];
	uint8_t boot_identifier[32];
} __attribute__ ((packed));

struct iso9660_pvd
{
	uint8_t type;
	uint8_t identifier[5];
	uint8_t version;
	uint8_t unused7;
	uint8_t system_identifier[32];
	uint8_t volume_identifier[32];
	uint8_t unused72[8];
	uint32_t volume_space_lsb;
	uint32_t volume_space_msb;
	uint8_t unused88[32];
	uint16_t volume_set_size_lsb;
	uint16_t volume_set_size_msb;
	uint16_t volume_sequence_lsb;
	uint16_t volume_sequence_msb;
	uint16_t logical_block_size_lsb;
	uint16_t logical_block_size_msb;
	uint32_t path_table_size_lsb;
	uint32_t path_table_size_msb;
	uint32_t lpath_table_lba;
	uint32_t lpath_table_opt_lba;
	uint32_t mpath_table_lba;
	uint32_t mpath_table_opt_lba;
	uint8_t root_directory[34];
	uint8_t volume_set_identifier[128];
	uint8_t publisher_identifier[128];
	uint8_t data_preparer_identifier[128];
	uint8_t application_identifier[128];
	uint8_t copyright_file_identifier[37];
	uint8_t abstract_file_identifier[37];
	uint8_t bibliographic_file_identifier[37];
	struct datetime_17 creation_time;
	struct datetime_17 modification_time;
	struct datetime_17 expiration_time;
	struct datetime_17 effective_time;
	uint8_t file_structure_version;
} __attribute__ ((packed));

struct iso9660_sb
{
	struct fs_sb *sb;
	struct file *dev;
	ino_t ino;
	fsblkcnt_t blkcnt;
	fsfilcnt_t inocnt;
	struct iso9660_pvd pvd;
};

struct iso9660_node
{
	struct node node;
	struct iso9660_dirent dirent;
	struct ramfile cache; /* XXX this must be somehow generalized to all the filesystems */
};

static int dir_lookup(struct node *node,
                      const char *name,
                      size_t name_len,
                      struct node **child);
static int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx);

static ssize_t reg_read(struct file *file, struct uio *uio);
static int reg_mmap(struct file *file, struct vm_zone *zone);
static int reg_fault(struct vm_zone *zone, off_t off, struct page **page);

static ssize_t lnk_readlink(struct node *node, struct uio *uio);

static int iso9660_node_release(struct node *node);
static int iso9660_mount(struct node *dir,
                         struct node *dev,
                         unsigned long flags,
                         const void *udata,
                         struct fs_sb **sb);
static int iso9660_stat(struct fs_sb *sb, struct statvfs *st);

static const struct fs_type_op
fs_type_op =
{
	.mount = iso9660_mount,
	.stat = iso9660_stat,
};

static struct fs_type
fs_type =
{
	.op = &fs_type_op,
	.name = "iso9660",
	.flags = 0,
};

static const struct node_op
dir_op =
{
	.release = iso9660_node_release,
	.lookup = dir_lookup,
	.readdir = dir_readdir,
	.getattr = vfs_common_getattr,
};

static const struct file_op
dir_fop =
{
};

static const struct node_op
reg_op =
{
	.release = iso9660_node_release,
	.getattr = vfs_common_getattr,
};

static const struct file_op
reg_fop =
{
	.read = reg_read,
	.mmap = reg_mmap,
	.seek = vfs_common_seek,
};

static const struct vm_zone_op
reg_vm_op =
{
	.fault = reg_fault,
};

static const struct node_op
fifo_op =
{
	.release = iso9660_node_release,
	.getattr = vfs_common_getattr,
};

static const struct node_op
sock_op =
{
	.release = iso9660_node_release,
	.getattr = vfs_common_getattr,
};

static const struct node_op
bdev_op =
{
	.release = iso9660_node_release,
	.getattr = vfs_common_getattr,
};

static const struct node_op
cdev_op =
{
	.release = iso9660_node_release,
	.getattr = vfs_common_getattr,
};

static const struct node_op
lnk_op =
{
	.readlink = lnk_readlink,
	.release = iso9660_node_release,
	.getattr = vfs_common_getattr,
};

static const struct file_op
lnk_fop =
{
};

static inline void
print_datetime_17(const struct datetime_17 *dt)
{
	printf("%.4s-%.2s-%.2s %.2s:%.2s:%.2s.%.2s (%d)",
	       (char*)dt->year, (char*)dt->month, (char*)dt->day,
	       (char*)dt->hour, (char*)dt->minute, (char*)dt->second,
	       (char*)dt->hundredth, dt->tz);
}

static inline void
print_datetime_7(const struct datetime_7 *dt)
{
	printf("%-4u-%02u-%02u %02u:%02u:%02u (%d)",
	       dt->year + 1900, dt->month, dt->day,
	       dt->hour, dt->minute, dt->second, dt->tz);
}

static inline void
print_dirent(const struct iso9660_dirent *dirent)
{
	printf("length      : 0x%02" PRIx8 "\n", dirent->length);
	printf("attr size   : 0x%02" PRIx8 "\n", dirent->ext_attr_length);
	printf("lba         : 0x%08" PRIx32 "\n", dirent->lba_lsb);
	printf("size        : 0x%08" PRIx32 "\n", dirent->size_lsb);
	print_datetime_7(&dirent->date);
	printf("flags       : 0x%02" PRIx8 "\n", dirent->flags);
	printf("interl size : 0x%02" PRIx8 "\n", dirent->interleaved_size);
	printf("interl gap  : 0x%02" PRIx8 "\n", dirent->interleaved_gap);
	printf("sequencce   : 0x%04" PRIx16 "\n", dirent->sequence_lsb);
	printf("name        : %.*s\n", (int)dirent->name_len, (char*)dirent->name);
}

static inline void
print_pvdesc(const struct iso9660_pvd *pvd)
{
	printf("type        : 0x%" PRIx8 "\n", pvd->type);
	printf("identifier  : %.5s\n", (char*)pvd->identifier);
	printf("version     : 0x%" PRIx8 "\n", pvd->version);
	printf("system id   : %.32s\n", (char*)pvd->system_identifier);
	printf("volume id   : %.32s\n", (char*)pvd->volume_identifier);
	printf("space       : 0x%08" PRIx32 "\n", pvd->volume_space_lsb);
	printf("set size    : 0x%04" PRIx16 "\n", pvd->volume_set_size_lsb);
	printf("sequence    : 0x%04" PRIx16 "\n", pvd->volume_sequence_lsb);
	printf("blk size    : 0x%04" PRIx16 "\n", pvd->logical_block_size_lsb);
	printf("path tbl sz : 0x%08" PRIx32 "\n", pvd->path_table_size_lsb);
	printf("path tbl lba: 0x%08" PRIx32 "\n", pvd->lpath_table_lba);
	printf("opt tbl lba : 0x%08" PRIx32 "\n", pvd->lpath_table_opt_lba);
	printf("volume set  : %.128s\n", pvd->volume_set_identifier);
	printf("publisher   : %.128s\n", pvd->publisher_identifier);
	printf("preparer    : %.128s\n", pvd->data_preparer_identifier);
	printf("application : %.128s\n", pvd->application_identifier);
	printf("copyright   : %.37s\n", pvd->copyright_file_identifier);
	printf("abstract    : %.37s\n", pvd->abstract_file_identifier);
	printf("biblio      : %.37s\n", pvd->bibliographic_file_identifier);
	printf("creation    : ");
	print_datetime_17(&pvd->creation_time);
	printf("\n");
	printf("modification: ");
	print_datetime_17(&pvd->modification_time);
	printf("\n");
	printf("expiration  : ");
	print_datetime_17(&pvd->expiration_time);
	printf("\n");
	printf("effective   : ");
	print_datetime_17(&pvd->effective_time);
	printf("\n");
	printf("root dir    :\n");
	print_dirent((struct iso9660_dirent*)&pvd->root_directory[0]);
}

static inline void
print_bootr(const struct boot_record *bootr)
{
	printf("type        : 0x%" PRIx8 "\n", bootr->type);
	printf("identifier  : %.5s\n", (char*)bootr->identifier);
	printf("version     : 0x%" PRIx8 "\n", bootr->version);
	printf("system id   : %.32s\n", bootr->boot_system_identifier);
	printf("boot id     : %.32s\n", bootr->boot_identifier);
}

static int
timespec_from_datetime(struct timespec *ts,
                       time_t year,
                       time_t month,
                       time_t yday,
                       time_t hour,
                       time_t minute,
                       time_t second,
                       time_t hundredth,
                       time_t tz)
{
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
	ts->tv_sec = second
	           + minute * 60
	           + hour * 3600
	           + yday * 86400
	           + (year - 70) * 31536000
	           + ((year - 69) / 4) * 86400
	           - ((year - 1) / 100) * 86400
	           + ((year + 299) / 400) * 86400
	           + tz * 60 * 15;
	ts->tv_nsec = hundredth * 10000000;
	return 0;
}

static int
timespec_from_datetime_17(struct timespec *ts,
                          const struct datetime_17 *dt)
{
	for (size_t i = 0; i < sizeof(*dt); ++i)
	{
		if (!isdigit(((uint8_t*)dt)[i]))
			return -EINVAL;
	}
	uint16_t year = (dt->year[0] - '0') * 1000
	              + (dt->year[1] - '0') * 100
	              + (dt->year[2] - '0') * 10
	              + (dt->year[3] - '0');
	uint8_t month = (dt->month[0] - '0') * 10
	              + (dt->month[1] - '0');
	uint8_t yday = (dt->day[0] - '0') * 10
	             + (dt->day[1] - '0');
	uint8_t hour = (dt->hour[0] - '0') * 10
	             + (dt->hour[1] - '0');
	uint8_t minute = (dt->minute[0] - '0') * 10
	               + (dt->minute[1] - '0');
	uint8_t second = (dt->second[0] - '0') * 10
	               + (dt->second[1] - '0');
	uint8_t hundredth = (dt->hundredth[0] - '0') * 10
	                  + (dt->hundredth[1] - '0');
	return timespec_from_datetime(ts, year, month, yday, hour, minute,
	                              second, hundredth, dt->tz);
}

static void
timespec_from_datetime_7(struct timespec *ts,
                         const struct datetime_7 *dt)
{
	time_t yday = dt->day ? dt->day - 1 : 0;
	time_t month = dt->month ? dt->month - 1 : 0;
	timespec_from_datetime(ts, dt->year, month, yday, dt->hour,
	                       dt->minute, dt->second, 0, dt->tz);
}

static void *
get_dirent_susb(const struct iso9660_dirent *dirent, char *code)
{
	size_t pos = sizeof(*dirent) + dirent->name_len;
	if (pos & 1)
		pos++;
	while (pos + 4 < dirent->length)
	{
		uint8_t *base = &((uint8_t*)dirent)[pos];
		if (pos + base[2] > dirent->length)
		{
			TRACE("ext attrs too big");
			return NULL;
		}
		if (base[0] == code[0] && base[1] == code[1])
			return base;
		pos += base[2];
	}
	return NULL;
}

static mode_t
get_dirent_mode(const struct iso9660_dirent *dirent)
{
	mode_t mode = 0;
	uint8_t *susb_px = get_dirent_susb(dirent, "PX");
	if (susb_px && susb_px[3] == 1 && susb_px[2] == 36)
	{
		mode = le16dec(&susb_px[4]);
		switch (mode & S_IFMT)
		{
			case S_IFBLK:
			case S_IFCHR:
			case S_IFDIR:
			case S_IFIFO:
			case S_IFLNK:
			case S_IFREG:
			case S_IFSOCK:
				break;
			default:
				goto end;
		}
		return mode;
	}
end:
	mode = 0555;
	if (dirent->flags & DIRENT_FLAG_DIRECTORY)
		mode |= S_IFDIR;
	else
		mode |= S_IFREG;
	return mode;
}

static int
get_dirent_dev(const struct iso9660_dirent *dirent, dev_t *rdev)
{
	uint8_t *susb_pn = get_dirent_susb(dirent, "PN");
	if (!susb_pn || susb_pn[3] != 1 || susb_pn[2] != 20)
		return -EINVAL;
	dev_t maj = le32dec(&susb_pn[4]);
	dev_t min = le32dec(&susb_pn[12]);
	*rdev = makedev(maj, min);
	return 0;
}

static void
get_dirent_times(const struct iso9660_dirent *dirent,
                 struct timespec *ctime,
                 struct timespec *atime,
                 struct timespec *mtime)
{
	struct timespec simpletime;
	timespec_from_datetime_7(&simpletime, &dirent->date);
	*ctime = simpletime;
	*atime = simpletime;
	*mtime = simpletime;
	uint8_t *susb_tf = get_dirent_susb(dirent, "TF");
	if (!susb_tf || susb_tf[3] != 1 || susb_tf[2] < 5)
		return;
	size_t ntimes = 0;
	for (size_t i = 0; i < 7; ++i)
	{
		if (susb_tf[4] & (1 << i))
			ntimes++;
	}
	size_t bpt = (susb_tf[4] & (1 << 7)) ? 17 : 7;
	if (susb_tf[2] != bpt * ntimes + 5)
		return;
	size_t pos = 5;
	if (susb_tf[4] & (1 << 0))
		pos += bpt;
	if (susb_tf[4] & (1 << 1))
	{
		if (susb_tf[4] & (1 << 7))
			timespec_from_datetime_17(mtime, (void*)&susb_tf[pos]);
		else
			timespec_from_datetime_7(mtime, (void*)&susb_tf[pos]);
		pos += bpt;
	}
	if (susb_tf[4] & (1 << 2))
	{
		if (susb_tf[4] & (1 << 7))
			timespec_from_datetime_17(atime, (void*)&susb_tf[pos]);
		else
			timespec_from_datetime_7(atime, (void*)&susb_tf[pos]);
		pos += bpt;
	}
	if (susb_tf[4] & (1 << 3))
	{
		if (susb_tf[4] & (1 << 7))
			timespec_from_datetime_17(ctime, (void*)&susb_tf[pos]);
		else
			timespec_from_datetime_7(ctime, (void*)&susb_tf[pos]);
		pos += bpt;
	}
	return;
}

static int
fs_mknode(struct iso9660_sb *sb,
          const struct iso9660_dirent *dirent,
          struct iso9660_node **nodep)
{
	mode_t mode = get_dirent_mode(dirent);
	dev_t rdev;
	switch (mode & S_IFMT)
	{
		case S_IFBLK:
		case S_IFCHR:
			if (get_dirent_dev(dirent, &rdev))
				return -EINVAL;
			break;
		default:
			rdev = 0;
			break;
	}
	struct iso9660_node *node = malloc(sizeof(*node), M_ZERO);
	if (!node)
	{
		TRACE("node allocation failed");
		return -ENOMEM;
	}
	memcpy(&node->dirent, dirent, sizeof(*dirent));
	ramfile_init(&node->cache);
	node->node.sb = sb->sb;
	node->node.ino = dirent->lba_lsb;
	node->node.rdev = rdev;
	node->node.attr.mode = mode;
	switch (node->node.attr.mode & S_IFMT)
	{
		case S_IFDIR:
			node->node.fop = &dir_fop;
			node->node.op = &dir_op;
			break;
		case S_IFREG:
			node->node.fop = &reg_fop;
			node->node.op = &reg_op;
			break;
		case S_IFBLK:
			node->node.op = &bdev_op;
			break;
		case S_IFCHR:
			node->node.op = &cdev_op;
			break;
		case S_IFIFO:
			node->node.op = &fifo_op;
			break;
		case S_IFLNK:
			node->node.fop = &lnk_fop;
			node->node.op = &lnk_op;
			break;
		case S_IFSOCK:
			node->node.op = &sock_op;
			break;
	}
	get_dirent_times(dirent, &node->node.attr.ctime,
	                 &node->node.attr.atime,
	                 &node->node.attr.mtime);
	node->node.attr.size = dirent->size_lsb;
	node->node.nlink = 1;
	node->node.blksize = BLOCK_SIZE;
	node->node.blocks = (node->node.attr.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
	refcount_init(&node->node.refcount, 1);
	sb->inocnt++;
	*nodep = node;
	return 0;
}

static int
iso9660_node_release(struct node *node)
{
	struct iso9660_node *isonode = (struct iso9660_node*)node;
	ramfile_destroy(&isonode->cache);
	return 0;
}

static int
get_node_cache_page(struct iso9660_sb *sb,
                    struct iso9660_node *node,
                    uint64_t off,
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
	uio_fromkbuf(&uio, &iov, *blk, PAGE_SIZE,
	             node->dirent.lba_lsb * BLOCK_SIZE + off);
	ssize_t ret = file_read(sb->dev, &uio);
	if (ret < 0)
	{
		TRACE("failed to read directory block: %s",
		      strerror(ret));
		vm_unmap(*blk, PAGE_SIZE);
		ramfile_rmpage(&node->cache, off / PAGE_SIZE);
		return ret;
	}
	/* if the second block of the page isn't found,
	 * the size_lsb will block further read
	 * and the page data will be zero-filled,
	 * so no worry here
	 */
	if (ret < BLOCK_SIZE)
	{
		TRACE("failed to read full directory block");
		vm_unmap(*blk, PAGE_SIZE);
		ramfile_rmpage(&node->cache, off / PAGE_SIZE);
		return -ENXIO;
	}
	if (ret < PAGE_SIZE)
		memset(&(*blk)[ret], 0, PAGE_SIZE - ret);
	return 0;
}

static int
dir_iterate(struct iso9660_sb *sb,
             struct iso9660_node *dir,
            int (*cb)(const struct iso9660_dirent *dirent, void *data),
            void *data)
{
	off_t off = 0;
	while (off < dir->dirent.size_lsb)
	{
		uint8_t *blk;
		int ret = get_node_cache_page(sb, dir, off, &blk);
		if (ret)
			return ret;
		const struct iso9660_dirent *dirent = (struct iso9660_dirent*)&blk[off % PAGE_SIZE];
		if (dirent->length > BLOCK_SIZE - (off % BLOCK_SIZE))
		{
			TRACE("dirent across page boundary");
			vm_unmap(blk, PAGE_SIZE);
			return -EINVAL;
		}
		if (dirent->name_len > dirent->length - sizeof(*dirent))
		{
			TRACE("dirent name too big");
			vm_unmap(blk, PAGE_SIZE);
			return -EINVAL;
		}
		if (!dirent->length)
		{
			vm_unmap(blk, PAGE_SIZE);
			off += BLOCK_SIZE - 1;
			off -= off % BLOCK_SIZE;
			continue;
		}
#if 0
		size_t pos = sizeof(*dirent) + dirent->name_len;
		if (pos & 1)
			pos++;
		while (pos + 4 < dirent->length)
		{
			uint8_t *base = &((uint8_t*)dirent)[pos];
			uint8_t length = base[2];
			if (pos + length > dirent->length)
			{
				TRACE("ext attrs too big");
				vm_unmap(blk, PAGE_SIZE);
				return -EINVAL;
			}
			printf("ext: %c%c %x %x\n", base[0], base[1], base[2], base[3]);
			pos += length;
		}
#endif
		if (cb(dirent, data))
		{
			vm_unmap(blk, PAGE_SIZE);
			break;
		}
		off += dirent->length;
		vm_unmap(blk, PAGE_SIZE);
	}
	return 0;
}

static const char *
get_fixed_name(const struct iso9660_dirent *dirent, size_t *name_len)
{
	uint8_t *susb_nm = get_dirent_susb(dirent, "NM");
	if (susb_nm && susb_nm[3] == 1)
	{
		*name_len = susb_nm[2] - 5;
		return (const char*)&susb_nm[5];
	}
	if (dirent->name_len != 1)
	{
		*name_len = dirent->name_len;
		return dirent->name;
	}
	if (dirent->name[0] == 0)
	{
		*name_len = 1;
		return ".";
	}
	if (dirent->name[0] == 1)
	{
		*name_len = 2;
		return "..";
	}
	*name_len = dirent->name_len;
	return dirent->name;
}

struct iso9660_lookup
{
	struct iso9660_sb *sb;
	struct node *node;
	const char *name;
	size_t name_len;
	struct node **child;
	size_t n;
	int ret;
};

static int
lookup_cb(const struct iso9660_dirent *dirent, void *data)
{
	struct iso9660_lookup *ctx = data;
	const char *name;
	size_t name_len;

	name = get_fixed_name(dirent, &name_len);
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
		struct node *child = node_cache_find(&ctx->sb->sb->node_cache, dirent->lba_lsb);
		if (child)
		{
			*ctx->child = child;
			ctx->ret = 0;
			node_cache_unlock(&ctx->sb->sb->node_cache);
			return 1;
		}
	}
	struct iso9660_node *child;
	ctx->ret = fs_mknode(ctx->sb, dirent, &child);
	if (!ctx->ret)
		*ctx->child = &child->node;
	node_cache_add(&ctx->sb->sb->node_cache, &child->node);
	node_cache_unlock(&ctx->sb->sb->node_cache);
	return 1;
}

static int
dir_lookup(struct node *node,
           const char *name,
           size_t name_len,
           struct node **child)
{
	struct iso9660_node *dir = (struct iso9660_node*)node;
	struct iso9660_sb *isosb = node->sb->private;
	struct iso9660_lookup isoctx;
	int ret;

	isoctx.sb = isosb;
	isoctx.node = node;
	isoctx.name = name;
	isoctx.name_len = name_len;
	isoctx.child = child;
	isoctx.n = 0;
	isoctx.ret = -ENOENT;
	ret = dir_iterate(isosb, dir, lookup_cb, &isoctx);
	if (ret)
		return ret;
	return isoctx.ret;
}

struct iso9660_readdir
{
	struct fs_readdir_ctx *ctx;
	size_t written;
	size_t n;
};

static int
readdir_cb(const struct iso9660_dirent *dirent, void *data)
{
	struct iso9660_readdir *ctx = data;
	const char *name;
	size_t name_len;
	mode_t mode;
	int ret;

	if (ctx->n != (size_t)ctx->ctx->off)
	{
		ctx->n++;
		return 0;
	}
	name = get_fixed_name(dirent, &name_len);
	mode = get_dirent_mode(dirent);
	ret = ctx->ctx->fn(ctx->ctx, name, name_len, ctx->n, dirent->lba_lsb,
	                   mode >> 12);
	ctx->n++;
	if (ret)
		return 1;
	ctx->written++;
	ctx->ctx->off++;
	return 0;
}

static int
dir_readdir(struct node *node, struct fs_readdir_ctx *ctx)
{
	struct iso9660_node *dir = (struct iso9660_node*)node;
	struct iso9660_sb *isosb = node->sb->private;
	struct iso9660_readdir isoctx;
	int ret;

	isoctx.ctx = ctx;
	isoctx.written = 0;
	isoctx.n = 0;
	ret = dir_iterate(isosb, dir, readdir_cb, &isoctx);
	if (ret)
		return ret;
	return isoctx.written;
}

static ssize_t
node_read(struct iso9660_node *node, struct uio *uio)
{
	if (uio->off < 0)
		return -EINVAL;
	if (uio->off >= node->node.attr.size)
		return 0;
	size_t count = uio->count;
	size_t rem = node->node.attr.size - uio->off;
	if (count > rem)
		count = rem;
	struct iso9660_sb *isosb = node->node.sb->private;
	off_t foff = (off_t)BLOCK_SIZE * node->dirent.lba_lsb;
	if (__builtin_add_overflow(foff, uio->off, &foff))
		return -EOVERFLOW;
	off_t tmpoff = uio->off;
	size_t tmpcount = uio->count;
	uio->off = foff;
	uio->count = count;
	ssize_t ret = file_read(isosb->dev, uio);
	uio->off = tmpoff + (uio->off - foff);
	uio->count = tmpcount - (count - uio->count);
	return ret;
}

static ssize_t
reg_read(struct file *file, struct uio *uio)
{
	return node_read((struct iso9660_node*)file->node, uio);
}

static int
reg_fault(struct vm_zone *zone, off_t off, struct page **page)
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

static int
reg_mmap(struct file *file, struct vm_zone *zone)
{
	(void)file;
	zone->op = &reg_vm_op;
	return 0;
}

static ssize_t
lnk_readlink(struct node *node, struct uio *uio)
{
	return node_read((struct iso9660_node*)node, uio);
}

static int
iso9660_stat(struct fs_sb *sb, struct statvfs *st)
{
	struct iso9660_sb *isosb = sb->private;
	st->f_bsize = BLOCK_SIZE;
	st->f_frsize = BLOCK_SIZE;
	st->f_blocks = isosb->blkcnt;
	st->f_bfree = 0;
	st->f_bavail = 0;
	st->f_files = isosb->inocnt;
	st->f_ffree = 0;
	st->f_favail = 0;
	st->f_fsid = 0;
	st->f_flag = sb->flags;
	st->f_namemax = 222;
	st->f_magic = ISO9660_MAGIC;
	return 0;
}

static int
parse_descriptors(struct iso9660_sb *sb)
{
	uint8_t desc_data[BLOCK_SIZE];
	const struct volume_descriptor *vdesc = (struct volume_descriptor*)&desc_data[0];
	int has_pvd = 0;
	size_t offset = 32768;
	while (1)
	{
		ssize_t ret = file_readseq(sb->dev, desc_data, BLOCK_SIZE,
		                           offset);
		if (ret < 0)
		{
			TRACE("failed to read descriptor: %s", strerror(ret));
			return -EINVAL;
		}
		if (ret < BLOCK_SIZE)
		{
			TRACE("failed to read full descriptor");
			return -EINVAL;
		}
		switch (vdesc->type)
		{
			case 0x00:
				break;
			case 0x01:
			{
				const struct iso9660_pvd *pvd = (struct iso9660_pvd*)vdesc;
				if (pvd->logical_block_size_lsb != BLOCK_SIZE)
				{
					TRACE("unsupported block size != 2048");
					return -EINVAL;
				}
				has_pvd = 1;
				memcpy(&sb->pvd, pvd, sizeof(*pvd));
				break;
			}
			case 0x02:
				break;
			case 0x03:
				break;
			case 0xFF:
				goto end;
			default:
				TRACE("unknown descriptor type: %" PRIx8,
				      vdesc->type);
				return -EINVAL;
		}
		offset += 2048;
	}
end:
	if (!has_pvd)
	{
		TRACE("no pvd");
		return -EINVAL;
	}
	return 0;
}

static int
iso9660_mount(struct node *dir,
              struct node *dev,
              unsigned long flags,
              const void *udata,
              struct fs_sb **sbp)
{
	struct iso9660_node *root = NULL;
	struct iso9660_sb *isosb = NULL;
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
	isosb = malloc(sizeof(*isosb), M_ZERO);
	if (!isosb)
	{
		TRACE("failed to malloc iso9660 sb");
		ret = -ENOMEM;
		goto err;
	}
	isosb->sb = sb;
	sb->private = isosb;
	sb->flags |= ST_RDONLY;
	ret = file_fromnode(dev, O_RDONLY, &isosb->dev);
	if (ret)
	{
		TRACE("failed to open dev");
		goto err;
	}
	ret = file_open(isosb->dev, dev);
	if (ret)
	{
		TRACE("failed to open file");
		goto err;
	}
	ret = parse_descriptors(isosb);
	if (ret)
		goto err;
	ret = fs_mknode(isosb, (struct iso9660_dirent*)&isosb->pvd.root_directory[0], &root);
	if (ret)
	{
		TRACE("failed to create tarfs root");
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
	if (isosb)
	{
		file_free(isosb->dev);
		free(isosb);
	}
	fs_sb_free(sb);
	free(root);
	return ret;
}

static int
init(void)
{
	vfs_register_fs_type(&fs_type);
	return 0;
}

static void
fini(void)
{
}

struct kmod_info
kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "fs/iso9660",
	.init = init,
	.fini = fini,
};
