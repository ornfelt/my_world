#include "../_syscall.h"

#include <unistd.h>
#include <fcntl.h>

int ftruncate(int fd, off_t length)
{
	return syscall4(SYS_ftruncateat, fd, (uintptr_t)"",
	                (uintptr_t)&length, AT_EMPTY_PATH);
}
