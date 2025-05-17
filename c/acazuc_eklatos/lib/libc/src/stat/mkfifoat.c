#include "../_syscall.h"

#include <sys/stat.h>

int
mkfifoat(int dirfd, const char *pathname, mode_t mode)
{
	return syscall3(SYS_mknodat,
	                dirfd,
	                (uintptr_t)pathname,
	                (mode & 07777) | S_IFIFO);
}
