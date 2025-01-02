#include "../_syscall.h"

#include <unistd.h>

pid_t setsid(void)
{
	return syscall0(SYS_setsid);
}
