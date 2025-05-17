#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int
daemon(int nochdir, int noclose)
{
	pid_t pid;

	pid = fork();
	if (pid == -1)
		return -1;
	if (pid)
		_exit(EXIT_SUCCESS);
	if (!nochdir)
	{
		int fd;

		fd = chdir("/");
		if (fd == -1)
			_exit(EXIT_FAILURE);
	}
	if (!noclose)
	{
		int devnull;

		devnull = open("/dev/null", O_RDONLY);
		if (devnull == -1)
			_exit(EXIT_FAILURE);
		if (dup2(devnull, 0) == -1
		 || dup2(devnull, 1) == -1
		 || dup2(devnull, 2) == -1)
			_exit(EXIT_FAILURE);
		close(devnull);
	}
	return 0;
}
