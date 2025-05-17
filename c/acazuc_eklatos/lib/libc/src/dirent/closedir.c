#include "_dirent.h"

#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

int
closedir(DIR *dirp)
{
	if (!dirp)
		return 0;
	free(dirp->dirent);
	close(dirp->fd);
	free(dirp);
	return 0;
}
