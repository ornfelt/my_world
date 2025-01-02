#include <sys/ioctl.h>

#include <termios.h>

int tcgetattr(int fd, struct termios *termios)
{
	return ioctl(fd, TCGETS, termios);
}
