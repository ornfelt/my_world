#include <stdio.h>

int fscanf(FILE *fp, const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vfscanf(fp, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
