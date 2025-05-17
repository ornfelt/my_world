#include <termios.h>
#include <errno.h>

int
tcdrain(int fd)
{
	(void)fd;
	/* XXX */
	errno = ENOSYS;
	return -1;
}
