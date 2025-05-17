#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

int
execle(const char *pathname, const char *arg, ...)
{
	va_list ap;
	char **argv;
	char **envp;
	char *a;
	int argc;
	int ret;

	argv = malloc(sizeof(*argv) * 2);
	if (!argv)
	{
		errno = ENOMEM;
		return -1;
	}
	argc = 1;
	argv[0] = (char*)arg;
	va_start(ap, arg);
	while ((a = va_arg(ap, char*)))
	{
		char **newarg;

		newarg = realloc(argv, sizeof(*newarg) * (argc + 2));
		if (!newarg)
		{
			free(argv);
			va_end(ap);
			errno = ENOMEM;
			return -1;
		}
		argv = newarg;
		argv[argc] = a;
		argc++;
	}
	argv[argc] = NULL;
	envp = va_arg(ap, char**);
	va_end(ap);
	ret = execve(pathname, argv, envp);
	free(argv);
	return ret;
}
