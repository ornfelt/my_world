#include <stdio.h>
#include <wchar.h>

int
wprintf(const wchar_t *fmt, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, fmt);
	ret = vfwprintf(stdout, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
