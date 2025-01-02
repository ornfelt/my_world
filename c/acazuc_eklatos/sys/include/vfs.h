#ifndef VFS_H
#define VFS_H

#include <refcount.h>
#include <queue.h>
#include <types.h>
#include <mutex.h>
#include <types.h>
#include <time.h>

#define VFS_NOFOLLOW (1 << 0)

#define ST_RDONLY (1 << 0)
#define ST_NOSUID (1 << 1)
#define ST_NOEXEC (1 << 2)

#define DEVFS_MAGIC   0x1001
#define RAMFS_MAGIC   0x1002
#define SYSFS_MAGIC   0x1003
#define PROCFS_MAGIC  0x1004
#define TARFS_MAGIC   0x1005
#define EXT2FS_MAGIC  0xEF53
#define ISO9660_MAGIC 0x1006
#define FATFS_MAGIC   0x1007

#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK     10
#define DT_SOCK    12

#define S_IXOTH (1 << 0)
#define S_IWOTH (1 << 1)
#define S_IROTH (1 << 2)
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)
#define S_IXGRP (1 << 3)
#define S_IWGRP (1 << 4)
#define S_IRGRP (1 << 5)
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IXUSR (1 << 6)
#define S_IWUSR (1 << 7)
#define S_IRUSR (1 << 8)
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_ISVTX (1 << 9)
#define S_ISGID (1 << 10)
#define S_ISUID (1 << 11)
#define S_IFMT (0xFF << 12)

#define S_IFBLK (DT_BLK << 12)
#define S_IFCHR (DT_CHR << 12)
#define S_IFDIR (DT_DIR << 12)
#define S_IFIFO (DT_FIFO << 12)
#define S_IFLNK (DT_LNK << 12)
#define S_IFREG (DT_REG << 12)
#define S_IFSOCK (DT_SOCK << 12)

#define S_ISBLK(mode) ((mode & S_IFMT) == S_IFBLK)
#define S_ISCHR(mode) ((mode & S_IFMT) == S_IFCHR)
#define S_ISDIR(mode) ((mode & S_IFMT) == S_IFDIR)
#define S_ISFIFO(mode) ((mode & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode) ((mode & S_IFMT) == S_IFLNK)
#define S_ISREG(mode) ((mode & S_IFMT) == S_IFREG)
#define S_ISSOCK(mode) ((mode & S_IFMT) == S_IFSOCK)

#define UTIME_OMIT -1
#define UTIME_NOW  -2

struct fs_node_op;
struct file_op;
struct fs_type;
struct fs_sb;
struct node;
struct file;
struct uio;

struct statvfs
{
	unsigned long f_bsize;
	unsigned long f_frsize;
	fsblkcnt_t f_blocks;
	fsblkcnt_t f_bfree;
	fsblkcnt_t f_bavail;
	fsfilcnt_t f_files;
	fsfilcnt_t f_ffree;
	fsfilcnt_t f_favail;
	unsigned long f_fsid;
	unsigned long f_flag;
	unsigned long f_namemax;
	unsigned long f_magic;
};

struct cdev
{
	const struct file_op *fop;
	void *userdata;
	dev_t rdev;
	TAILQ_ENTRY(cdev) chain;
};

struct bdev
{
	const struct file_op *fop;
	void *userdata;
	dev_t rdev;
	TAILQ_ENTRY(bdev) chain;
};

struct fs_type_op
{
	int (*mount)(struct node *dir, struct node *dev, unsigned long flags,
	             const void *udata, struct fs_sb **sb);
	int (*stat)(struct fs_sb *sb, struct statvfs *st);
};

struct fs_type
{
	const struct fs_type_op *op;
	char *name;
	uint32_t flags;
	TAILQ_ENTRY(fs_type) chain;
};

TAILQ_HEAD(node_cache_head, node);

struct node_cache
{
	struct mutex mutex;
	struct node_cache_head *entries;
	size_t size; /* size of htable */
	size_t count;
};

#define FS_SB_READONLY (1 << 0)

