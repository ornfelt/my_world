#include "../_syscall.h"

#include <unistd.h>

pid_t
getpid(void)
{
	return syscall0(SYS_getpid);
}
