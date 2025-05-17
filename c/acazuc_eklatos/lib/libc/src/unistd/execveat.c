#include "../_syscall.h"

#include <unistd.h>

int
execveat(int dirfd,
         const char *pathname,
         char * const argv[],
         char * const envp[],
         int flags)
{
	return syscall5(SYS_execveat,
	                dirfd,
	                (uintptr_t)pathname,
	                (uintptr_t)argv,
	                (uintptr_t)envp,
	                flags);
}
