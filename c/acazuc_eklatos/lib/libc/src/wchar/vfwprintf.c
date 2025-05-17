#include "_wprintf.h"

#include <wchar.h>

int
vfwprintf(FILE *fp, const wchar_t *fmt, va_list va_arg)
{
	struct buf buf;
	int ret;

	buf.type = PRINTF_FP;
	buf.fp = fp;
	flockfile(fp);
	ret = wprintf_buf(&buf, fmt, va_arg);
	funlockfile(fp);
	return ret;
}
