#include "../_syscall.h"

#include <sys/shm.h>

int
shmget(key_t key, size_t size, int flags)
{
	return syscall3(SYS_shmget, key, size, flags);
}
