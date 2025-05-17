#include "../_syscall.h"

#include <sys/statvfs.h>

#include <fcntl.h>

int
fstatvfs(int fd, struct statvfs *buf)
{
	return syscall4(SYS_fstatvfsat,
	                fd,
	                (uintptr_t)"",
	                (uintptr_t)buf,
	                AT_EMPTY_PATH);
}
