#include <termios.h>

int cfsetospeed(struct termios *termios, speed_t speed)
{
	termios->c_ospeed = speed;
	return 0;
}
