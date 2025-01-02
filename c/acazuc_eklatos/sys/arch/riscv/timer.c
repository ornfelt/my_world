#define ENABLE_TRACE

#include "arch/riscv/sbi.h"

#include <arch/asm.h>
#include <arch/csr.h>

#include <endian.h>
#include <errno.h>
#include <time.h>
#include <fdt.h>
#include <irq.h>
#include <std.h>
#include <mem.h>

static const struct clock_source clock_source;
static uint32_t freq;
static uint64_t base;
static uint64_t current;
static uint64_t nextval;
static uint64_t interval;
static uint64_t interval_rem;
static uint64_t interval_cnt;

#define FREQUENCY 1000

static inline uint64_t csrr_time(void)
{
#if __riscv_xlen == 64
	return csrr(CSR_TIME);
#else
	/* somehow avoid 32bit overflow race condition on 64bit read */
	while (1)
	{
		uint64_t value = ((uint64_t)csrr(CSR_TIME) << 0)
		               | ((uint64_t)csrr(CSR_TIMEH) << 32);
		if ((value & 0xFFFFFFFF) > 0x00000010
		 && (value & 0xFFFFFFFF) < 0xFFFFFFF0)
			return value;
	}
#endif
}

static inline void csrw_stimecmp(uint64_t value)
{
	/* STIMECMP required sstc extension, sbi handles everything better
	 * with a very little overhead
	 * XXX use STIMECMP if available
	 */
#if 1
	sbi_set_timer(value);
#else
#if __riscv_xlen == 64
	csrw(CSR_STIMECMP, value);
#else
	csrw(CSR_STIMECMPH, value >> 32);
	csrw(CSR_STIMECMP, value);
#endif
#endif
}

void timer_interrupt(const struct irq_ctx *ctx, void *userdata)
{
	(void)ctx;
	(void)userdata;
	current = nextval;
	nextval = current + interval;
	if (++interval_cnt == FREQUENCY)
	{
		interval_cnt = 0;
		nextval += interval_rem;
	}
	csrw_stimecmp(nextval);
}

static int get_frequency(void)
{
	struct fdt_node *node;
	TAILQ_FOREACH(node, &fdt_nodes, chain)
	{
		struct fdt_node *child;
		TAILQ_FOREACH(child, &node->children, chain)
		{
			if (strcmp(child->name, "cpus"))
				continue;
			struct fdt_prop *freq_prop = fdt_get_prop(child, "timebase-frequency");
			if (!freq_prop)
				continue;
			if (freq_prop->len != 4)
				continue;
			freq = be32toh(*(uint32_t*)&freq_prop->data[0]);
			return 0;
		}
	}
	return -ENOENT;
}

int timer_init(void)
{
	int ret = get_frequency();
	if (ret)
	{
		TRACE("timer: failed to get frequency");
		return ret;
	}
	base = csrr_time();
	current = base;
	interval = freq / FREQUENCY;
	interval_rem = freq % FREQUENCY;
	csrw_stimecmp(base + interval);
#if 0
	printf("timer frequency: %" PRIu32 ".%" PRIu32 " MHz\n", freq / 1000000, (freq / 10000) % 100);
#endif
	ret = clock_register(CLOCK_MONOTONIC, &clock_source);
	if (ret)
	{
		TRACE("timer: failed to register clock");
		return ret;
	}
	return 0;
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
	uint64_t ticks = csrr_time() - base;
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
	.name = "mtime",
	.precision = 1,
	.getres = getres,
	.gettime = gettime,
	.settime = settime,
};
