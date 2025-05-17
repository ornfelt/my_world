#include "_stdio.h"

#include <stdio.h>
#include <errno.h>

int fileno(FILE *fp)
{
	int ret;

	if (!fp)
	{
		errno = EBADF;
		return -1;
	}
	flockfile(fp);
	ret = fp->fd;
	funlockfile(fp);
	return ret;
}
