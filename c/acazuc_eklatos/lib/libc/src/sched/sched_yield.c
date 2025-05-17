#include "../_syscall.h"

#include <sched.h>

int
sched_yield(void)
{
	return syscall0(SYS_sched_yield);
}
