#include "../_syscall.h"

#include <unistd.h>

int pause(void)
{
	return syscall1(SYS_sigsuspend, 0);
}
