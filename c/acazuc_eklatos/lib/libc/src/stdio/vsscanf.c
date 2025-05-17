#include "_scanf.h"

#include <stdio.h>

int
vsscanf(const char *str, const char *fmt, va_list va_arg)
{
	struct buf buf;

	buf.type = SCANF_STR;
	buf.str = str;
	return scanf_buf(&buf, fmt, va_arg);
}
