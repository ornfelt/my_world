#include "_dirent.h"

#include <dirent.h>
#include <unistd.h>

void
rewinddir(DIR *dirp)
{
	lseek(dirp->fd, SEEK_SET, 0);
}
