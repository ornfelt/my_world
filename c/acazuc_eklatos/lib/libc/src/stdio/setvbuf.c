#include "_stdio.h"

#include <stdlib.h>
#include <stdio.h>

int setvbuf(FILE *fp, char *buf, int mode, size_t size)
{
	if (mode != _IOFBF && mode != _IONBF && mode != _IOLBF)
		return EOF;
	int owned;
	if (!buf && size)
	{
		buf = malloc(size);
		if (!buf)
			return EOF;
		owned = 1;
	}
	else
	{
		owned = 0;
	}
	flockfile(fp);
	if (fp->buf_owned)
		free(fp->buf);
	fp->buf_mode = mode;
	fp->buf_pos = 0;
	fp->buf_size = size;
	fp->buf = buf;
	fp->buf_owned = owned;
	funlockfile(fp);
	return 0;
}
