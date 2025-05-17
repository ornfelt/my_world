#include <unistd.h>
#include <utmp.h>

int
login_tty(int fd)
{
	if (setsid() == -1
	 || dup2(fd, 0) == -1
	 || dup2(fd, 1) == -1
	 || dup2(fd, 2) == -1
	 || close(fd) == -1)
		return -1;
	return 0;
}
