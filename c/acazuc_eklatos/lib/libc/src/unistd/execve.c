#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int
execve(const char *pathname, char * const argv[], char * const envp[])
{
	return syscall5(SYS_execveat,
	                AT_FDCWD,
	                (uintptr_t)pathname,
	                (uintptr_t)argv,
	                (uintptr_t)envp,
	                0);
}
