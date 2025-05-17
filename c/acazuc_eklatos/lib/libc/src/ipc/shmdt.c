#include "../_syscall.h"

#include <sys/shm.h>

int
shmdt(const void *shmaddr)
{
	return syscall1(SYS_shmdt, (uintptr_t)shmaddr);
}
