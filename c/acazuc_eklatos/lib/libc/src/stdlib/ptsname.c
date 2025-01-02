#include <sys/ioctl.h>
#include <sys/param.h>

#include <stdlib.h>
#include <stdio.h>

char *ptsname(int fd)
{
	static char name[MAXPATHLEN];
	int id;
	if (ioctl(fd, TIOCGPTN, &id) == -1)
		return NULL;
	snprintf(name, sizeof(name), "/dev/pts/%d", id);
	return name;
}
