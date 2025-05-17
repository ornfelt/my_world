#include <wchar.h>

int
fwprintf(FILE *fp, const wchar_t *fmt, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, fmt);
	ret = vfwprintf(fp, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
