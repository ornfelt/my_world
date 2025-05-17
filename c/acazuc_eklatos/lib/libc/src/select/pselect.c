#include "../_syscall.h"

#include <sys/select.h>

int
pselect(int nfds,
        fd_set *rfds,
        fd_set *wfds,
        fd_set *efds,
        const struct timespec *timeout,
        const sigset_t *sigmask)
{
	return syscall6(SYS_pselect,
	                nfds,
	                (uintptr_t)rfds,
	                (uintptr_t)wfds,
	                (uintptr_t)efds,
	                (uintptr_t)timeout,
	                (uintptr_t)sigmask);
}
