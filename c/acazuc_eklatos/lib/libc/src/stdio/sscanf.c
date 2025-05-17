#include <stdio.h>

int
sscanf(const char *str, const char *fmt, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, fmt);
	ret = vsscanf(str, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
