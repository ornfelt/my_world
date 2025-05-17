#include "../_syscall.h"

#include <sys/sem.h>

int
semop(int semid, struct sembuf *sops, size_t nsops)
{
	return syscall4(SYS_semtimedop,
	                semid,
	                (uintptr_t)sops,
	                nsops,
	                (uintptr_t)NULL);
}
