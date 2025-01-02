#include "../_syscall.h"

#include <unistd.h>

int pipe(int pipefd[2])
{
	return syscall2(SYS_pipe2, (uintptr_t)&pipefd[0], 0);
}
