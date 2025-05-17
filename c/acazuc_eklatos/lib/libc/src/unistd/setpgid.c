#include "../_syscall.h"

#include <unistd.h>

int
setpgid(pid_t pid, pid_t pgid)
{
	return syscall2(SYS_setpgid, pid, pgid);
}
