#include "../_syscall.h"

#include <sys/resource.h>

int getrlimit(int res, struct rlimit *limit)
{
	return syscall2(SYS_getrlimit, res, (uintptr_t)limit);
}
