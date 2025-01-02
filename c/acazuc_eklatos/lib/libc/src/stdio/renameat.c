#include "../_syscall.h"

#include <stdio.h>

int renameat(int olddirfd, const char *oldpath, int newdirfd,
             const char *newpath)
{
	return syscall4(SYS_renameat, olddirfd, (uintptr_t)oldpath,
	                newdirfd, (uintptr_t)newpath);
}
