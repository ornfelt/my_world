#include <sys/socket.h>

ssize_t recv(int fd, void *data, size_t count, int flags)
{
	return recvfrom(fd, data, count, flags, NULL, NULL);
}
