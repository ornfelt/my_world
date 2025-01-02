#include "../_syscall.h"

#include <unistd.h>

off_t lseek(int fd, off_t offset, int whence)
{
	int ret = syscall3(SYS_lseek, fd, (uintptr_t)&offset, whence);
	if (ret == -1)
		return ret;
	return offset;
}
