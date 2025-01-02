#include "_stdio.h"

#include <stdio.h>

FILE *fdopen(int fd, const char *mode)
{
	int flags;
	if (!parse_flags(mode, &flags))
		return NULL;
	FILE *fp = mkfp();
	if (!fp)
		return NULL;
	fp->fd = fd;
	fp->mode = flags;
	fp->cookie = fp;
	fp->io_funcs.read = io_read;
	fp->io_funcs.write = io_write;
	fp->io_funcs.seek = io_seek;
	fp->io_funcs.close = io_close;
	TAILQ_INSERT_TAIL(&files, fp, chain);
	return fp;
}
