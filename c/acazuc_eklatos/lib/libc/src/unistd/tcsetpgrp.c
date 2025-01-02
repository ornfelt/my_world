#include <sys/ioctl.h>

#include <unistd.h>

int tcsetpgrp(int fd, pid_t pgrp)
{
	return ioctl(fd, TIOCSPGRP, pgrp);
}
