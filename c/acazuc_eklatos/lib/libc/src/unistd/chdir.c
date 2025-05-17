#include "../_syscall.h"

#include <unistd.h>

int
chdir(const char *pathname)
{
	return syscall1(SYS_chdir, (uintptr_t)pathname);
}
