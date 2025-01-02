#include "_printf.h"

#include <stdio.h>

#ifdef printf
#undef printf
#endif

int printf(const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vfprintf(stdout, fmt, va_arg);
	va_end(va_arg);
	return ret;
}

int __printf_chk(int flag, const char *fmt, ...)
{
	(void)flag;
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vfprintf(stdout, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
