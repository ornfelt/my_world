#include "_stdio.h"

#include <stdio.h>

int fseek(FILE *fp, long offset, int whence)
{
	int ret;
	flockfile(fp);
	if (!fp->io_funcs.seek)
	{
		ret = -1;
		goto end;
	}
	if (whence != SEEK_CUR || fp->buf_mode == _IONBF)
	{
		if (fflush(fp)
		 || fp->io_funcs.seek(fp->cookie, offset, whence) == -1)
			ret = -1;
		else
			ret = 0;
		goto end;
	}
	/* XXX some optimizations could be done if dst is inside current buffer */
	long current = ftell(fp);
	if (current == -1
	 || fflush(fp)
	 || fp->io_funcs.seek(fp->cookie, current + offset, SEEK_SET) == -1)
		ret = -1;
	else
		ret = 0;

end:
	funlockfile(fp);
	return ret;
}
