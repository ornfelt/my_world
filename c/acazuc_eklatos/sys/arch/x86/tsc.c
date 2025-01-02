#include "arch/x86/asm.h"
#include "arch/x86/x86.h"

#include <time.h>
#include <std.h>

static const struct clock_source clock_source;
static uint64_t tsc_base;
static uint64_t tsc_freq;

void tsc_init(void)
{
	struct timespec start;
	struct timespec end;
	uint64_t tsc_start;
	uint64_t tsc_end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	tsc_start = rdtsc();
	while (1)
	{
		struct timespec diff;
		tsc_end = rdtsc();
		clock_gettime(CLOCK_MONOTONIC, &end);
		timespec_sub(&diff, &end, &start);
		if (diff.tv_sec)
			break;
	}
	tsc_base = tsc_start;
	tsc_freq = tsc_end - tsc_start;
#if 0
	printf("tsc frequency: %lu.%09lu GHz\n", tsc_freq / 1000000000, tsc_freq % 1000000000);
#endif
	clock_register(CLOCK_MONOTONIC, &clock_source);
}

static int getres(struct timespec *ts)
{
	ts->tv_sec = 0;
	if (tsc_freq > 1000000000)
		ts->tv_nsec = 0;
	else
		ts->tv_nsec = 1000000000 / tsc_freq;
	return 0;
}

static int gettime(struct timespec *ts)
{
	uint64_t ticks = rdtsc() - tsc_base;
	ts->tv_sec = ticks / tsc_freq;
	ticks %= tsc_freq;
	ts->tv_nsec = (ticks * 1000000000) / tsc_freq;
	return 0;
}

static int settime(const struct timespec *ts)
{
	(void)ts; /* XXX */
	return 0;
}

static const struct clock_source clock_source =
{
	.name = "TSC",
	.precision = 1,
	.getres = getres,
	.gettime = gettime,
	.settime = settime,
};
