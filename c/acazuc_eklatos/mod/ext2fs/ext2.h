#ifndef EXT2_H
#define EXT2_H

#include <types.h>
#include <vfs.h>

#define EXT2_MAXBLKSZ_U32 1024
#define EXT2_MAXBLKSZ_U8  4096

struct ext2_sb
{
	uint32_t inodes_count;
	uint32_t blocks_count;
	uint32_t r_blocks_count;
	uint32_t free_blocks_count;
	uint32_t free_inodes_count;
	uint32_t first_data_block;
	uint32_t log_block_size;
	uint32_t log_frag_size;
	uint32_t blocks_per_group;
	uint32_t frags_per_group;
	uint32_t inodes_per_group;
	uint32_t mtime;
	uint32_t wtime;
	uint16_t mnt_count;
	uint16_t max_mnt_count;
	uint16_t magic;
	uint16_t state;
	uint16_t errors;
	uint16_t minor_rev_level;
	uint32_t lastcheck;
	uint32_t checkinterval;
	uint32_t creator_os;
	uint32_t rev_level;
	uint16_t def_resuid;
	uint16_t def_resgid;
	/* major >= 1 */
	uint32_t first_ino;
	uint16_t inode_size;
	uint16_t block_group_nr;
	uint32_t feature_compat;
	uint32_t feature_incompat;
	uint32_t feature_ro_compat;
	uint8_t uuid[16];
	uint8_t volume_name[16];
	uint8_t last_mounted[64];
	uint32_t algo_bitmap;
	uint8_t prealloc_blocks;
	uint8_t prealloc_dir_blocks;
	uint16_t alignment_206;
	uint8_t journal_uuid[16];
	uint32_t journal_inum;
	uint32_t journal_dev;
	uint32_t last_orphan;
	uint32_t hash_seed[4];
	uint8_t def_hash_version;
	uint8_t alignment_253[3];
	uint32_t default_mount_options;
	uint32_t first_meta_bg;
	uint8_t padding[760];
};

#define EXT2_VALID_FS 1
#define EXT2_ERROR_FS 2

#define EXT2_ERRORS_CONTINUE 1
#define EXT2_ERRORS_RO       2
#define EXT2_ERRORS_PANIC    3

#define EXT2_OS_LINUX   0
#define EXT2_OS_HURD    1
#define EXT2_OS_MASIX   2
#define EXT2_OS_FREEBSD 3
#define EXT2_OS_LITES   4

#define EXT2_GOOD_OLD_REV 0
#define EXT2_DYNAMIC_REV  1

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC  0x1
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES 0x2
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL   0x4
#define EXT2_FEATURE_COMPAT_EXT_ATTR      0x8
#define EXT2_FEATURE_COMPAT_RESIZE_INO    0x10
#define EXT2_FEATURE_COMPAT_DIR_INDEX     0x20

#define EXT2_FEATURE_INCOMPAT_COMPRESSION 0x1
#define EXT2_FEATURE_INCOMPAT_FILETYPE    0x2
#define EXT3_FEATURE_INCOMPAT_RECOVER     0x4
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV 0x8
#define EXT2_FEATURE_INCOMPAT_META_BG     0x10

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x1
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE   0x2
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR    0x4

#define EXT2_LZV1_ALG   0x0
#define EXT2_LZRW3A_ALG 0x1
#define EXT2_GZIP_ALG   0x2
#define EXT2_BZIP2_ALG  0x4
#define EXT2_LZO_ALG    0x8

struct ext2_group_desc
{
	uint32_t block_bitmap;
	uint32_t inode_bitmap;
	uint32_t inode_table;
	uint16_t free_blocks_count;
	uint16_t free_inodes_count;
	uint16_t used_dirs_count;
	uint16_t pad;
	uint8_t reserved[12];
};

struct ext2_inode
{
	uint16_t mode;
	uint16_t uid;
	uint32_t size;
	uint32_t atime;
	uint32_t ctime;
	uint32_t mtime;
	uint32_t dtime;
	uint16_t gid;
	uint16_t links_count;
	uint32_t blocks;
	uint32_t flags;
	uint32_t osd1;
	uint32_t block[15];
	uint32_t generation;
	uint32_t file_acl;
	uint32_t dir_acl;
	uint32_t faddr;
	uint32_t osd2[3];
};

