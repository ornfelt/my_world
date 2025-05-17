#include <termios.h>

speed_t
cfgetospeed(const struct termios *termios)
{
	return termios->c_ospeed;
}
