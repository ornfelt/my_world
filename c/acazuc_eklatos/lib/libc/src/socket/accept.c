#include "../_syscall.h"

#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	return syscall3(SYS_accept, sockfd, (uintptr_t)addr, (uintptr_t)addrlen);
}
