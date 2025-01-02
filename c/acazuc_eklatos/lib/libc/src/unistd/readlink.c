#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
	return syscall4(SYS_readlinkat, AT_FDCWD, (uintptr_t)pathname,
	                (uintptr_t)buf, (uintptr_t)bufsiz);
}
