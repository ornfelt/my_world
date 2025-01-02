#include <stdio.h>

int dprintf(int fd, const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vdprintf(fd, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
