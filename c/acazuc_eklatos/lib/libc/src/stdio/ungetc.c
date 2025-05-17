#include "_stdio.h"

#include <stdio.h>

int
ungetc(int c, FILE *fp)
{
	int ret;

	flockfile(fp);
	ret = ungetc_unlocked(c, fp);
	funlockfile(fp);
	return ret;
}
