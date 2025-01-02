#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

int execle(const char *pathname, const char *arg, ...)
{
	char **argv = malloc(sizeof(*argv) * 2);
	if (!argv)
	{
		errno = ENOMEM;
		return -1;
	}
	int argc = 1;
	argv[0] = (char*)arg;
	va_list ap;
	va_start(ap, arg);
	char *a;
	while ((a = va_arg(ap, char*)))
	{
		char **newarg = realloc(argv, sizeof(*newarg) * (argc + 2));
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
	char **envp = va_arg(ap, char**);
	va_end(ap);
	int ret = execve(pathname, argv, envp);
	free(argv);
	return ret;
}
