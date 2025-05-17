#include "../_syscall.h"

#include <sys/socket.h>

int
getsockname(int fd, struct sockaddr *addr, socklen_t *len)
{
	return syscall3(SYS_getsockname, fd, (uintptr_t)addr, (uintptr_t)len);
}
