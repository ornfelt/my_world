#include "_printf.h"

#include <stdio.h>

#ifdef vfprintf
#undef vfprintf
#endif

int
vfprintf(FILE *fp, const char *fmt, va_list va_arg)
{
	struct buf buf;
	int ret;

	buf.type = PRINTF_FP;
	buf.fp = fp;
	flockfile(fp);
	ret = printf_buf(&buf, fmt, va_arg);
	funlockfile(fp);
	return ret;
}

int
__vfprintf_chk(FILE *fp, int flag, const char *fmt, va_list va_arg)
{
	struct buf buf;
	int ret;

	(void)flag;
	buf.type = PRINTF_FP;
	buf.fp = fp;
	flockfile(fp);
	ret = printf_buf(&buf, fmt, va_arg);
	funlockfile(fp);
	return ret;
}
