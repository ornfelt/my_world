#include <stdio.h>

int sscanf(const char *str, const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vsscanf(str, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
