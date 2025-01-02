#include "../_syscall.h"

#include <sys/mman.h>

int munmap(void *addr, size_t len)
{
	return syscall2(SYS_munmap, (uintptr_t)addr, len);
}
