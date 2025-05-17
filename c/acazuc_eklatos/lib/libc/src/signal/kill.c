#include "../_syscall.h"

#include <signal.h>

int
kill(pid_t pid, int sig)
{
	return syscall2(SYS_kill, pid, sig);
}
