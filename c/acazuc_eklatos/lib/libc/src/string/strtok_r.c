#include <string.h>

char *
strtok_r(char *str, const char *delim, char **saveptr)
{
	char *src;
	if (str)
	{
		src = str;
	}
	else
	{
		if (!*saveptr)
			return NULL;
		src = *saveptr + 1;
		if (!*src)
		{
			*saveptr = NULL;
			return NULL;
		}
	}
	while (strchr(delim, *src))
		src++;
	char *tok = strpbrk(src, delim);
	if (!tok)
	{
		*saveptr = NULL;
		return *src ? src : NULL;
	}
	*tok = '\0';
	*saveptr = tok;
	return src;
}
