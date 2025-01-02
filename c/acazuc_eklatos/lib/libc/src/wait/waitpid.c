#include "../_syscall.h"

#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *wstatus, int options)
{
	return syscall4(SYS_wait4, pid, (uintptr_t)wstatus, options,
	                (uintptr_t)NULL);
}
