#include "_stdio.h"

#include <stdio.h>

int ungetc(int c, FILE *fp)
{
	flockfile(fp);
	int ret = ungetc_unlocked(c, fp);
	funlockfile(fp);
	return ret;
}
