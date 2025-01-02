#ifndef SYS_STATVFS_H
#define SYS_STATVFS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

int statvfs(const char *pathname, struct statvfs *buf);
int fstatvfs(int fd, struct statvfs *buf);
int fstatvfsat(int dirfd, const char *pathname, struct statvfs *buf, int flags);

#ifdef __cplusplus
}
#endif

#endif
