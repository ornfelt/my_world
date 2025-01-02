#include <wchar.h>

int swprintf(wchar_t *d, size_t n, const wchar_t *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vswprintf(d, n, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
