#include "../_syscall.h"

#include <unistd.h>

pid_t
getsid(pid_t pid)
{
	return syscall1(SYS_getsid, pid);
}
