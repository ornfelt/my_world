#include "../_syscall.h"

#include <sys/resource.h>

int
setrlimit(int res, const struct rlimit *limit)
{
	return syscall2(SYS_setrlimit, res, (uintptr_t)limit);
}
