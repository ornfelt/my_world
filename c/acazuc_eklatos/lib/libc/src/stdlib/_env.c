#include "_env.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char **environ;

const char *envcmp(const char *env, const char *key, unsigned long key_len)
{
	const char *eq = strchr(env, '=');
	if (!eq)
		return NULL;
	size_t len = eq - env;
	if (len == key_len && !strncmp(key, env, len))
		return eq + 1;
	return NULL;
}

char *makeenv(const char *name, const char *value)
{
	int len = snprintf(NULL, 0, "%s=%s", name, value ? value : "");
	if (len <= 0)
		return NULL;
	len++;
	char *v = malloc(len);
	if (!v)
		return NULL;
	snprintf(v, len, "%s=%s", name, value ? value : "");
	return v;
}
