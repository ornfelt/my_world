#include <sys/ioctl.h>

#include <stdlib.h>

int unlockpt(int fd)
{
	int v = 0;
	return ioctl(fd, TIOCSPTLCK, &v);
}
