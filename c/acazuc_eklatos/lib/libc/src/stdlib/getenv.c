#include "_env.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *
getenv(const char *name)
{
	size_t key_len;
	char *eq;

	if (!environ)
		return NULL;
	if (!name || !*name)
	{
		errno = EINVAL;
		return NULL;
	}
	eq = strchr(name, '=');
	if (eq)
	{
		errno = EINVAL;
		return NULL;
	}
	key_len = strlen(name);
	for (size_t i = 0; environ[i]; ++i)
	{
		const char *val;

		val = envcmp(environ[i], name, key_len);
		if (!val)
			continue;
		return (char*)val;
	}
	return NULL;
}
