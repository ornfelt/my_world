#include "../_syscall.h"

#include <sys/stat.h>

int
utimensat(int dirfd,
          const char *pathname,
          const struct timespec times[2],
          int flags)
{
	return syscall4(SYS_utimensat,
	                dirfd,
	                (uintptr_t)pathname,
	                (uintptr_t)&times[0],
	                flags);
}
