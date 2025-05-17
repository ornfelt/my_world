#include "../_syscall.h"

#include <sys/select.h>

int
select(int nfds,
       fd_set *rfds,
       fd_set *wfds,
       fd_set *efds,
       struct timeval *timeout)
{
	struct timespec ts;

	if (timeout)
	{
		ts.tv_sec = timeout->tv_sec;
		ts.tv_nsec = timeout->tv_usec * 1000;
	}
	return syscall6(SYS_pselect,
	                nfds,
	                (uintptr_t)rfds,
	                (uintptr_t)wfds,
	                (uintptr_t)efds,
	                timeout ? (uintptr_t)&ts : 0,
	                0);
}
