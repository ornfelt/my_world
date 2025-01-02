#include "../_syscall.h"

#include <signal.h>

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	return syscall3(SYS_sigaction, signum, (uintptr_t)act, (uintptr_t)oldact);
}
