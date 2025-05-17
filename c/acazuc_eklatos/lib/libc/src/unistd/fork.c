#include "../_syscall.h"
#include "../_atfork.h"

#include <unistd.h>

pid_t
fork(void)
{
	/* XXX there is multithread stuff to be done here
	 * linker thread blocks must be cleanup up
	 * tls_addr must be reset too
	 */
	for (size_t i = 0; i < g_atfork_prepare_nb; ++i)
		g_atfork_prepare[i]();
	pid_t ret = syscall1(SYS_clone, 0);
	switch (ret)
	{
		case -1:
			return -1;
		case 0:
			for (size_t i = 0; i < g_atfork_child_nb; ++i)
				g_atfork_child[i]();
			break;
		default:
			for (size_t i = 0; i < g_atfork_parent_nb; ++i)
				g_atfork_parent[i]();
	}
	return ret;
}
