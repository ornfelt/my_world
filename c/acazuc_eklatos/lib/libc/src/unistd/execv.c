#include <unistd.h>

extern char **environ;

int execv(const char *pathname, char * const argv[])
{
	return execve(pathname, argv, environ);
}
