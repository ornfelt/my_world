#include "../_chk.h"
#include "_printf.h"

#include <stdio.h>

#ifdef vsnprintf
#undef vsnprintf
#endif

int
vsnprintf(char *d, size_t n, const char *fmt, va_list va_arg)
{
	struct buf buf;
	int ret;

	buf.type = PRINTF_BUF;
	buf.data = d;
	buf.size = n;
	ret = printf_buf(&buf, fmt, va_arg);
	if (buf.size)
	{
		if (buf.len < buf.size)
			buf.data[buf.len] = '\0';
		else
			buf.data[buf.size - 1] = '\0';
	}
	return ret;
}

int
__vsnprintf_chk(char *d,
                size_t n,
                int flag,
                size_t ds,
                const char *fmt,
                va_list va_arg)
{
	size_t lim = n > ds ? ds : n;
	int ret;

	(void)flag;
	ret = vsnprintf(d, lim, fmt, va_arg);
	if (ds < n && ret >= 0 && (size_t)ret >= ds)
		__chk_fail();
	return ret;
}
