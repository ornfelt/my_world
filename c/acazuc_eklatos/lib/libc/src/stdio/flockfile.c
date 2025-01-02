#include "_stdio.h"

#include <stdio.h>

void flockfile(FILE *fp)
{
	_libc_lock(&fp->lock);
}
