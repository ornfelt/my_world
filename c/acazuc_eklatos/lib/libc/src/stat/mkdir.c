#include "../_syscall.h"

#include <sys/stat.h>
#include <fcntl.h>

int mkdir(const char *pathname, mode_t mode)
{
	return syscall4(SYS_mknodat, AT_FDCWD, (uintptr_t)pathname,
	                (mode & 07777) | S_IFDIR, 0);
}
