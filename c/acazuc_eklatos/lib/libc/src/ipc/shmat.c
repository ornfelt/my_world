#include "../_syscall.h"

#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int flags)
{
	return (void*)syscall3(SYS_shmat, shmid, (uintptr_t)shmaddr, flags);
}
