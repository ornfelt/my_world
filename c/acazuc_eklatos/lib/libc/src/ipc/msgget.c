#include "../_syscall.h"

#include <sys/msg.h>

int
msgget(key_t key, int flags)
{
	return syscall2(SYS_msgget, key, flags);
}
