#include <libgen.h>
#include <string.h>

char *
basename(char *path)
{
	char *slash;
	char *base_slash;

	if (!path || !*path)
		return ".";
	while (1)
	{
		slash = strchr(path, '/');
		if (!slash)
			return *path ? path : "/";
		base_slash = slash;
		while (*slash == '/')
			slash++;
		if (!*slash)
		{
			*base_slash = '\0';
			return *path ? path : "/";
		}
		path = slash;
	}
}
