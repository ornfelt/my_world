#include "../_syscall.h"

#include <unistd.h>

uid_t
geteuid(void)
{
	return syscall0(SYS_geteuid);
}
