#define ENABLE_TRACE

#include <endian.h>
#include <errno.h>
#include <time.h>
#include <std.h>
#include <fdt.h>
#include <mem.h>

#define RTC_TIME_LOW        0x00
#define RTC_TIME_HIGH       0x04
#define RTC_ALARM_LOW       0x08
#define RTC_ALARM_HIGH      0x0C
#define RTC_IRQ_ENABLED     0x10
#define RTC_CLEAR_ALARM     0x14
#define RTC_ALARM_STATUS    0x18
#define RTC_CLEAR_INTERRUPT 0x1C

static uint8_t *rtc_data;
static const struct clock_source realtime_clock_source;

static inline uint32_t rtc_read(uint32_t reg)
{
	return *(uint32_t volatile*)&rtc_data[reg];
}

static inline void rtc_write(uint32_t reg, uint32_t val)
{
	*(uint32_t volatile*)&rtc_data[reg] = val;
}

static void read_ts(struct timespec *ts)
{
	uint32_t vlo = rtc_read(RTC_TIME_LOW);
	int32_t vhi = (int32_t)rtc_read(RTC_TIME_HIGH);
	time_t val = (vhi * (time_t)0x100000000) + vlo;
	ts->tv_sec = val / 1000000000;
	ts->tv_nsec = val % 1000000000;
}

int rtc_init_addr(uintptr_t addr)
{
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	rtc_data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!rtc_data)
	{
		TRACE("rtc: failed to map data");
		return -ENOMEM;
	}
	int ret = clock_register(CLOCK_REALTIME, &realtime_clock_source);
	if (ret)
	{
		TRACE("rtc: failed to register clock");
		return ret;
	}
	struct timespec ts;
	read_ts(&ts);
	return 0;
}

int rtc_init_fdt(struct fdt_node *node)
{
	int ret;
	struct fdt_prop *reg = fdt_get_prop(node, "reg");
	if (!reg)
	{
		TRACE("rtc: no 'reg' property");
		return -EINVAL;
	}
	uintptr_t mmio_base;
	size_t mmio_size;
	ret = fdt_get_base_size_reg(reg, 0, &mmio_base, &mmio_size);
	if (ret)
	{
		TRACE("rtc: invalid reg");
		return ret;
	}
	return rtc_init_addr(mmio_base);
}

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
