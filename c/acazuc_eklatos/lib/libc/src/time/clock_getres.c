#include "../_syscall.h"

#include <time.h>

int clock_getres(clockid_t clk_id, struct timespec *res)
{
	return syscall2(SYS_clock_getres, clk_id, (uintptr_t)res);
}
