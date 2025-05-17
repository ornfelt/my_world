#include "_stdio.h"

#include <stdio.h>

int
__fwriting(FILE *fp)
{
	return fp->mode == O_WRONLY
	    || fp->mode == O_RDWR;
}
