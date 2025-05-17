#include "_stdio.h"

#include <stdio.h>

int
fcloseall(void)
{
	FILE *f;
	FILE *nxt;
	int err = 0;

	TAILQ_FOREACH_SAFE(f, &files, chain, nxt)
	{
		if (fclose(f))
			err = EOF;
	}
	if (fclose(stdin) || fclose(stdout) || fclose(stderr))
		err = EOF;
	return err;
}
