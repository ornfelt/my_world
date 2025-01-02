#include "../_syscall.h"

#include <sys/socket.h>

ssize_t sendmsg(int fd, const struct msghdr *msg, int flags)
{
	return syscall3(SYS_sendmsg, fd, (uintptr_t)msg, flags);
}
