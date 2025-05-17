#include <sys/resource.h>

#include <unistd.h>
#include <errno.h>

int
nice(int inc)
{
	int prio;

	errno = 0;
	prio = getpriority(PRIO_PROCESS, 0);
	if (errno)
		return -1;
	prio += inc;
	if (setpriority(PRIO_PROCESS, 0, prio) == -1)
		return -1;
	return prio;
}
