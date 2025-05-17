#include <stdlib.h>
#include <errno.h>

#include "_atfork.h"

int
__libc_atfork(void (*prepare)(void),
              void (*parent)(void),
              void (*child)(void))
{
	if (prepare && g_atfork_prepare_nb >= ATFORK_MAX)
		return ENOMEM;
	if (parent && g_atfork_parent_nb >= ATFORK_MAX)
		return ENOMEM;
	if (child && g_atfork_child_nb >= ATFORK_MAX)
		return ENOMEM;
	if (prepare)
		g_atfork_prepare[g_atfork_prepare_nb++] = prepare;
	if (parent)
		g_atfork_parent[g_atfork_parent_nb++] = parent;
	if (child)
		g_atfork_child[g_atfork_child_nb++] = child;
	return 0;
}
