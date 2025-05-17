#include <sys/ioctl.h>

#include <termios.h>
#include <errno.h>

int
tcsetattr(int fd, int actions, const struct termios *termios)
{
	switch (actions)
	{
		case TCSANOW:
			return ioctl(fd, TCSETS, termios);
		case TCSADRAIN:
			return ioctl(fd, TCSETSW, termios);
		case TCSAFLUSH:
			return ioctl(fd, TCSETSF, termios);
		default:
			errno = EINVAL;
			return -1;
	}
}
