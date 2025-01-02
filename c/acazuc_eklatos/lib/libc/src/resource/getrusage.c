#include "../_syscall.h"

#include <sys/resource.h>

int getrusage(int who, struct rusage *rusage)
{
	return syscall2(SYS_getrusage, who, (uintptr_t)rusage);
}
