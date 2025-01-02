#include "_stdio.h"

#include <stdio.h>

int ferror(FILE *fp)
{
	flockfile(fp);
	int ret = fp->err;
	funlockfile(fp);
	return ret;
}
