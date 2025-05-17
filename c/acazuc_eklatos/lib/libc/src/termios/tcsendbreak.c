#include <sys/ioctl.h>

#include <termios.h>

int
tcsendbreak(int fd, int duration)
{
	return ioctl(fd, TCSBRK, duration);
}
