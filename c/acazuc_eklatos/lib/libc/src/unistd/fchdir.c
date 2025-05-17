#include "../_syscall.h"

#include <unistd.h>

int
fchdir(int fd)
{
	return syscall1(SYS_fchdir, fd);
}
