#include "../_syscall.h"

#include <sys/time.h>

int
getitimer(int which, struct itimerval *cur)
{
	return syscall2(SYS_getitimer, which, (uintptr_t)cur);
}
