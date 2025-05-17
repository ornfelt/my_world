#include "_stdio.h"

#include <stdio.h>

long
ftell(FILE *fp)
{
	long ret;

	flockfile(fp);
	if (!fp->io_funcs.seek)
	{
		ret = -1;
		goto end;
	}
	ret = fp->io_funcs.seek(fp->cookie, 0, SEEK_CUR);
	if (fp->buf_mode == _IONBF)
		goto end;
	if (fp->buf_type)
		ret += fp->buf_pos;
	else
		ret -= fp->buf_pos;

end:
	funlockfile(fp);
	return ret;
}
