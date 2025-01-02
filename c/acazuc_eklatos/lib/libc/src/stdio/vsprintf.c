#include "../_chk.h"

#include <limits.h>
#include <stdio.h>

#ifdef vsprintf
#undef vsprintf
#endif

int vsprintf(char *d, const char *fmt, va_list va_arg)
{
	return vsnprintf(d, INT_MAX, fmt, va_arg);
}

int __vsprintf_chk(char *d, int flag, size_t ds, const char *fmt,
                   va_list va_arg)
{
	(void)flag;
	int ret = vsnprintf(d, ds, fmt, va_arg);
	if (ret >= 0 && (size_t)ret >= ds)
		__chk_fail();
	return ret;
}
