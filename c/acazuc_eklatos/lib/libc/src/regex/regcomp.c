#include <string.h>
#include <regex.h>

int regcomp(regex_t *preg, const char *regex, int flags)
{
	preg->regex = strdup(regex);
	if (!preg->regex)
		return REG_ESPACE;
	preg->regex_len = strlen(regex);
	preg->flags = flags;
	preg->re_nsub = 0;
	return 0;
}
