#include "../_syscall.h"

#include <unistd.h>

int ftruncateat(int dirfd, const char *pathname, off_t length, int flags)
{
	return syscall4(SYS_ftruncateat, dirfd, (uintptr_t)pathname,
	                (uintptr_t)&length, flags);
}
