#ifndef SYS_MSG_H
#define SYS_MSG_H

#include <sys/types.h>
#include <sys/ipc.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_NOERROR (1 << 15)
#define MSG_EXCEPT  (1 << 16)

struct msgid_ds
{
	struct ipc_perm msg_perm;
	time_t msg_stime;
	time_t msg_rtime;
	time_t msg_ctime;
	unsigned long msg_cbytes;
	msgqnum_t msg_qnum;
	msglen_t msg_qbytes;
	pid_t msg_lspid;
	pid_t msg_lrpid;
};

struct msgbuf
{
	long mtype;
	char mtext[1];
};

int msgget(key_t key, int flags);
int msgsnd(int msgid, const void *msgp, size_t msgsz, int flags);
ssize_t msgrcv(int msgid, void *msgp, size_t msgsz, long msgtyp, int flags);
int msgctl(int msgid, int cmd, struct msgid_ds *buf);

#ifdef __cplusplus
}
#endif

#endif
