#include "../_syscall.h"

#include <sys/wait.h>

pid_t wait(int *wstatus)
{
	return syscall4(SYS_wait4, -1, (uintptr_t)wstatus, 0, (uintptr_t)NULL);
}
