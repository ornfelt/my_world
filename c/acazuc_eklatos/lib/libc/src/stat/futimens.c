#include "../_syscall.h"

#include <sys/stat.h>

#include <fcntl.h>

int futimens(int fd, const struct timespec times[2])
{
	return syscall4(SYS_utimensat, fd, (uintptr_t)"",
	                (uintptr_t)&times[0], AT_EMPTY_PATH);
}
