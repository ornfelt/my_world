#include "../_syscall.h"

#include <sys/stat.h>

int
mkdirat(int dirfd, const char *pathname, mode_t mode)
{
	return syscall4(SYS_mknodat,
	                dirfd,
	                (uintptr_t)pathname,
	                (mode & 07777) | S_IFDIR,
	                0);
}
