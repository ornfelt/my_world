#include "../_syscall.h"

#include <unistd.h>

int setreuid(uid_t ruid, uid_t euid)
{
	return syscall2(SYS_setreuid, ruid, euid);
}
