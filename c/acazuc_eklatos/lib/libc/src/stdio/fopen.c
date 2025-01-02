#include "_stdio.h"

#include <stdlib.h>
#include <stdio.h>

FILE *fopen(const char *pathname, const char *mode)
{
	FILE *fp = mkfp();
	if (!fp)
		return NULL;
	int flags;
	if (!parse_flags(mode, &flags))
	{
		free(fp);
		return NULL;
	}
	fp->fd = open(pathname, flags, 0666);
	if (fp->fd == -1)
	{
		free(fp);
		return NULL;
	}
	fp->mode = flags;
	fp->cookie = fp;
	fp->io_funcs.read = io_read;
	fp->io_funcs.write = io_write;
	fp->io_funcs.seek = io_seek;
	fp->io_funcs.close = io_close;
	TAILQ_INSERT_TAIL(&files, fp, chain);
	return fp;
}
