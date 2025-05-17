#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int
link(const char *oldpath, const char *newpath)
{
	return syscall5(SYS_linkat,
	                AT_FDCWD,
	                (uintptr_t)oldpath,
	                AT_FDCWD,
	                (uintptr_t)newpath,
	                0);
}
