#include "_stdio.h"

#include <stdio.h>

int __flfb(FILE *fp)
{
	return fp->buf_mode == _IOLBF;
}
