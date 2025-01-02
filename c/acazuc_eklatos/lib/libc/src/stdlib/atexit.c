#include "_atexit.h"

#include <stdlib.h>

int atexit(void (*function)(void))
{
	if (g_atexit_fn_nb >= ATEXIT_MAX)
		return 1;
	g_atexit_fn[g_atexit_fn_nb++] = function;
	return 0;
}
