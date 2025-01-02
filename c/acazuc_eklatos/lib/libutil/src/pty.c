#include <sys/ioctl.h>

#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <utmp.h>
#include <pty.h>

int openpty(int *mainfd, int *childfd, char *name,
            const struct termios *termios,
            const struct winsize *winsize)
{
	*mainfd = posix_openpt(O_RDWR);
	if (*mainfd == -1)
		return -1;
	if (grantpt(*mainfd) == -1)
	{
		close(*mainfd);
		*mainfd = -1;
		return -1;
	}
	if (unlockpt(*mainfd) == -1)
	{
		close(*mainfd);
		*mainfd = -1;
		return -1;
	}
	if (name) /* it's as bad as it looks like */
	{
		char *pty_file = ptsname(*mainfd);
		if (!pty_file)
		{
			close(*mainfd);
			*mainfd = -1;
			return -1;
		}
		strcpy(name, pty_file);
	}
	*childfd = ioctl(*mainfd, TIOCGPTPEER, O_RDWR);
	if (*childfd == -1)
	{
		close(*mainfd);
		*mainfd = -1;
		return -1;
	}
	if (termios)
		tcsetattr(*mainfd, TCSANOW, termios);
	if (winsize)
		ioctl(*mainfd, TIOCSWINSZ, winsize);
	return 0;
}

pid_t forkpty(int *mainfd, int *childfd, const struct termios *termios,
              const struct winsize *winsize)
{
	int cfd;
	int ret = openpty(mainfd, &cfd, NULL, termios, winsize);
	if (ret == -1)
		return -1;
	if (childfd)
		*childfd = cfd;
	pid_t pid = fork();
	if (pid == -1)
	{
		close(*mainfd);
		*mainfd = -1;
		close(*childfd);
		*childfd = -1;
		return -1;
	}
	if (!pid)
	{
		if (login_tty(cfd) == -1)
			return -1;
	}
	return pid;
}
