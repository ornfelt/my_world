#include <sys/ioctl.h>

#include <termios.h>

pid_t tcgetsid(int fd)
{
	return ioctl(fd, TIOCGSID);
}
