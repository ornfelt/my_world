#include "_env.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

int putenv(char *string)
{
	if (!string || !*string)
	{
		errno = EINVAL;
		return -1;
	}
	char *eq = strchr(string, '=');
	if (!eq)
	{
		errno = EINVAL;
		return -1;
	}
	size_t key_len = eq - string;
	if (!environ)
	{
		char **env = malloc(sizeof(*env) * 2);
		if (!env)
			return -1;
		env[0] = strdup(string);
		if (!env[0])
		{
			free(env);
			return -1;
		}
		env[1] = NULL;
		environ = env;
		return 0;
	}
	size_t i;
	for (i = 0; environ[i]; ++i)
	{
		const char *val = envcmp(environ[i], string, key_len);
		if (!val)
			continue;
		char *newval = strdup(string);
		if (!newval)
			return -1;
		free(environ[i]);
		environ[i] = newval;
		return 0;
	}
	char **newenv = realloc(environ, sizeof(*environ) * (i + 2));
	if (!newenv)
		return -1;
	environ = newenv;
	newenv[i] = strdup(string);
	if (!newenv[i])
		return -1;
	newenv[i + 1] = NULL;
	return 0;
}
