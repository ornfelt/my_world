#include "../_syscall.h"

#include <time.h>

int
clock_settime(clockid_t clk_id, const struct timespec *tp)
{
	return syscall2(SYS_clock_settime, clk_id, (uintptr_t)tp);
}
