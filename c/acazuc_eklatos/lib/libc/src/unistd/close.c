#include "../_syscall.h"

#include <unistd.h>

int
close(int fd)
{
	return syscall1(SYS_close, fd);
}
