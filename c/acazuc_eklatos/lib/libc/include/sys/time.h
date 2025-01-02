#ifndef SYS_TIME_H
#define SYS_TIME_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

struct timeval
{
	time_t tv_sec;
	time_t tv_usec;
};

struct timezone
{
	int tz_minuteswest;
	int tz_dsttime;
};

struct itimerval
{
	struct timeval it_interval;
	struct timeval it_value;
};

int gettimeofday(struct timeval *tv, struct timezone *tz);
int getitimer(int which, struct itimerval *cur);
int setitimer(int which, const struct itimerval *val, struct itimerval *old);

#ifdef __cplusplus
}
#endif

#endif
