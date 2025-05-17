#include "_stdio.h"

#include <stdio.h>

void
clearerr(FILE *fp)
{
	flockfile(fp);
	fp->eof = 0;
	fp->err = 0;
	funlockfile(fp);
}
