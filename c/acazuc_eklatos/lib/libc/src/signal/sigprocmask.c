#include "../_syscall.h"

#include <signal.h>

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	return syscall4(SYS_sigprocmask, how, (uintptr_t)set,
	                (uintptr_t)oldset, sizeof(set->set));
}
