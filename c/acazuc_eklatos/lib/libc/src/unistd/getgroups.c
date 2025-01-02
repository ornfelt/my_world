#include "../_syscall.h"

#include <unistd.h>

int getgroups(int size, gid_t list[])
{
	return syscall2(SYS_getgroups, size, (uintptr_t)&list[0]);
}
