#include "../_syscall.h"

#include <sys/socket.h>

ssize_t
sendto(int fd,
       const void *data,
       size_t count,
       int flags,
       const struct sockaddr *addr,
       socklen_t addrlen)
{
	struct iovec iov;
	struct msghdr msg;

	iov.iov_base = (void*)data;
	iov.iov_len = count;
	msg.msg_name = (void*)addr;
	msg.msg_namelen = addrlen;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	return syscall3(SYS_sendmsg, fd, (uintptr_t)&msg, flags);
}
