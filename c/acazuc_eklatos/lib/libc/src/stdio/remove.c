#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int
remove(const char *pathname)
{
	if (!unlink(pathname))
		return 0;
	if (errno != EISDIR)
		return -1;
	return rmdir(pathname);
}
