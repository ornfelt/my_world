#include <termios.h>

speed_t
cfgetispeed(const struct termios *termios)
{
	return termios->c_ispeed;
}
