#include "../_syscall.h"

#include <stdio.h>

int
rename(const char *oldpath, const char *newpath)
{
	return syscall4(SYS_renameat,
	                AT_FDCWD,
	                (uintptr_t)oldpath,
	                AT_FDCWD,
	                (uintptr_t)newpath);
}
