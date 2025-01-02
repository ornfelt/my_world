#include "../_syscall.h"

#include <unistd.h>

pid_t gettid(void)
{
	return syscall0(SYS_gettid);
}
