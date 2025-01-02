#ifndef IPC_H
#define IPC_H

#include <types.h>

struct timespec;
struct shmid_ds;
struct semid_ds;
struct msgid_ds;
struct sembuf;

int ipc_init(void);

int sys_shmget(key_t key, size_t size, int flags);
void *sys_shmat(int shmid, const void *shmaddr, int flags);
int sys_shmdt(const void *shmaddr);
int sys_shmctl(int shmid, int cmd, struct shmid_ds *buf);
int sys_semget(key_t key, int nsems, int flags);
int sys_semtimedop(int semid, struct sembuf *sops, size_t nsops,
                   const struct timespec *timeout);
int sys_semctl(int semid, int semnum, int cmd, intptr_t data);
int sys_msgget(key_t key, int flags);
int sys_msgsnd(int msgid, const void *msgp, size_t msgsz, int flags);
ssize_t sys_msgrcv(int msgid, void *msgp, size_t msgsz, long msgtyp,
                   int flags);
int sys_msgctl(int msgid, int cmd, struct msgid_ds *buf);

#endif
