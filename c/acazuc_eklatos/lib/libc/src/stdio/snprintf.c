#include "../_chk.h"

#include <stdio.h>

#ifdef snprintf
#undef snprintf
#endif

int
snprintf(char *d, size_t n, const char *fmt, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, fmt);
	ret = vsnprintf(d, n, fmt, va_arg);
	va_end(va_arg);
	return ret;
}

int
__snprintf_chk(char *d, size_t n, int flag, size_t ds, const char *fmt, ...)
{
	size_t lim = n > ds ? ds : n;
	va_list va_arg;
	int ret;

	(void)flag;
	va_start(va_arg, fmt);
	ret = vsnprintf(d, lim, fmt, va_arg);
	va_end(va_arg);
	if (ds < n && ret >= 0 && (size_t)ret >= ds)
		__chk_fail();
	return ret;
}
