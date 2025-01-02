#include "../_syscall.h"

#include <sys/mman.h>

int madvise(void *addr, size_t len, int advise)
{
	return syscall3(SYS_madvise, (uintptr_t)addr, len, advise);
}
