#include "../_syscall.h"

#include <sys/socket.h>

ssize_t
recvfrom(int fd,
         void *data,
         size_t count,
         int flags,
         struct sockaddr *addr,
         socklen_t *addrlen)
{
	struct iovec iov;
	struct msghdr msg;
	ssize_t ret;

	iov.iov_base = data;
	iov.iov_len = count;
	msg.msg_name = addr;
	msg.msg_namelen = addrlen ? *addrlen : 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	ret = syscall3(SYS_recvmsg, fd, (uintptr_t)&msg, flags);
	if (addrlen)
		*addrlen = msg.msg_namelen;
	return ret;
}
