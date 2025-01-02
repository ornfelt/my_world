#include "_syscall.h"

#include <sys/futex.h>

int futex(int *uaddr, int op, int val, const struct timespec *timeout)
{
	return syscall4(SYS_futex, (uintptr_t)uaddr, op, val,
	                (uintptr_t)timeout);
}
