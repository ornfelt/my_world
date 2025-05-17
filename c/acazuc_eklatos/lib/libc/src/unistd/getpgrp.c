#include "../_syscall.h"

#include <unistd.h>

pid_t
getpgrp(void)
{
	return syscall0(SYS_getpgrp);
}
