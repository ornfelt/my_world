#include <stdio.h>

#ifdef vprintf
#undef vprintf
#endif

int
vprintf(const char *fmt, va_list va_arg)
{
	return vfprintf(stdout, fmt, va_arg);
}

int
__vprintf_chk(int flag, const char *fmt, va_list va_arg)
{
	(void)flag;
	return vfprintf(stdout, fmt, va_arg);
}
