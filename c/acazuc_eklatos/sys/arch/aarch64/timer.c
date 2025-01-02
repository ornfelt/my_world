#include <arch/asm.h>

#include <time.h>
#include <irq.h>
#include <std.h>

static const struct clock_source clock_source;
static struct irq_handle vtimer_irq;
static size_t freq;
static uint64_t base;
static uint64_t current;
static uint64_t nextval;
static uint64_t interval;
static uint64_t interval_rem;
static uint64_t interval_cnt;

#define FREQUENCY 1000

#if defined(__arm__)
#define set_cntv_cval_el0(x) set_cntv_cval(x)
#define get_cntfrq_el0() get_cntfrq()
#define get_cntvct_el0() get_cntvct()
#define set_cntv_ctl_el0(x) set_cntv_ctl(x)
#endif

static void vtimer_interrupt(void *userdata)
{
	(void)userdata;
	current = nextval;
	nextval = current + interval;
	if (++interval_cnt == FREQUENCY)
	{
		interval_cnt = 0;
		nextval += interval_rem;
	}
	set_cntv_cval_el0(nextval);
}

void timer_init(void)
{
	if (arch_register_native_irq(27, vtimer_interrupt, NULL, &vtimer_irq))
		panic("timer: failed to register irq\n");
	freq = get_cntfrq_el0();
	base = get_cntvct_el0();
	current = base;
	interval = freq / FREQUENCY;
	interval_rem = freq % FREQUENCY;
	set_cntv_cval_el0(base + interval);
	set_cntv_ctl_el0(1);
#if 0
	printf("vtimer frequency: %lu.%lu MHz\n", freq / 1000000, (freq / 10000) % 100);
#endif
	if (clock_register(CLOCK_MONOTONIC, &clock_source))
		panic("timer: failed to register clock\n");
}

static int getres(struct timespec *ts)
{
	ts->tv_sec = 0;
	if (freq > 1000000000)
		ts->tv_nsec = 0;
	else
		ts->tv_nsec = 1000000000 / freq;
	return 0;
}

static int gettime(struct timespec *ts)
{
	uint64_t ticks = get_cntvct_el0() - base;
	ts->tv_sec = ticks / freq;
	ticks %= freq;
	ts->tv_nsec = (ticks * 1000000000) / freq;
	return 0;
}

static int settime(const struct timespec *ts)
{
	(void)ts; /* XXX */
	return 0;
}

static const struct clock_source clock_source =
{
	.name = "vtimer",
	.precision = 1,
	.getres = getres,
	.gettime = gettime,
	.settime = settime,
};
