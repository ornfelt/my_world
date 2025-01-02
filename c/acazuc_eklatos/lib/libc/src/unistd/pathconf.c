#include <unistd.h>

long pathconf(const char *path, int name)
{
	(void)path;
	/* XXX use path */
	return fpathconf(-1, name);
}
