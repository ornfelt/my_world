#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int
rmdir(const char *pathname)
{
	return syscall3(SYS_unlinkat,
	                AT_FDCWD,
	                (uintptr_t)pathname,
	                AT_REMOVEDIR);
}
