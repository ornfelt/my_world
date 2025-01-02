#include "../_syscall.h"

#include <sys/socket.h>

int getsockopt(int fd, int level, int opt, void *val, socklen_t *len)
{
	return syscall5(SYS_getsockopt, fd, level, opt, (uintptr_t)val,
	                (uintptr_t)len);
}
