#include "../_syscall.h"

#include <unistd.h>

int chroot(const char *path)
{
	return syscall1(SYS_chroot, (uintptr_t)path);
}
