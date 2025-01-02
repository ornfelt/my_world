#include "../_syscall.h"

#include <sys/statvfs.h>

int fstatvfsat(int dirfd, const char *pathname, struct statvfs *buf, int flags)
{
	return syscall4(SYS_fstatvfsat, dirfd, (uintptr_t)pathname,
	                (uintptr_t)buf, flags);
}
