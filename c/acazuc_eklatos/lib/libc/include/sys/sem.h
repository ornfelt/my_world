#ifndef SYS_SEM_H
#define SYS_SEM_H

#include <sys/types.h>
#include <sys/ipc.h>

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEM_UNDO (1 << 14)

#define GETVAL  10
#define SETVAL  11
#define GETPID  12
#define GETNCNT 13
#define GETZCNT 14
#define GETALL  15
#define SETALL  16

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

struct semid_ds
{
	struct ipc_perm sem_perm;
	time_t sem_otime;
	time_t sem_ctime;
	unsigned long sem_nsems;
};

struct sembuf
{
	unsigned short sem_num;
	short sem_op;
	short sem_flg;
};

int semget(key_t key, int nsems, int flags);
int semop(int semid, struct sembuf *sops, size_t nsops);
int semtimedop(int semid, struct sembuf *sops, size_t nsops,
               const struct timespec *timeout);
int semctl(int semid, int semnum, int cmd, ...);

#ifdef __cplusplus
}
#endif

#endif
