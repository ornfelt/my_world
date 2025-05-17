#include <stdio.h>

int
scanf(const char *fmt, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, fmt);
	ret = vfscanf(stdin, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
