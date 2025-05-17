#include "../_syscall.h"

#include <unistd.h>

int
pipe2(int pipefd[2], int flags)
{
	return syscall2(SYS_pipe2, (uintptr_t)&pipefd[0], flags);
}
