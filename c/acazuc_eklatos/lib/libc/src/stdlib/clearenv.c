#include "_env.h"

#include <stdlib.h>

int clearenv(void)
{
	char **newenv = calloc(1, sizeof(*newenv));
	if (!newenv)
		return 1;
	char **oldenv = environ;
	environ = newenv;
	if (!oldenv)
		return 0;
	for (size_t i = 0; oldenv[i]; ++i)
		free(oldenv[i]);
	free(oldenv);
	return 0;
}