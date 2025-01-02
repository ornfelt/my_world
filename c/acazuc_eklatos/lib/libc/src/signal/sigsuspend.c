#include "../_syscall.h"

#include <signal.h>

int sigsuspend(const sigset_t *set)
{
	return syscall1(SYS_sigsuspend, (uintptr_t)set);
}
