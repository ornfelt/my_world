#include "../_syscall.h"

#include <unistd.h>

void
exit_group(int status)
{
	syscall1(SYS_exit_group, status);
	while (1);
}
