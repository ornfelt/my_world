#include "../_syscall.h"

#include <sys/mman.h>

int mprotect(void *addr, size_t len, int prot)
{
	return syscall3(SYS_mprotect, (uintptr_t)addr, len, prot);
}
