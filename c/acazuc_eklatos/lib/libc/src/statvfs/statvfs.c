#include "../_syscall.h"

#include <sys/statvfs.h>

#include <fcntl.h>

int
statvfs(const char *pathname, struct statvfs *buf)
{
	return syscall4(SYS_fstatvfsat,
	                AT_FDCWD,
	                (uintptr_t)pathname,
	                (uintptr_t)buf,
	                0);
}
