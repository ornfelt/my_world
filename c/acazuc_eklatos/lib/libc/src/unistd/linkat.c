#include "../_syscall.h"

#include <unistd.h>

int
linkat(int olddirfd,
       const char *oldpath,
       int newdirfd,
       const char *newpath,
       int flags)
{
	return syscall5(SYS_linkat,
	                olddirfd,
	                (uintptr_t)oldpath,
	                newdirfd,
	                (uintptr_t)newpath,
	                flags);
}
