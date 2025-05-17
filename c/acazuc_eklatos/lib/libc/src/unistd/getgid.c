#include "../_syscall.h"

#include <unistd.h>

gid_t
getgid(void)
{
	return syscall0(SYS_getgid);
}
