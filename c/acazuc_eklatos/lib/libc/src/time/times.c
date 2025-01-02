#include "../_syscall.h"

#include <sys/times.h>

clock_t times(struct tms *tms)
{
	clock_t clk;
	int ret = syscall2(SYS_times, (uintptr_t)tms, (uintptr_t)&clk);
	if (ret)
		return ret;
	return clk;
}
