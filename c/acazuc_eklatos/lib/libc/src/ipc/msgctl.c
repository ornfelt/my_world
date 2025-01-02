#include "../_syscall.h"

#include <sys/msg.h>

int msgctl(int msgid, int cmd, struct msgid_ds *buf)
{
	return syscall3(SYS_msgctl, msgid, cmd, (uintptr_t)buf);
}
