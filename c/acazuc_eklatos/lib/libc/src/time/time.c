#include "../_syscall.h"

#include <time.h>

time_t
time(time_t *tloc)
{
	time_t res;

	if (syscall1(SYS_time, (uintptr_t)&res) == -1)
		return -1;
	if (tloc)
		*tloc = res;
	return res;
}
