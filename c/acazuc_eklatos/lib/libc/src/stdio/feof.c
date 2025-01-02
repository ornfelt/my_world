#include "_stdio.h"

#include <stdio.h>

int feof(FILE *fp)
{
	flockfile(fp);
	int ret = fp->eof;
	funlockfile(fp);
	return ret;
}
