#include <sys/ioctl.h>

#include <termios.h>
#include <unistd.h>
#include <errno.h>

int
isatty(int fd)
{
	struct termios termios;

	if (ioctl(fd, TCGETS, &termios) == -1)
	{
		if (errno != EBADF)
			errno = ENOTTY;
		return 0;
	}
	return 1;
}
