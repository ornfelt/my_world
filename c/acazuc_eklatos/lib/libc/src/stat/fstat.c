#include "../_syscall.h"

#include <sys/stat.h>
#include <fcntl.h>

int fstat(int fd, struct stat *statbuf)
{
	return syscall4(SYS_fstatat, fd, (uintptr_t)"", (uintptr_t)statbuf,
	                AT_EMPTY_PATH);
}
