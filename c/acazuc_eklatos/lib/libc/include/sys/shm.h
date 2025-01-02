#ifndef SYS_SHM_H
#define SYS_SHM_H

#include <sys/types.h>
#include <sys/ipc.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHM_RND    (1 << 12)
#define SHM_RDONLY (1 << 13)

struct shmid_ds
{
	struct ipc_perm shm_perm;
	size_t shm_segsz;
	time_t shm_atime;
	time_t shm_dtime;
	time_t shm_ctime;
	pid_t shm_cpid;
	pid_t shm_lpid;
	shmatt_t shm_nattch;
};

int shmget(key_t key, size_t size, int flags);
void *shmat(int shmid, const void *shmaddr, int flags);
int shmdt(const void *shmaddr);
int shmctl(int shmid, int cmd, struct shmid_ds *buf);

#ifdef __cplusplus
}
#endif

#endif
