#include "_dirent.h"

#include <dirent.h>
#include <unistd.h>

long
telldir(DIR *dirp)
{
	return lseek(dirp->fd, SEEK_CUR, 0);
}
