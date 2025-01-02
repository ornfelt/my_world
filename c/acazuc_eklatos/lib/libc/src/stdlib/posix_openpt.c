#include <stdlib.h>
#include <fcntl.h>

int posix_openpt(int flags)
{
	return open("/dev/ptmx", flags);
}
