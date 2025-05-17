#include "_stdio.h"

#include <stdio.h>

int
ferror(FILE *fp)
{
	int ret;

	flockfile(fp);
	ret = fp->err;
	funlockfile(fp);
	return ret;
}
