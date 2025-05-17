#include "_stdio.h"

#include <stdio.h>

void
clearerr_unlocked(FILE *fp)
{
	fp->eof = 0;
	fp->err = 0;
}
