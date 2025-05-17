#include "../_syscall.h"

#include <sys/stat.h>
#include <fcntl.h>

int
mknod(const char *pathname, mode_t mode, dev_t dev)
{
	return syscall4(SYS_mknodat, AT_FDCWD, (uintptr_t)pathname, mode, dev);
}
