#include <libgen.h>
#include <string.h>

char *
dirname(char *path)
{
	char *base = path;
	char *prev = NULL;

	if (!path || !*path)
		return ".";
	while (1)
	{
		char *slash;

		slash = strchr(path, '/');
		if (slash)
		{
			char *tmp;

			tmp = slash;
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
