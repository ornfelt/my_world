#include "_stdio.h"

#include <stdio.h>

void
_flushlfb(void)
{
	FILE *fp;

	TAILQ_FOREACH(fp, &files, chain)
	{
		if (fp->buf_mode == _IOLBF)
			fflush(fp);
	}
}
