#include "../_syscall.h"

#include <unistd.h>

int setgroups(size_t size, const gid_t *list)
{
	return syscall2(SYS_setgroups, size, (uintptr_t)list);
}
