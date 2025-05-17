#include <time.h>

int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
		return -1;
	if (tv)
	{
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / 1000;
	}
	if (tz)
	{
		tz->tz_minuteswest = 0; /* XXX */
		tz->tz_dsttime = 0; /* XXX */
	}
	return 0;
}
