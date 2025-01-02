#ifndef TYPES_H
#define TYPES_H

typedef __INT8_TYPE__ int8_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __INT16_TYPE__ int16_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __INT32_TYPE__ int32_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __INT64_TYPE__ int64_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __INTMAX_TYPE__ intmax_t;
typedef __UINTMAX_TYPE__ uintmax_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __SIZE_TYPE__ size_t;
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
typedef uint32_t socklen_t;
typedef uint16_t sa_family_t;

#define major(n) (((n) >> 16) & 0xFFFF)
#define minor(n) ((n) & 0xFFFF)
#define makedev(ma, mi) (((ma & 0xFFFF) << 16) | ((mi) & 0xFFFF))

#endif
