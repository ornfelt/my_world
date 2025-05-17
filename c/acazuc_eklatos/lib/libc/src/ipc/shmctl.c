#include "../_syscall.h"

#include <sys/shm.h>

int
shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
	return syscall3(SYS_shmctl, shmid, cmd, (uintptr_t)buf);
}
