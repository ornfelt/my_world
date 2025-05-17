#include <sys/wait.h>

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

int
system(const char *command)
{
	int ret;
	pid_t pid;

	if (!command)
		return 1;
	sighandler_t intret = signal(SIGINT, SIG_IGN);
	if (intret == SIG_ERR)
		return -1;
	sighandler_t quitret = signal(SIGQUIT, SIG_IGN);
	if (quitret == SIG_ERR)
	{
		signal(SIGINT, intret);
		return -1;
	}
	pid = vfork();
	if (pid == -1)
	{
		signal(SIGINT, intret);
		signal(SIGQUIT, quitret);
		return -1;
	}
	if (pid == 0)
	{
		char * const argv[] =
		{
			"sh",
			"-c",
			(char*)command,
			NULL,
		};
		execv("/bin/sh", argv);
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		int wstatus;

		ret = waitpid(pid, &wstatus, 0);
		if (ret != -1)
		{
			ret = wstatus;
			break;
		}
		if (errno == EINTR)
			continue;
		ret = errno;
		break;
	}
	signal(SIGINT, intret);
	signal(SIGQUIT, quitret);
	return ret;
}
