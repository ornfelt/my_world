#include "../_syscall.h"

#include <sys/stat.h>
#include <fcntl.h>

int fchmod(int fd, mode_t mode)
{
	return syscall4(SYS_fchmodat, fd, (uintptr_t)"", mode, AT_EMPTY_PATH);
}
