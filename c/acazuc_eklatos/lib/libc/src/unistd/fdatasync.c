#include "../_syscall.h"

#include <unistd.h>

int fdatasync(int fd)
{
	return syscall1(SYS_fdatasync, fd);
}
