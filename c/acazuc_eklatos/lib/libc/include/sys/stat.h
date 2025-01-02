#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <sys/types.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

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

int utimensat(int dirfd, const char *pathname, const struct timespec times[2],
              int flags);
int futimens(int fd, const struct timespec times[2]);
int mkdir(const char *pathname, mode_t mode);
int mkdirat(int dirfd, const char *pathname, mode_t mode);
int mknod(const char *pathname, mode_t mode, dev_t dev);
int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev);
int mkfifo(const char *pathname, mode_t mode);
int mkfifoat(int dirfd, const char *pathname, mode_t mode);
int chmod(const char *pathname, mode_t mode);
int fchmod(int fd, mode_t mode);
int lchmod(const char *pathname, mode_t mode);
int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
mode_t umask(mode_t mask);
int stat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);
int lstat(const char *pathname, struct stat *statbuf);
int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);

int kmload(int fd, const char *params, int flags);
int kmunload(const char *name, int flags);

#ifdef __cplusplus
}
#endif

#endif
