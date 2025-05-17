#include "../_syscall.h"

#include <sys/msg.h>

ssize_t
msgrcv(int msgid, void *msgp, size_t msgsz, long msgtyp, int flags)
{
	return syscall5(SYS_msgrcv, msgid, (uintptr_t)msgp, msgsz, msgtyp, flags);
}
