#include "../_syscall.h"

#include <signal.h>

int
sigaltstack(const stack_t *ss, stack_t *old_ss)
{
	return syscall2(SYS_sigaltstack, (uintptr_t)ss, (uintptr_t)old_ss);
}
