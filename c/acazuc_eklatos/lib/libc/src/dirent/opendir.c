#include "_dirent.h"

#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>

DIR *
opendir(const char *name)
{
	DIR *dir;

	dir = malloc(sizeof(*dir));
	if (!dir)
		return NULL;
	dir->fd = open(name, O_RDONLY | O_DIRECTORY);
	if (dir->fd == -1)
	{
		free(dir);
		return NULL;
	}
	dir->buf_pos = 0;
	dir->buf_end = 0;
	dir->dirent = NULL;
	return dir;
}
