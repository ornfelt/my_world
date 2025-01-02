#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int access(const char *pathname, int mode)
{
	return syscall4(SYS_faccessat, AT_FDCWD, (uintptr_t)pathname, mode, 0);
}
