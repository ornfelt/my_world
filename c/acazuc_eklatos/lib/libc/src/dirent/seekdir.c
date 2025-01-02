#include "_dirent.h"

#include <dirent.h>
#include <unistd.h>

void seekdir(DIR *dirp, long loc)
{
	lseek(dirp->fd, SEEK_SET, loc);
}
