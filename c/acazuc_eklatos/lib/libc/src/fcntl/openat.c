#include "../_syscall.h"

#include <stdarg.h>
#include <fcntl.h>

int openat(int dirfd, const char *pathname, int flags, ...)
{
	mode_t mode;
	va_list va_arg;
	va_start(va_arg, flags);
	if (flags & O_CREAT)
		mode = va_arg(va_arg, mode_t);
	else
		mode = 0;
	va_end(va_arg);
	return syscall4(SYS_openat, dirfd, (uintptr_t)pathname, flags, mode);
}
