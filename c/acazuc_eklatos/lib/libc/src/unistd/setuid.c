#include "../_syscall.h"

#include <unistd.h>

int
setuid(uid_t uid)
{
	return syscall1(SYS_setuid, uid);
}
