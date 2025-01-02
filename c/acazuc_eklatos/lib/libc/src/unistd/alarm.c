#include <unistd.h>
#include <time.h>

unsigned alarm(unsigned seconds)
{
	struct itimerval val;
	struct itimerval old;
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts))
		return 0;
	val.it_value.tv_sec = ts.tv_sec + seconds;
	val.it_value.tv_usec = ts.tv_nsec / 1000;
	val.it_interval.tv_sec = 0;
	val.it_interval.tv_usec = 0;
	if (setitimer(ITIMER_REAL, &val, &old))
		return 0;
	if (old.it_value.tv_sec > ts.tv_sec)
		return old.it_value.tv_sec - ts.tv_sec;
	return 0;
}
