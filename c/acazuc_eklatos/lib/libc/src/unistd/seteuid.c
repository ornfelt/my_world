#include "../_syscall.h"

#include <unistd.h>

int seteuid(uid_t euid)
{
	return syscall2(SYS_setreuid, (uintptr_t)-1, euid);
}
