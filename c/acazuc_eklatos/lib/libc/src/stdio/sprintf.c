#include "../_chk.h"

#include <stdio.h>

#ifdef sprintf
#undef sprintf
#endif

int sprintf(char *d, const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vsprintf(d, fmt, va_arg);
	va_end(va_arg);
	return ret;
}

int __sprintf_chk(char *d, int flag, size_t ds, const char *fmt, ...)
{
	(void)flag;
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vsnprintf(d, ds, fmt, va_arg);
	va_end(va_arg);
	if (ret >= 0 && (size_t)ret >= ds)
		__chk_fail();
	return ret;
}
