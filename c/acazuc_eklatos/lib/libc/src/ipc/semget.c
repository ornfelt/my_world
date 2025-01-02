#include "../_syscall.h"

#include <sys/sem.h>

int semget(key_t key, int nsems, int flags)
{
	return syscall3(SYS_semget, key, nsems, flags);
}
