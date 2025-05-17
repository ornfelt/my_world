#include "../_syscall.h"

#include <sys/stat.h>

int
fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags)
{
	return syscall4(SYS_fstatat,
	                dirfd,
	                (uintptr_t)pathname,
	                (uintptr_t)statbuf,
	                flags);
}
