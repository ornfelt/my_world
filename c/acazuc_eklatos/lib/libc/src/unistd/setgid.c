#include "../_syscall.h"

#include <unistd.h>

int setgid(gid_t gid)
{
	return syscall1(SYS_setgid, gid);
}
