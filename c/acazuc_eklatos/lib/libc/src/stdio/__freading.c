#include "_stdio.h"

#include <stdio.h>

int __freading(FILE *fp)
{
	return fp->mode == O_RDONLY;
}
