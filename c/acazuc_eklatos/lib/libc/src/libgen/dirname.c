#include <libgen.h>
#include <string.h>

char *dirname(char *path)
{
	if (!path || !*path)
		return ".";
	char *base = path;
	char *prev = NULL;
	while (1)
	{
		char *slash = strchr(path, '/');
		if (slash)
		{
			char *tmp = slash;
			while (*slash == '/')
				slash++;
			if (*slash)
			{
				prev = tmp;
				path = slash;
				continue;
			}
		}
		if (slash && !prev)
			return "/";
		if (!prev)
			return ".";
		if (prev == base)
			return "/";
		*prev = '\0';
		return base;
	}
}
