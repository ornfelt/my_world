#include "_stdio.h"

#include <stdio.h>
#include <errno.h>

int fileno(FILE *fp)
{
	if (!fp)
	{
		errno = EBADF;
		return -1;
	}
	flockfile(fp);
	int ret = fp->fd;
	funlockfile(fp);
	return ret;
}
