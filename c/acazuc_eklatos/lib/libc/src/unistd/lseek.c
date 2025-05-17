#include "../_syscall.h"

#include <unistd.h>

off_t
lseek(int fd, off_t offset, int whence)
{
	if (syscall3(SYS_lseek, fd, (uintptr_t)&offset, whence) == -1)
		return -1;
	return offset;
}
