#include "../_syscall.h"

#include <sys/uio.h>

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
	return syscall3(SYS_writev, fd, (uintptr_t)iov, iovcnt);
}
