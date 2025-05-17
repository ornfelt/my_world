#include "../_syscall.h"

#include <sys/resource.h>

int
setpriority(int which, id_t who, int prio)
{
	return syscall3(SYS_setpriority, which, who, prio);
}
