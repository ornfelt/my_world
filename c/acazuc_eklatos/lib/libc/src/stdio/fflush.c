#include "_stdio.h"

#include <stdio.h>

int fflush(FILE *fp)
{
	if (!fp)
	{
		int res = 0;
		FILE *f;
		TAILQ_FOREACH(f, &files, chain)
		{
			if (fflush(f))
				res = EOF;
		}
		return res;
	}
	flockfile(fp);
	int ret = fflush_unlocked(fp);
	funlockfile(fp);
	return ret;
}
