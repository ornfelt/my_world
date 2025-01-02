#include "../_syscall.h"

#include <sys/socket.h>

int setsockopt(int fd, int level, int opt, const void *val,
               socklen_t len)
{
	return syscall5(SYS_setsockopt, fd, level, opt, (uintptr_t)val, len);
}
