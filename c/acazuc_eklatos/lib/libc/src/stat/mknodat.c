#include "../_syscall.h"

#include <sys/stat.h>

int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev)
{
	return syscall4(SYS_mknodat, dirfd, (uintptr_t)pathname, mode, dev);
}
