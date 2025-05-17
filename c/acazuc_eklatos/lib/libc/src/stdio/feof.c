#include "_stdio.h"

#include <stdio.h>

int
feof(FILE *fp)
{
	int ret;

	flockfile(fp);
	ret = fp->eof;
	funlockfile(fp);
	return ret;
}
