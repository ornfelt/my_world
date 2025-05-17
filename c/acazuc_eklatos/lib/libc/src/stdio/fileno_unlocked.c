#include "_stdio.h"

#include <stdio.h>
#include <errno.h>

int
fileno_unlocked(FILE *fp)
{
	if (!fp)
	{
		errno = EBADF;
		return -1;
	}
	return fp->fd;
}
