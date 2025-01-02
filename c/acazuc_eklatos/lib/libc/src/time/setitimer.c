#include "../_syscall.h"

#include <sys/time.h>

int setitimer(int which, const struct itimerval *val, struct itimerval *old)
{
	return syscall3(SYS_setitimer, which, (uintptr_t)val, (uintptr_t)old);
}
