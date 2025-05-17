#include "../_syscall.h"

#include <sys/uio.h>

#include <unistd.h>

ssize_t
read(int fd, void *buffer, size_t count)
{
	struct iovec iov;

	iov.iov_base = buffer;
	iov.iov_len = count;
	return syscall3(SYS_readv, fd, (uintptr_t)&iov, 1);
}
