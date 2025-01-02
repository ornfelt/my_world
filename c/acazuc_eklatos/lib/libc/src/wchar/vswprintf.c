#include "_wprintf.h"

#include <wchar.h>

int vswprintf(wchar_t *d, size_t n, const wchar_t *fmt, va_list va_arg)
{
	struct buf buf;
	buf.type = PRINTF_BUF;
	buf.data = d;
	buf.size = n;
	int ret = wprintf_buf(&buf, fmt, va_arg);
	if (buf.size)
	{
		if (buf.len < buf.size)
			buf.data[buf.len] = L'\0';
		else
			buf.data[buf.size - 1] = L'\0';
	}
	return ret;
}
