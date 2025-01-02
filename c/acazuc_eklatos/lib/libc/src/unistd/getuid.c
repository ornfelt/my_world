#include "../_syscall.h"

#include <unistd.h>

uid_t getuid(void)
{
	return syscall0(SYS_getuid);
}
