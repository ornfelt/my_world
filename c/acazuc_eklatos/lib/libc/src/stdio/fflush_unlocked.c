#include "_stdio.h"

#include <string.h>
#include <stdio.h>

int
fflush_unlocked(FILE *fp)
{
	int res = 0;
	FILE *f;

	if (!fp)
	{
		TAILQ_FOREACH(f, &files, chain)
		{
			if (fflush_unlocked(f))
				res = EOF;
		}
		return res;
	}
	if (fp->buf_type)
	{
		if (fp->buf_pos)
		{
			size_t wr;

			wr = write_data(fp, fp->buf, fp->buf_pos);
			if (wr != fp->buf_pos)
				return EOF;
			fp->buf_pos = 0;
		}
	}
	else
	{
		fp->buf_pos = 0;
	}
	if (fp->flush && fp->flush(fp) == -1)
		return EOF;
	return 0;
}
