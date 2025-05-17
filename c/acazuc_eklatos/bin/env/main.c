#include <sys/wait.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

extern char **environ;

int
main(int argc, char **argv)
{
	if (argc == 1)
	{
		for (size_t i = 0; environ[i]; ++i)
			puts(environ[i]);
		return EXIT_SUCCESS;
	}
	for (int i = 1; i < argc; ++i)
	{
		if (!strchr(argv[i], '='))
		{
			int wstatus;
			int pid;

			pid = fork();
			if (pid == -1)
				return EXIT_FAILURE;
			if (pid == 0)
			{
				execvp(argv[i], &argv[i]);
				return EXIT_FAILURE;
			}
			while (waitpid(pid, &wstatus, 0) == -1)
			{
				if (errno != EAGAIN)
					return EXIT_FAILURE;
			}
			if (WIFEXITED(wstatus))
				return WEXITSTATUS(wstatus);
			return EXIT_FAILURE;
		}
		putenv(argv[i]);
	}
	return EXIT_SUCCESS;
}
