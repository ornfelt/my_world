#include <stdio.h>

#ifdef fprintf
#undef fprintf
#endif

int
fprintf(FILE *fp, const char *fmt, ...)
{
	va_list va_arg;
	int ret;

	va_start(va_arg, fmt);
	ret = vfprintf(fp, fmt, va_arg);
	va_end(va_arg);
	return ret;
}

int
__fprintf_chk(FILE *fp, int flag, const char *fmt, ...)
{
	va_list va_arg;
	int ret;

	(void)flag;
	va_start(va_arg, fmt);
	ret = vfprintf(fp, fmt, va_arg);
	va_end(va_arg);
	return ret;
}
