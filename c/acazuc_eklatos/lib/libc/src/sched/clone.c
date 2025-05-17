#include "../_syscall.h"

#include <sched.h>

pid_t
clone(int flags)
{
	return syscall1(SYS_clone, flags);
}
