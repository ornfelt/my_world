#define ENABLE_TRACE

#include <endian.h>
#include <time.h>
#if WITH_FDT
#include <fdt.h>
#endif
#include <mem.h>

static uint32_t *pl031_data;
static const struct clock_source realtime_clock_source;

static void read_ts(struct timespec *ts)
{
	uint32_t value = *pl031_data;
	ts->tv_sec = value;
	ts->tv_nsec = 0;
}

int pl031_init_addr(uintptr_t addr)
{
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	pl031_data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!pl031_data)
	{
		TRACE("pl031: failed to map data");
		return -ENOMEM;
	}
	int ret = clock_register(CLOCK_REALTIME, &realtime_clock_source);
	if (ret)
	{
		TRACE("pl031: failed to register clock");
		return ret;
	}
	return 0;
}

#if WITH_FDT
int pl031_init_fdt(struct fdt_node *node)
{
	int ret;
	struct fdt_prop *reg = fdt_get_prop(node, "reg");
	if (!reg)
	{
		TRACE("pl031: no 'reg' property");
		return -EINVAL;
	}
	uintptr_t mmio_base;
	size_t mmio_size;
	ret = fdt_get_base_size_reg(reg, 0, &mmio_base, &mmio_size);
	if (ret)
	{
		TRACE("pl031: invalid reg");
		return ret;
	}
	return pl031_init_addr(mmio_base);
}
#endif

static int realtime_getres(struct timespec *ts)
{
	ts->tv_sec = 1;
	ts->tv_nsec = 0;
	return 0;
}

static int realtime_gettime(struct timespec *ts)
{
	read_ts(ts);
	return 0;
}

static int realtime_settime(const struct timespec *ts)
{
	(void)ts; /* XXX */
	return 0;
}

static const struct clock_source realtime_clock_source =
{
	.precision = 1000000000,
	.getres = realtime_getres,
	.gettime = realtime_gettime,
	.settime = realtime_settime,
};
