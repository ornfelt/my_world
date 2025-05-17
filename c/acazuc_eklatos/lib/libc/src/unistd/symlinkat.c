#include "../_syscall.h"

#include <unistd.h>

int
symlinkat(const char *target, int newdirfd, const char *linkpath)
{
	return syscall3(SYS_symlinkat,
	                (uintptr_t)target,
	                newdirfd,
	                (uintptr_t)linkpath);
}
