#ifndef TIME_H
#define TIME_H

#include <errno.h>
#include <types.h>

#define CLOCK_REALTIME  0
#define CLOCK_MONOTONIC 1

#define ITIMER_REAL    0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF    2

struct timeval
{
	time_t tv_sec;
	time_t tv_usec;
};

struct timespec
{
	time_t tv_sec;
	time_t tv_nsec;
};

struct clock_source
{
	const char *name;
	time_t precision; /* in ns, informative */
	int (*getres)(struct timespec *ts);
	int (*gettime)(struct timespec *ts);
	int (*settime)(const struct timespec *ts);
};

struct tms
{
	clock_t tms_utime;
	clock_t tms_stime;
	clock_t tms_cutime;
	clock_t tms_cstime;
};

struct itimerval
{
	struct timeval it_interval;
	struct timeval it_value;
};

void clock_init(void);

int clock_register(int clock, const struct clock_source *source);

int clock_getres(int clock, struct timespec *ts);
int clock_gettime(int clock, struct timespec *ts);
int clock_settime(int clock, const struct timespec *ts);

time_t realtime_seconds(void);

void spinsleep(const struct timespec *duration);

int uptime_register_sysfs(void);

static int timeval_validate(const struct timeval *tv)
{
	if (tv->tv_usec < 0
	 || tv->tv_usec >= 1000000)
		return -EINVAL;
	return 0;
}

static int timespec_validate(const struct timespec *ts)
{
	if (ts->tv_nsec < 0
	 || ts->tv_nsec >= 1000000000)
		return -EINVAL;
	return 0;
}

static void timeval_normalize(struct timeval *tv)
{
	if (tv->tv_usec >= 1000000)
	{
		tv->tv_sec += tv->tv_usec / 1000000;
		tv->tv_usec %= 1000000;
	}
	else if (tv->tv_usec < 0)
	{
		tv->tv_sec += tv->tv_usec / 1000000;
		tv->tv_usec = (1000000 - tv->tv_usec % 1000000);
	}
}

static void timespec_normalize(struct timespec *ts)
{
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec += ts->tv_nsec / 1000000000;
		ts->tv_nsec %= 1000000000;
	}
	else if (ts->tv_nsec < 0)
	{
		ts->tv_sec += ts->tv_nsec / 1000000000;
		ts->tv_nsec = (1000000000 - ts->tv_nsec % 1000000000);
	}
}

static inline void timeval_from_timespec(struct timeval *tv,
                                         struct timespec *ts)
{
	tv->tv_sec = ts->tv_sec;
	tv->tv_usec = ts->tv_nsec / 1000;
}

static inline void timespec_from_timeval(struct timespec *ts,
                                         struct timeval *tv)
{
	ts->tv_sec = tv->tv_sec;
	ts->tv_nsec = tv->tv_usec * 1000;
}

static inline void timespec_add(struct timespec *d,
                                const struct timespec *a,
                                const struct timespec *b)
{
	d->tv_sec = a->tv_sec + b->tv_sec;
	d->tv_nsec = a->tv_nsec + b->tv_nsec;
	if (d->tv_nsec >= 1000000000)
	{
		d->tv_sec++;
		d->tv_nsec -= 1000000000;
	}
}

static inline void timeval_add(struct timeval *d,
                               const struct timeval *a,
                               const struct timeval *b)
{
	d->tv_sec = a->tv_sec + b->tv_sec;
	d->tv_usec = a->tv_usec + b->tv_usec;
	if (d->tv_usec >= 1000000)
	{
		d->tv_sec++;
		d->tv_usec -= 1000000;
	}
}

static inline void timespec_sub(struct timespec *d,
                                const struct timespec *a,
                                const struct timespec *b)
{
	d->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_nsec >= b->tv_nsec)
	{
		d->tv_nsec = a->tv_nsec - b->tv_nsec;
	}
	else
	{
		d->tv_nsec = 1000000000 - (b->tv_nsec - a->tv_nsec);
		d->tv_sec--;
	}
}

static inline void timeval_sub(struct timeval *d,
                               const struct timeval *a,
                               const struct timeval *b)
{
	d->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_usec >= b->tv_usec)
	{
		d->tv_usec = a->tv_usec - b->tv_usec;
	}
	else
	{
		d->tv_usec = 1000000 - (b->tv_usec - a->tv_usec);
		d->tv_sec--;
	}
}

static inline int timespec_cmp(const struct timespec *a,
                               const struct timespec *b)
{
	if (a->tv_sec < b->tv_sec)
		return -1;
	if (a->tv_sec > b->tv_sec)
		return 1;
	if (a->tv_nsec < b->tv_nsec)
		return -1;
	if (a->tv_nsec > b->tv_nsec)
		return 1;
	return 0;
}

static inline int timeval_cmp(const struct timeval *a,
                              const struct timeval *b)
{
	if (a->tv_sec < b->tv_sec)
		return -1;
	if (a->tv_sec > b->tv_sec)
		return 1;
	if (a->tv_usec < b->tv_usec)
		return -1;
	if (a->tv_usec > b->tv_usec)
		return 1;
	return 0;
}

extern struct timespec g_boottime;

#endif
