#include "_env.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

int
setenv(const char *name, const char *value, int overwrite)
{
	char **newenv;
	char *eq;
	size_t key_len;
	size_t i;

	if (!name || !*name)
	{
		errno = EINVAL;
		return -1;
	}
	eq = strchr(name, '=');
	if (eq)
	{
		errno = EINVAL;
		return -1;
	}
	key_len = strlen(name);
	if (!environ)
	{
		char **env;

		env = malloc(sizeof(*env) * 2);
		if (!env)
			return -1;
		env[0] = makeenv(name, value);
		if (!env[0])
		{
			free(env);
			return -1;
		}
		env[1] = NULL;
		environ = env;
		return 0;
	}
	for (i = 0; environ[i]; ++i)
	{
		const char *val;
		char *newval;

		val = envcmp(environ[i], name, key_len);
		if (!val)
			continue;
		if (!overwrite)
			return 0;
		newval = makeenv(name, value);
		if (!newval)
			return -1;
		free(environ[i]);
		environ[i] = newval;
		return 0;
	}
	newenv = realloc(environ, sizeof(*environ) * (i + 2));
	if (!newenv)
		return -1;
	environ = newenv;
	newenv[i] = makeenv(name, value);
	if (!newenv[i])
		return -1;
	newenv[i + 1] = NULL;
	return 0;
}
