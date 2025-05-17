#include "../_syscall.h"

#include <unistd.h>

pid_t
getpgid(pid_t pid)
{
	return syscall1(SYS_getpgid, pid);
}
