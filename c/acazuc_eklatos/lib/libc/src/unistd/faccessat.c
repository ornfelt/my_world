#include "../_syscall.h"

#include <unistd.h>

int
faccessat(int dirfd, const char *pathname, int mode, int flags)
{
	return syscall4(SYS_faccessat, dirfd, (uintptr_t)pathname, mode, flags);
}
