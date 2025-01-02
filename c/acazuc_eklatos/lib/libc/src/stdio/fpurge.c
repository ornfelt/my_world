#include "_stdio.h"

#include <stdio.h>

int fpurge(FILE *fp)
{
	flockfile(fp);
	fp->buf_pos = 0;
	funlockfile(fp);
	return 0;
}
