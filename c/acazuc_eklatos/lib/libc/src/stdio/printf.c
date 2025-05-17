#include "_printf.h"

#include <stdio.h>

#ifdef printf
#undef printf
#endif

int
printf(const char *fmt, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, fmt);
	ret = vfprintf(stdout, fmt, va_arg);
	va_end(va_arg);
	return ret;
}

int
__printf_chk(int flag, const char *fmt, ...)
{
	va_list va_arg;
	int ret;

	(void)flag;
	va_start(va_arg, fmt);
	ret = vfprintf(stdout, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
