#include "../_syscall.h"

#include <sys/sem.h>

int semtimedop(int semid, struct sembuf *sops, size_t nsops,
               const struct timespec *timeout)
{
	return syscall4(SYS_semtimedop, semid, (uintptr_t)sops, nsops,
	                (uintptr_t)timeout);
}
