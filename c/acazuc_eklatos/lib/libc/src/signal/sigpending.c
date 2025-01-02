#include "../_syscall.h"

#include <signal.h>

int sigpending(sigset_t *set)
{
	return syscall1(SYS_sigpending, (uintptr_t)set);
}
