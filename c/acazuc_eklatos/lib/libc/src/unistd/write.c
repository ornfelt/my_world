#include "../_syscall.h"

#include <sys/uio.h>

#include <unistd.h>

ssize_t write(int fd, const void *buffer, size_t count)
{
	struct iovec iov;
	iov.iov_base = (void*)buffer;
	iov.iov_len = count;
	return syscall3(SYS_writev, fd, (uintptr_t)&iov, 1);
}
