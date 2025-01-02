#include "_stdio.h"

#include <stdio.h>

int __freadable(FILE *fp)
{
	return fp->mode == O_RDONLY;
}
