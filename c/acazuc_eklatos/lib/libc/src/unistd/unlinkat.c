#include "../_syscall.h"

#include <unistd.h>

int unlinkat(int dirfd, const char *pathname, int flags)
{
	return syscall3(SYS_unlinkat, dirfd, (uintptr_t)pathname, flags);
}
