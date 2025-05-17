#include "../_syscall.h"

#include <sys/uio.h>

ssize_t
readv(int fd, const struct iovec *iov, int iovcnt)
{
	return syscall3(SYS_readv, fd, (uintptr_t)iov, iovcnt);
}
