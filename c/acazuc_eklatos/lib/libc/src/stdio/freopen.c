#include "_stdio.h"

#include <stdio.h>

FILE *
freopen(const char *pathname, const char *mode, FILE *fp)
{
	int flags;
	int fd;

	if (!pathname) /* not supported */
		return fp;
	if (!parse_flags(mode, &flags))
		return NULL;
	fd = open(pathname, flags, 0666);
	if (fd == -1)
		return NULL;
	fflush(fp);
	if (fp->io_funcs.close)
		fp->io_funcs.close(fp->cookie);
	initfp(fp);
	fp->fd = fd;
	fp->mode = flags;
	fp->cookie = fp;
	fp->io_funcs.read = io_read;
	fp->io_funcs.write = io_write;
	fp->io_funcs.seek = io_seek;
	fp->io_funcs.close = io_close;
	return fp;
}
