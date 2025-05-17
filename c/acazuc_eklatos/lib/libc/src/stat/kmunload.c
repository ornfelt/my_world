#include "../_syscall.h"

#include <sys/stat.h>

int
kmunload(const char *name, int flags)
{
	return syscall2(SYS_kmunload, (uintptr_t)name, flags);
}
