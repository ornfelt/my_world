#include "../_syscall.h"

#include <unistd.h>

int
getdents(int fd, struct sys_dirent *dirp, unsigned long count)
{
	return syscall3(SYS_getdents, fd, (uintptr_t)dirp, count);
}
