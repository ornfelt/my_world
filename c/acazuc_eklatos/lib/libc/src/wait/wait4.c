#include "../_syscall.h"

#include <sys/wait.h>

pid_t wait4(pid_t pid, int *wstatus, int options, struct rusage *rusage)
{
	return syscall4(SYS_wait4, pid, (uintptr_t)wstatus, options,
	                (uintptr_t)rusage);
}
