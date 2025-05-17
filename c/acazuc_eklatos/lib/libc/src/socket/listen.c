#include "../_syscall.h"

#include <sys/socket.h>

int
listen(int sockfd, int backlog)
{
	return syscall2(SYS_listen, sockfd, backlog);
}
