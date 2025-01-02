#include <stdlib.h>

int grantpt(int fd)
{
	(void)fd;
	/* XXX chmod-ish by ioctl */
	return 0;
}
