#include "../_syscall.h"

#include <sys/msg.h>

int
msgsnd(int msgid, const void *msgp, size_t msgsz, int flags)
{
	return syscall4(SYS_msgsnd, msgid, (uintptr_t)msgp, msgsz, flags);
}
