#include "../_syscall.h"

#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return syscall3(SYS_connect, sockfd, (uintptr_t)addr, addrlen);
}
