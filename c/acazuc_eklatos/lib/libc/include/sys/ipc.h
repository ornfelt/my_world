#ifndef SYS_IPC_H
#define SYS_IPC_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_STAT 1
#define IPC_SET  2
#define IPC_RMID 3

#define IPC_PRIVATE 0

#define IPC_CREAT  (1 << 9)
#define IPC_EXCL   (1 << 10)
#define IPC_NOWAIT (1 << 11)

struct ipc_perm
{
	key_t key;
	uid_t uid;
	gid_t gid;
	uid_t cuid;
	gid_t cgid;
	unsigned short mode;
	unsigned short seq;
};

key_t ftok(const char *pathname, int proj_id);

#ifdef __cplusplus
}
#endif

#endif
