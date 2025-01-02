#include <termios.h>

int cfsetispeed(struct termios *termios, speed_t speed)
{
	termios->c_ispeed = speed;
	return 0;
}
