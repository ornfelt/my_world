#include "../_syscall.h"

#include <unistd.h>

ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{
	return syscall4(SYS_readlinkat, dirfd, (uintptr_t)pathname,
	                (uintptr_t)buf, (uintptr_t)bufsiz);
}
