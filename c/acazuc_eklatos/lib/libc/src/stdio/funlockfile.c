#include "_stdio.h"

#include <stdio.h>

void
funlockfile(FILE *fp)
{
	_libc_unlock(&fp->lock);
}
