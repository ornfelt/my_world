#include "_stdio.h"

#include <stdio.h>

int ftrylockfile(FILE *fp)
{
	return !_libc_trylock(&fp->lock);
}
