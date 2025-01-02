#include "../_syscall.h"

#include <unistd.h>

int getpagesize(void)
{
	return syscall0(SYS_getpagesize);
}