struct fs_sb
{
	const struct fs_type *type;
	struct node_cache node_cache;
	struct node *root;
	struct node *dir;
	dev_t dev;
	refcount_t refcount;
	uint32_t flags;
	void *private;
	TAILQ_ENTRY(fs_sb) chain;
};

#define FS_ATTR_ATIME (1 << 0)
#define FS_ATTR_MTIME (1 << 1)
#define FS_ATTR_CTIME (1 << 2)
#define FS_ATTR_UID   (1 << 3)
#define FS_ATTR_GID   (1 << 4)
#define FS_ATTR_MODE  (1 << 5)
#define FS_ATTR_SIZE  (1 << 6)

typedef uint32_t fs_attr_mask_t;

struct fs_attr
{
	struct timespec atime;
	struct timespec mtime;
	struct timespec ctime;
	uid_t uid;
	gid_t gid;
	mode_t mode;
	off_t size;
};

#define NODE_CACHED (1 << 0)

struct node
{
	const struct node_op *op;
	const struct file_op *fop;
	uint32_t flags;
	struct fs_sb *sb;
	struct fs_attr attr;
	union
	{
		struct fs_sb *mount; /* if dir is a mount point */
		struct pipe *pipe; /* DT_FIFO */
		struct cdev *cdev; /* DT_CHR */
		struct bdev *bdev; /* DT_BLK */
		struct pfls *pfls; /* DT_SOCK */
	};
	blksize_t blksize;
	blkcnt_t blocks;
	nlink_t nlink;
	dev_t rdev;
	ino_t ino;
	refcount_t refcount;
	void *userdata;
	TAILQ_ENTRY(node) cache_chain;
};

struct fs_readdir_ctx;

typedef int (*fs_readdir_fn_t)(struct fs_readdir_ctx *ctx, const char *name,
                               uint32_t namelen, off_t off, ino_t ino,
                               uint32_t type);

struct fs_readdir_ctx
{
	fs_readdir_fn_t fn;
	off_t off;
	void *userdata;
};

struct node_op
{
	int (*release)(struct node *node);
	int (*lookup)(struct node *node, const char *name, size_t name_len,
	              struct node **child);
	int (*readdir)(struct node *node, struct fs_readdir_ctx *ctx);
	int (*mknode)(struct node *node, const char *name, size_t name_len,
	              fs_attr_mask_t mask, const struct fs_attr *attr,
	              dev_t dev);
	int (*getattr)(struct node *node, fs_attr_mask_t mask,
	               struct fs_attr *attr);
	int (*setattr)(struct node *node, fs_attr_mask_t mask,
	               const struct fs_attr *attr);
	int (*symlink)(struct node *node, const char *name, size_t name_len,
	               const char *target, fs_attr_mask_t mask,
	               const struct fs_attr *attr);
	ssize_t (*readlink)(struct node *node, struct uio *uio);
	int (*link)(struct node *node, struct node *src, const char *name);
	int (*unlink)(struct node *node, const char *name);
	int (*rmdir)(struct node *node, const char *name);
	int (*rename)(struct node *srcdir, const char *srcname,
	              struct node *dstdir, const char *dstname);
};

struct stat
{
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
};

struct sys_dirent
{
	ino_t ino;
	off_t off;
	uint16_t reclen;
	uint8_t type;
	char name[];
} __attribute__((packed));

int vfs_getnode(struct node *cwd, const char *path, int flags,
                struct node **node);
int vfs_getdir(struct node *cwd, const char *path, int flags,
               struct node **node, char **end_fn);

mode_t vfs_getperm(struct node *node, uid_t uid, gid_t gid);

void node_free(struct node *node);
void node_ref(struct node *node);

void vfs_init_sma(void);
void vfs_init(void);

int vfs_mount(struct node *dir, struct node *dev,
              const struct fs_type *type, unsigned long flags,
              const void *udata, struct fs_sb **sbp);

int vfs_register_fs_type(struct fs_type *fs_type);
const struct fs_type *vfs_get_fs_type(const char *name);

off_t vfs_common_seek(struct file *file, off_t off, int whence);
int vfs_common_getattr(struct node *node, fs_attr_mask_t mask,
                       struct fs_attr *attr);
