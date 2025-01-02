#include "../_syscall.h"

#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return syscall3(SYS_bind, sockfd, (uintptr_t)addr, addrlen);
}
