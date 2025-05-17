#include "../_syscall.h"

#include <sys/socket.h>

int
socket(int domain, int type, int protocol)
{
	return syscall3(SYS_socket, domain, type, protocol);
}
