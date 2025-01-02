#include "../_syscall.h"

#include <unistd.h>

int setregid(gid_t rgid, gid_t egid)
{
	return syscall2(SYS_setregid, rgid, egid);
}
