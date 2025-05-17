#include "../_syscall.h"

#include <sys/mman.h>

int
msync(void *addr, size_t len, int flags)
{
	return syscall3(SYS_msync, (uintptr_t)addr, len, flags);
}
