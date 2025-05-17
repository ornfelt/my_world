#include "_stdio.h"

#include <stdio.h>

FILE *
fopencookie(void *cookie, const char *mode, cookie_io_functions_t *funcs)
{
	FILE *fp;
	int flags;

	if (!parse_flags(mode, &flags))
		return NULL;
	fp = mkfp();
	if (!fp)
		return NULL;
	fp->mode = flags;
	fp->io_funcs.read = funcs ? funcs->read : NULL;
	fp->io_funcs.write = funcs ? funcs->write : NULL;
	fp->io_funcs.seek = funcs ? funcs->seek : NULL;
	fp->io_funcs.close = funcs ? funcs->close : NULL;
	fp->cookie = cookie;
	TAILQ_INSERT_TAIL(&files, fp, chain);
	return fp;
}
