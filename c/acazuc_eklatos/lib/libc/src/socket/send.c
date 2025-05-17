#include <sys/socket.h>

ssize_t
send(int fd, const void *data, size_t count, int flags)
{
	return sendto(fd, data, count, flags, NULL, 0);
}
