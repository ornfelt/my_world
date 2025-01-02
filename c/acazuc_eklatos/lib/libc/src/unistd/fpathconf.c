#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

long fpathconf(int fd, int name)
{
	(void)fd;
	/* XXX use fd */
	switch (name)
	{
		case _PC_LINK_MAX:
			return 50;
		case _PC_MAX_CANON:
			return 4096;
		case _PC_MAX_INPUT:
			return 4096;
		case _PC_NAME_MAX:
			return 256;
		case _PC_PATH_MAX:
			return PATH_MAX;
		case _PC_PIPE_BUF:
			return PIPE_BUF;
		case _PC_CHOWN_RESTRICTED:
			return 0;
		case _PC_NO_TRUNC:
			return 1;
		case _PC_VDISABLE:
			return 1;
		default:
			errno = EINVAL;
			return -1;
	}
}
