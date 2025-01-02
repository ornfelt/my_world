#ifndef FCNTL_H
#define FCNTL_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_APPEND    (1 << 2)
#define O_ASYNC     (1 << 3)
#define O_CREAT     (1 << 4)
#define O_TRUNC     (1 << 5)
#define O_NOFOLLOW  (1 << 6)
#define O_DIRECTORY (1 << 7)
#define O_EXCL      (1 << 8)
#define O_NONBLOCK  (1 << 9)
#define O_CLOEXEC   (1 << 10)
#define O_NOCTTY    (1 << 11)

#define AT_FDCWD -100

#define AT_SYMLINK_NOFOLLOW (1 << 0)
#define AT_REMOVEDIR        (1 << 1)
#define AT_SYMLINK_FOLLOW   (1 << 2)
#define AT_EMPTY_PATH       (1 << 3)
#define AT_EACCESS          (1 << 4)

#define F_DUPFD  0
#define F_GETFD  1
#define F_SETFD  2
#define F_GETFL  3
#define F_SETFL  4
#define F_SETLK  5
#define F_SETLKW 6
#define F_GETLK  7

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

#define FD_CLOEXEC (1 << 0)

struct flock
{
	int16_t l_type;
	int16_t l_whence;
	off_t l_start;
	off_t l_len;
	pid_t l_pid;
};

int open(const char *pathname, int flags, ...);
int creat(const char *pathname, mode_t mode);
int openat(int dirfd, const char *pathname, int flags, ...);

int fcntl(int fd, int cmd, ...);

#ifdef __cplusplus
}
#endif

#endif
