#include "../_syscall.h"

#include <sys/wait.h>

pid_t
wait3(int *wstatus, int options, struct rusage *rusage)
{
	return syscall4(SYS_wait4,
	                -1,
	                (uintptr_t)wstatus,
	                options,
	                (uintptr_t)rusage);
}
