#include <errno.h>
#include <timer.h>
#include <time.h>
#include <file.h>
#include <std.h>
#include <uio.h>
#include <vfs.h>

struct timespec g_boottime;

static const struct clock_source *clock_monotonic;
static const struct clock_source *clock_realtime;
static struct timespec realtime_resync;
static struct timespec realtime_base;
static struct timer realtime_timer;

static const struct clock_source *get_source(int clock)
{
	switch (clock)
	{
		case CLOCK_REALTIME:
			return clock_realtime;
		case CLOCK_MONOTONIC:
			return clock_monotonic;
		default:
			return NULL;
	}
}

static int set_source(int clock, const struct clock_source *source)
{
	switch (clock)
	{
		case CLOCK_REALTIME:
			clock_realtime = source;
			return 0;
		case CLOCK_MONOTONIC:
			clock_monotonic = source;
			return 0;
		default:
			return -EINVAL;
	}
}

static void realtime_adjust(struct timer *timer)
{
	struct timespec realtime_ts;
	struct timespec rtc_ts;
	struct timespec delta;
	int diff;

	(void)timer;
	const struct clock_source *source = get_source(CLOCK_REALTIME);
	if (!source)
		return;
	if (source->gettime(&rtc_ts))
		return;
	if (clock_gettime(CLOCK_REALTIME, &realtime_ts))
		panic("failed to get time\n");
	/* XXX this is really bad, but it's going to be reworked
	 * at some point for NTP support
	 */
	diff = timespec_cmp(&realtime_ts, &rtc_ts);
	if (diff > 0)
	{
		timespec_sub(&delta, &realtime_ts, &rtc_ts);
		if (delta.tv_sec)
		{
			delta.tv_nsec /= 2;
			delta.tv_sec--;
			if (delta.tv_sec % 2)
				delta.tv_nsec -= 500000000;
			delta.tv_sec /= 2;
			timespec_normalize(&delta);
			timespec_add(&realtime_base, &realtime_base, &delta);
		}
	}
	else if (diff < 0)
	{
		timespec_sub(&delta, &rtc_ts, &realtime_ts);
		if (delta.tv_sec)
		{
			delta.tv_nsec /= 2;
			delta.tv_sec--;
			if (delta.tv_sec % 2)
				delta.tv_nsec -= 500000000;
			delta.tv_sec /= 2;
			timespec_normalize(&delta);
			timespec_sub(&realtime_base, &realtime_base, &delta);
		}
	}
	realtime_resync.tv_sec++;
	timer_add(&realtime_timer, CLOCK_MONOTONIC, realtime_resync,
	          realtime_adjust, NULL);
}

void clock_init(void)
{
	if (clock_gettime(CLOCK_MONOTONIC, &realtime_resync))
		panic("failed to get time\n");
	realtime_resync.tv_sec++;
	timer_add(&realtime_timer, CLOCK_MONOTONIC, realtime_resync,
	          realtime_adjust, NULL);
}

int clock_register(int clock, const struct clock_source *source)
{
	const struct clock_source *current = get_source(clock);
	if (!current)
		return set_source(clock, source);
	if (current->precision < source->precision)
		return -EINVAL;
	return set_source(clock, source);
}

int clock_getres(int clock, struct timespec *ts)
{
	const struct clock_source *source = get_source(clock);
	if (!source)
		return -EINVAL;
	return source->getres(ts);
}

int clock_gettime(int clock, struct timespec *ts)
{
	if (clock == CLOCK_REALTIME)
	{
		struct timespec monotonic_ts;
		int ret = clock_gettime(CLOCK_MONOTONIC, &monotonic_ts);
		if (ret)
			return ret;
		if (!realtime_base.tv_sec)
		{
			struct timespec realtime_ts;
			const struct clock_source *source = get_source(CLOCK_REALTIME);
			if (source)
			{
				ret = source->gettime(&realtime_ts);
				if (ret)
					return ret;
			}
			else
			{
				/* no realtime clock available, consider booting
				 * from epoch
				 */
				realtime_ts = monotonic_ts;
			}
			timespec_sub(&realtime_base, &monotonic_ts, &realtime_ts);
		}
		timespec_sub(ts, &monotonic_ts, &realtime_base);
		return 0;
	}
	const struct clock_source *source = get_source(clock);
	if (!source)
		return -EINVAL;
	return source->gettime(ts);
}

int clock_settime(int clock, const struct timespec *ts)
{
	const struct clock_source *source = get_source(clock);
	if (!source)
		return -EINVAL;
	return source->settime(ts);
}

time_t realtime_seconds(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts))
		return 0;
	return ts.tv_sec;
}

void spinsleep(const struct timespec *duration)
{
	struct timespec begin;
	struct timespec end;
	struct timespec diff;
	clock_gettime(CLOCK_MONOTONIC, &begin);
	while (1)
	{
		clock_gettime(CLOCK_MONOTONIC, &end);
		timespec_sub(&diff, &end, &begin);
		if (timespec_cmp(&diff, duration) >= 0)
			return;
	}
}

static ssize_t uptime_read(struct file *file, struct uio *uio)
{
	(void)file;
	size_t count = uio->count;
	off_t off = uio->off;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	struct timespec diff;
	timespec_sub(&diff, &ts, &g_boottime);
	uprintf(uio, "%" PRIu64 ".%09" PRIu64 "\n", diff.tv_sec, diff.tv_nsec);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op uptime_fop =
{
	.read = uptime_read,
};

int uptime_register_sysfs(void)
{
	return sysfs_mknode("uptime", 0, 0, 0444, &uptime_fop, NULL);
}
