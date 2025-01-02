#include "../_syscall.h"

#include <unistd.h>

int setegid(gid_t egid)
{
	return syscall2(SYS_setregid, (uintptr_t)-1, egid);
}
