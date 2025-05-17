#include "_dirent.h"

#include <dirent.h>
#include <stdlib.h>

DIR *
fdopendir(int fd)
{
	DIR *dir;

	dir = malloc(sizeof(*dir));
	if (!dir)
		return NULL;
	dir->fd = fd;
	dir->buf_pos = 0;
	dir->buf_end = 0;
	dir->dirent = NULL;
	return dir;
}
