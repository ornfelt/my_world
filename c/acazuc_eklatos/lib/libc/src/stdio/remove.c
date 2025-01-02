#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int remove(const char *pathname)
{
	int res = unlink(pathname);
	if (!res)
		return 0;
	if (errno != EISDIR)
		return -1;
	return rmdir(pathname);
}
