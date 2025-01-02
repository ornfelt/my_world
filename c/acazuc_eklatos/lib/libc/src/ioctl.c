#include "_syscall.h"

#include <sys/ioctl.h>

#include <stdarg.h>

int ioctl(int fd, unsigned long request, ...)
{
	va_list va_arg;
	va_start(va_arg, request);
	int res = syscall3(SYS_ioctl, fd, request, va_arg(va_arg, uintptr_t));
	va_end(va_arg);
	return res;
}
