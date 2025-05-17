#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char *
getcwd(char *buf, size_t size)
{
	/* XXX use some syscall to resolve path */
	char *env;

	env = getenv("PWD");
	if (!env)
		return NULL;
	if (strlcpy(buf, env, size) >= size)
		return NULL;
	return buf;
}
