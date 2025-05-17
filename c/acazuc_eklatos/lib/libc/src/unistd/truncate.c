#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int
truncate(const char *pathname, off_t length)
{
	return syscall4(SYS_ftruncateat,
	                AT_FDCWD,
	                (uintptr_t)pathname,
	                (uintptr_t)&length,
	                0);
}
