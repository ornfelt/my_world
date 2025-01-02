#include "../_syscall.h"

#include <sys/resource.h>

int getpriority(int which, id_t who)
{
	int prio;
	int ret = syscall3(SYS_getpriority, which, who, (uintptr_t)&prio);
	if (ret == -1)
		return -1;
	return prio;
}
