#include "_atexit.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void exit(int status)
{
	/* XXX rm tmpfile */
	for (size_t i = 0; i < g_atexit_fn_nb; ++i)
		g_atexit_fn[i]();
	fcloseall();
	exit_group(status);
}
