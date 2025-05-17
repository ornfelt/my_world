#include <sys/ioctl.h>

#include <unistd.h>

pid_t
tcgetpgrp(int fd)
{
	pid_t pgrp;

	if (ioctl(fd, TIOCGPGRP, &pgrp) == -1)
		return -1;
	return pgrp;
}
