#include "../_syscall.h"

#include <sys/stat.h>
#include <fcntl.h>

int
mkfifo(const char *pathname, mode_t mode)
{
	return syscall3(SYS_mknodat,
	                AT_FDCWD,
	                (uintptr_t)pathname,
	                (mode & 07777) | S_IFIFO);
}
