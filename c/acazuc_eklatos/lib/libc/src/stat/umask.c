#include "../_syscall.h"

#include <sys/stat.h>

mode_t umask(mode_t mask)
{
	return syscall1(SYS_umask, mask);
}
