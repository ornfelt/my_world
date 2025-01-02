#include "_stdio.h"

#include <stdlib.h>
#include <stdio.h>

int fclose(FILE *fp)
{
	flockfile(fp);
	int err = 0;
	if (fflush(fp))
		err = EOF;
	if (fp->io_funcs.close && fp->io_funcs.close(fp->cookie))
		err = EOF;
	funlockfile(fp);
	if (fp != stdin && fp != stdout && fp != stderr)
	{
		TAILQ_REMOVE(&files, fp, chain);
		if (fp->buf_owned)
			free(fp->buf);
		free(fp);
	}
	return err;
}
