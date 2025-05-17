#include "_scanf.h"

#include <stdio.h>

int
vfscanf(FILE *fp, const char *fmt, va_list va_arg)
{
	struct buf buf;
	int ret;

	buf.type = SCANF_FP;
	buf.fp = fp;
	flockfile(fp);
	ret = scanf_buf(&buf, fmt, va_arg);
	funlockfile(fp);
	return ret;
}
