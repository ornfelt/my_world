#include "../_syscall.h"

#include <unistd.h>

void
_exit(int status)
{
	(void)syscall1(SYS_exit, status);
	while (1);
}
