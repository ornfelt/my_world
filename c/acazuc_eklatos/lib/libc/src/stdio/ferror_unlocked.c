#include "_stdio.h"

#include <stdio.h>

int
ferror_unlocked(FILE *fp)
{
	return fp->err;
}
