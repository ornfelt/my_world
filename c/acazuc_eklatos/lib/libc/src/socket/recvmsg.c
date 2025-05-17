#include "../_syscall.h"

#include <sys/socket.h>

ssize_t
recvmsg(int fd, struct msghdr *msg, int flags)
{
	return syscall3(SYS_recvmsg, fd, (uintptr_t)msg, flags);
}
