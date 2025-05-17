#include <stdio.h>

int
dprintf(int fd, const char *fmt, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, fmt);
	ret = vdprintf(fd, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
