#include "../_syscall.h"

#include <unistd.h>

gid_t getegid(void)
{
	return syscall0(SYS_getegid);
}
