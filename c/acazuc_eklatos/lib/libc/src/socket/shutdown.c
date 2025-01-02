#include "../_syscall.h"

#include <sys/socket.h>

int shutdown(int fd, int how)
{
	return syscall2(SYS_shutdown, fd, how);
}
