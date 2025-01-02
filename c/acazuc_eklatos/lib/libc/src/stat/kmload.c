#include "../_syscall.h"

#include <sys/stat.h>

int kmload(int fd, const char *params, int flags)
{
	return syscall3(SYS_kmload, fd, (uintptr_t)params, flags);
}
