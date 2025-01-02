#include <stdio.h>

#ifdef fprintf
#undef fprintf
#endif

int fprintf(FILE *fp, const char *fmt, ...)
{
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vfprintf(fp, fmt, va_arg);
	va_end(va_arg);
	return ret;
}

int __fprintf_chk(FILE *fp, int flag, const char *fmt, ...)
{
	(void)flag;
	va_list va_arg;
	va_start(va_arg, fmt);
	int ret = vfprintf(fp, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
