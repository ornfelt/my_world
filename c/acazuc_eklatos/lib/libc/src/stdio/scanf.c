#include <stdio.h>

int scanf(const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vfscanf(stdin, fmt, va_arg);
	va_end(va_arg);
	return ret;
}