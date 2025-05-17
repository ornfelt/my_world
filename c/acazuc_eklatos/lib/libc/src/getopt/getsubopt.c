#include <string.h>
#include <stdlib.h>

int
getsubopt(char **optionp, char * const *tokens, char **valuep)
{
	size_t optlen;
	char *nextopt;
	char *eq;

	if (!**optionp)
		return -1;
	nextopt = strchr(*optionp, ',');
	if (nextopt)
		eq = memchr(*optionp, '=', nextopt - *optionp);
	else
		eq = strchr(*optionp, '=');
	if (eq)
		optlen = eq - *optionp;
	else if (nextopt)
		optlen = nextopt - *optionp;
	else
		optlen = strlen(*optionp);
	for (size_t i = 0; tokens[i]; ++i)
	{
		if (strlen(tokens[i]) != optlen
		 || strncmp(tokens[i], *optionp, optlen))
			continue;
		*valuep = eq ? eq + 1 : NULL;
		if (nextopt)
		{
			*nextopt = '\0';
			*optionp = nextopt + 1;
		}
		else
		{
			*optionp += strlen(*optionp);
		}
		return i;
	}
	return -1;
}
