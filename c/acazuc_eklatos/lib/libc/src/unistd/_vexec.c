#include "_vexec.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int
vexec(const char *pathname, int path, const char *arg, va_list ap)
{
	char **argv;
	int argc;
	char *a;
	int ret;

	argv = malloc(sizeof(*argv) * 2);
	if (!argv)
	{
		errno = ENOMEM;
		return -1;
	}
	argc = 1;
	argv[0] = (char*)arg;
	while ((a = va_arg(ap, char*)))
	{
		char **newarg;

		newarg = realloc(argv, sizeof(*newarg) * (argc + 2));
		if (!newarg)
		{
			free(argv);
			errno = ENOMEM;
			return -1;
		}
		argv = newarg;
		argv[argc] = a;
		argc++;
	}
	argv[argc] = NULL;
	if (path)
		ret = execvp(pathname, argv);
	else
		ret = execv(pathname, argv);
	free(argv);
	return ret;
}
