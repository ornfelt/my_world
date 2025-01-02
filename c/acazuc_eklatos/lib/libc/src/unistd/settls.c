#include "../_syscall.h"

#include <unistd.h>

int settls(void *addr)
{
	return syscall1(SYS_settls, (uintptr_t)addr);
}