int vfs_common_setattr(struct node *node, fs_attr_mask_t mask,
                       const struct fs_attr *attr);

int vfs_printmounts(struct uio *uio);

int cdev_init(void);
int cdev_alloc(const char *name, uid_t uid, gid_t gid, mode_t mode, dev_t rdev,
               const struct file_op *fop, struct cdev **cdev);
int bdev_alloc(const char *name, uid_t uid, gid_t gid, mode_t mode, dev_t rdev,
               const struct file_op *fop, struct bdev **bdev);
struct cdev *cdev_find(dev_t dev);
struct bdev *bdev_find(dev_t dev);
void cdev_free(struct cdev *cdev);
void bdev_free(struct bdev *bdev);

int node_release(struct node *node);
int node_lookup(struct node *node, const char *name, size_t name_len,
                struct node **child);
int node_readdir(struct node *node, struct fs_readdir_ctx *ctx);
int node_mknode(struct node *node, const char *name, size_t name_len,
                fs_attr_mask_t mask, struct fs_attr *attr, dev_t dev);
int node_getattr(struct node *node, fs_attr_mask_t mask,
                 struct fs_attr *attr);
int node_setattr(struct node *node, fs_attr_mask_t mask,
                 struct fs_attr *attr);
int node_symlink(struct node *node, const char *name, size_t name_len,
                 const char *target, fs_attr_mask_t mask,
                 struct fs_attr *attr);
ssize_t node_readlink(struct node *node, struct uio *uio);
int node_link(struct node *node, struct node *src, const char *name);
int node_unlink(struct node *node, const char *name);
int node_rmdir(struct node *node, const char *name);
int node_rename(struct node *srcdir, const char *srcname,
                struct node *dstdir, const char *dstname);

int node_cache_init(struct node_cache *cache);
void node_cache_destroy(struct node_cache *cache);
void node_cache_lock(struct node_cache *cache);
void node_cache_unlock(struct node_cache *cache);
struct node *node_cache_find(struct node_cache *cache, ino_t ino);
int node_cache_add(struct node_cache *cache, struct node *node);
int node_cache_remove(struct node_cache *cache, ino_t ino);

int fs_sb_alloc(const struct fs_type *type, struct fs_sb **sb);
void fs_sb_free(struct fs_sb *sb);
void fs_sb_ref(struct fs_sb *sb);

int devfs_mkcdev(const char *name, uid_t uid, gid_t gid, mode_t mode,
                 dev_t rdev, struct cdev *cdev);
int devfs_mkbdev(const char *name, uid_t uid, gid_t gid, mode_t mode,
                 dev_t rdev, struct bdev *bdev);

int sysfs_mknode(const char *name, uid_t uid, gid_t gid, mode_t mode,
                 const struct file_op *fop, struct node **node);

int mounts_register_sysfs(void);

#define VFS_HANDLE_DOT_DOTDOT_LOOKUP(node, parent, name, namelen, child) \
do \
{ \
	if (!strncmp(name, ".", namelen)) \
	{ \
		*(child) = node; \
		node_ref(*(child)); \
		return 0; \
	} \
	if (!strncmp(name, "..", namelen)) \
	{ \
		if ((node)->sb->root == (node)) \
			return node_lookup((node)->sb->dir, name, namelen, child); \
		*(child) = parent; \
		node_ref(*(child)); \
		return 0; \
	} \
} while (0)

#define VFS_HANDLE_DOT_DOTDOT_READDIR(node, parent, ctx, written) \
do \
{ \
	if ((ctx)->off == 0) \
	{ \
		if ((ctx)->fn(ctx, ".", 1, 0, (node)->ino, DT_DIR)) \
			return written; \
		(written)++; \
		(ctx)->off++; \
	} \
	if ((ctx)->off == 1) \
	{ \
		if (ctx->fn(ctx, "..", 2, 1, (node)->sb->root == (node) ? (node)->sb->dir->ino : parent->ino, DT_DIR)) \
			return written; \
		(written)++; \
		(ctx)->off++; \
	} \
} while (0)

extern struct node *g_vfs_root;

#endif
