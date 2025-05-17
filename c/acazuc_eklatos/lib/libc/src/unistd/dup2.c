#include "../_syscall.h"

#include <unistd.h>

int
dup2(int oldfd, int newfd)
{
	return syscall3(SYS_dup3, oldfd, newfd, 0);
}
