#include "../_syscall.h"

#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	struct timespec ts;
	{
		ts.tv_sec = timeout / 1000;
		ts.tv_nsec = (timeout % 1000) * 1000000;
	}
	return syscall4(SYS_ppoll, (uintptr_t)fds, nfds,
	                timeout >= 0 ? (uintptr_t)&ts : 0, 0);
}
