#include "../_syscall.h"

#include <sys/socket.h>

int socketpair(int domain, int type, int protocol, int fds[2])
{
	return syscall4(SYS_socketpair, domain, type, protocol,
	                (uintptr_t)&fds[0]);
}
