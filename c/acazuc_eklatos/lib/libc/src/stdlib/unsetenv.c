#include "_env.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

int
unsetenv(const char *name)
{
	if (!environ)
		return 0;
	if (!name || !*name)
	{
		errno = EINVAL;
		return -1;
	}
	char *eq = strchr(name, '=');
	if (eq)
	{
		errno = EINVAL;
		return -1;
	}
	size_t key_len = strlen(name);
	for (size_t i = 0; environ[i]; ++i)
	{
		const char *val = envcmp(environ[i], name, key_len);
		if (!val)
			continue;
		for (; environ[i]; ++i)
			environ[i] = environ[i + 1];
		return 0;
	}
	return 0;
}