#define EXT2_SECRM_FL        0x0001
#define EXT2_UNRM_FL         0x0002
#define EXT2_COMPR_FL        0x0004
#define EXT2_SYNC_FL         0x0008
#define EXT2_IMMUTABLE_FL    0x0010
#define EXT2_APPEND_FL       0x0020
#define EXT2_NODUMP_FL       0x0040
#define EXT2_NOATIME_FL      0x0080
#define EXT2_DIRTY_FL        0x0100
#define EXT2_COMPRBLK_FL     0x0200
#define EXT2_NOCOMPR_FL      0x0400
#define EXT2_ECOMPR_FL       0x0800
#define EXT2_BTREE_FL        0x1000
#define EXT2_INDEX_FL        0x1000
#define EXT2_IMAGIC_FL       0x2000
#define EXT3_JOURNAL_DATA_FL 0x4000

#define EXT2_BAD_INO         1
#define EXT2_ROOT_INO        2
#define EXT2_ACL_IDX_INO     3
#define EXT2_ACL_DATA_INO    4
#define EXT2_BOOT_LOADER_INO 5
#define EXT2_UNDEL_DIR_INO   6

#define EXT2_FT_UNKNOWN  0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR      2
#define EXT2_FT_CHRDEV   3
#define EXT2_FT_BLKDEV   4
#define EXT2_FT_FIFO     5
#define EXT2_FT_SOCK     6
#define EXT2_FT_SYMLINK  7

struct ext2_dirent
{
	uint32_t inode;
	uint16_t rec_len;
	uint8_t name_len;
	uint8_t file_type;
	char name[];
};

struct ext2_fs
{
	struct fs_sb *sb;
	struct ext2_sb ext2sb;
	struct file *file;
	uint32_t bgdt_off;
	uint32_t blksz;
	uint32_t blkmask;
	uint32_t blk_per_blk;
	uint32_t groups_count;
};

struct ext2_node
{
	struct node node;
	struct node *parent;
	struct ext2_inode inode;
};

int read_block(struct ext2_fs *fs, void *data, uint32_t id);
int write_block(struct ext2_fs *fs, const void *data, uint32_t id);
int read_disk_data(struct ext2_fs *fs, void *data, size_t count, off_t off);
int write_disk_data(struct ext2_fs *fs, void *data, size_t count, off_t off);
int alloc_block(struct ext2_fs *fs, uint32_t *blkid);
int alloc_block_zero(struct ext2_fs *fs, uint32_t *blkid);
int free_block(struct ext2_fs *fs, uint32_t blkid);
int write_sb(struct ext2_fs *fs);
int group_alloc_inode(struct ext2_fs *fs, uint32_t grpid, ino_t *ino);
int get_node(struct ext2_fs *fs, uint32_t ino, struct ext2_node **nodep);
int fs_mknode(struct ext2_fs *fs, ino_t ino, fs_attr_mask_t mask,
              const struct fs_attr *attr, dev_t rdev,
              struct ext2_node **nodep);
int read_group_desc(struct ext2_fs *fs, uint32_t id,
                    struct ext2_group_desc *group_desc);
int write_group_desc(struct ext2_fs *fs, uint32_t id,
                     struct ext2_group_desc *group_desc);

int dir_lookup(struct node *node, const char *name, size_t name_len,
               struct node **childp);
int dir_readdir(struct node *node, struct fs_readdir_ctx *ctx);
int dir_add_dirent(struct ext2_node *node, const char *name,
                   size_t name_len, ino_t ino, struct ext2_inode *inode);
int dir_mknode(struct node *node, const char *name, size_t name_len,
               fs_attr_mask_t mask, const struct fs_attr *attr,
               dev_t rdev);

int read_node_block(struct ext2_fs *fs, struct ext2_node *node,
                    uint32_t id, void *data);
int node_truncate(struct ext2_node *node, off_t size);
ssize_t node_read(struct ext2_node *node, struct uio *uio);
ssize_t node_write(struct ext2_node *node, struct uio *uio);
int update_node_inode(struct ext2_node *node);
int alloc_inode(struct ext2_fs *fs, ino_t *ino);
int free_inode(struct ext2_fs *fs, ino_t ino);
int write_inode(struct ext2_fs *fs, uint32_t ino, struct ext2_inode *inode);
int read_inode(struct ext2_fs *fs, uint32_t ino, struct ext2_inode *inode);

#endif
