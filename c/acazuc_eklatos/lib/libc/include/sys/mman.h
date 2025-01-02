#ifndef MMAN_H
#define MMAN_H

#include <sys/types.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_ANONYMOUS       (1 << 0)
#define MAP_ANON            MAP_ANONYMOUS
#define MAP_SHARED          (1 << 1)
#define MAP_PRIVATE         (1 << 2)
#define MAP_FIXED           (1 << 3)
#define MAP_FIXED_NOREPLACE (1 << 4)
#define MAP_EXCL            MAP_FIXED_NOREPLACE
#define MAP_POPULATE        (1 << 5)
#define MAP_PREFAULT_READ   MAP_POPULATE
#define MAP_NORESERVE       (1 << 6)
#define MAP_ALIGNED(n)      ((n) << 7)

#define PROT_NONE  0
#define PROT_EXEC  (1 << 0)
#define PROT_READ  (1 << 1)
#define PROT_WRITE (1 << 2)

#define MS_ASYNC      (1 << 0)
#define MS_SYNC       (1 << 1)
#define MS_INVALIDATE (1 << 2)

#define MAP_FAILED ((void*)-1)

#define MADV_NORMAL   0
#define MADV_DONTNEED 1

void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off);
int munmap(void *addr, size_t len);
int mprotect(void *addr, size_t len, int prot);
int msync(void *addr, size_t len, int flags);
int madvise(void *addr, size_t len, int advise);

#ifdef __cplusplus
}
#endif

#endif
