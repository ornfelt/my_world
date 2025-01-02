#include "_scanf.h"

#include <stdio.h>

int vfscanf(FILE *fp, const char *fmt, va_list va_arg)
{
	struct buf buf;
	buf.type = SCANF_FP;
	buf.fp = fp;
	flockfile(fp);
	int ret = scanf_buf(&buf, fmt, va_arg);
	funlockfile(fp);
	return ret;
}
