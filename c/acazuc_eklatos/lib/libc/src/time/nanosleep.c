#include "../_syscall.h"

#include <time.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
	return syscall2(SYS_nanosleep, (uintptr_t)req, (uintptr_t)rem);
}
