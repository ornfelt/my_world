#include "../_syscall.h"

#include <poll.h>

int
ppoll(struct pollfd *fds,
      nfds_t nfds,
      const struct timespec *timeout,
      const sigset_t *sigmask)
{
	return syscall4(SYS_ppoll,
	                (uintptr_t)fds,
	                nfds,
	                (uintptr_t)timeout,
	                (uintptr_t)sigmask);
}
