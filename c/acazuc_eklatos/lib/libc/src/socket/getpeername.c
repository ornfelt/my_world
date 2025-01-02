#include "../_syscall.h"

#include <sys/socket.h>

int getpeername(int fd, struct sockaddr *addr, socklen_t *len)
{
	return syscall3(SYS_getpeername, fd, (uintptr_t)addr, (uintptr_t)len);
}
