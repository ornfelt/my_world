#include "../_syscall.h"

#include <unistd.h>

int fexecve(int fd, char * const argv[], char * const envp[])
{
	return syscall5(SYS_execveat, fd, (uintptr_t)NULL, (uintptr_t)argv,
	                (uintptr_t)envp, 0);
}
