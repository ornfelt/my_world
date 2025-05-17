#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int
symlink(const char *target, const char *linkpath)
{
	return syscall3(SYS_symlinkat,
	                (uintptr_t)target,
	                AT_FDCWD,
	                (uintptr_t)linkpath);
}
