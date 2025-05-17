#include "../stdio/_printf.h"

#include <wchar.h>

int
vwprintf(const wchar_t *fmt, va_list va_arg)
{
	return vfwprintf(stdout, fmt, va_arg);
}
