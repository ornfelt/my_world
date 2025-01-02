#include "_vexec.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int vexec(const char *pathname, int path, const char *arg, va_list ap)
{
	char **argv = malloc(sizeof(*argv) * 2);
	if (!argv)
	{
		errno = ENOMEM;
		return -1;
	}
	int argc = 1;
	argv[0] = (char*)arg;
	char *a;
	while ((a = va_arg(ap, char*)))
	{
		char **newarg = realloc(argv, sizeof(*newarg) * (argc + 2));
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
	int ret;
	if (path)
		ret = execvp(pathname, argv);
	else
		ret = execv(pathname, argv);
	free(argv);
	return ret;
}
