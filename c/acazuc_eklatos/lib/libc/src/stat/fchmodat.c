#include "../_syscall.h"

#include <sys/stat.h>

int
fchmodat(int dirfd, const char *pathname, mode_t mode, int flags)
{
	return syscall4(SYS_fchmodat, dirfd, (uintptr_t)pathname, mode, flags);
}
