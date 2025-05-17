#include "../_syscall.h"

#include <unistd.h>

void *
gettls(void)
{
	return (void*)syscall0(SYS_gettls);
}
