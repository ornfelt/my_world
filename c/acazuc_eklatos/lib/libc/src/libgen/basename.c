#include <libgen.h>
#include <string.h>

char *basename(char *path)
{
	if (!path || !*path)
		return ".";
	while (1)
	{
		char *slash = strchr(path, '/');
		if (!slash)
			return *path ? path : "/";
		char *base_slash = slash;
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
