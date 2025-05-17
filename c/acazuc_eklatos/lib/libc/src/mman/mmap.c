#include "../_syscall.h"

#include <sys/mman.h>

void *
mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
	return (void*)syscall6(SYS_mmap,
	                       (uintptr_t)addr,
	                       len,
	                       prot,
	                       flags,
	                       fd,
	                       off ? (uintptr_t)&off : 0);
}
