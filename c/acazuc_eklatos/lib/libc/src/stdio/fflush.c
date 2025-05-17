#include "_stdio.h"

#include <stdio.h>

int
fflush(FILE *fp)
{
	int res = 0;
	FILE *f;
	int ret;

	if (!fp)
	{
		TAILQ_FOREACH(f, &files, chain)
		{
			if (fflush(f))
				res = EOF;
		}
		return res;
	}
	flockfile(fp);
	ret = fflush_unlocked(fp);
	funlockfile(fp);
	return ret;
}
