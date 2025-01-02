#include "../_syscall.h"

#include <fcntl.h>

int creat(const char *pathname, mode_t mode)
{
	return syscall4(SYS_openat, AT_FDCWD, (uintptr_t)pathname,
	                O_CREAT | O_WRONLY | O_TRUNC, mode);
}
