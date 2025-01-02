#include <wchar.h>

int fwprintf(FILE *fp, const wchar_t *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vfwprintf(fp, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
