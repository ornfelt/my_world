#include <unistd.h>
#include <time.h>

unsigned int
sleep(unsigned int seconds)
{
	struct timespec ts;
	struct timespec rem;

	ts.tv_sec = seconds;
	ts.tv_nsec = 0;
	if (nanosleep(&ts, &rem) == -1)
		return rem.tv_sec;
	return 0;
}
