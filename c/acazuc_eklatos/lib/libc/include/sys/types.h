#ifndef SYS_TYPES_H
#define SYS_TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t ssize_t;
typedef uint64_t fsfilcnt_t;
typedef uint64_t fsblkcnt_t;
typedef int64_t time_t;
typedef int64_t off_t;
typedef uint64_t ino_t;
typedef uint32_t mode_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef int32_t pid_t;
typedef uint32_t dev_t;
typedef uint32_t nlink_t;
typedef int64_t blksize_t;
typedef int64_t blkcnt_t;
typedef uint32_t pri_t;
typedef ssize_t clock_t;
typedef int32_t id_t;
typedef int32_t clockid_t;
typedef size_t key_t;
typedef size_t msgqnum_t;
typedef size_t msglen_t;
typedef size_t shmatt_t;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned long long u_quad;

#define major(n) (((n) >> 16) & 0xFFFF)
#define minor(n) ((n) & 0xFFFF)
#define makedev(ma, mi) (((ma & 0xFFFF) << 16) | ((mi) & 0xFFFF))

#ifdef __cplusplus
}
#endif

#endif
