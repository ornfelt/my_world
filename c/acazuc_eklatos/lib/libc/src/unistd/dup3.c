#include "../_syscall.h"

#include <unistd.h>

int
dup3(int oldfd, int newfd, int flags)
{
	return syscall3(SYS_dup3, oldfd, newfd, flags);
}
