#include "../_syscall.h"

#include <unistd.h>

int dup(int oldfd)
{
	return syscall1(SYS_dup, oldfd);
}
