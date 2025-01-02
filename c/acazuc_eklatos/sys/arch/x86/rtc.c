#include "arch/x86/x86.h"
#include "arch/x86/asm.h"

#include <std.h>

#define RTC_REGSEL 0x70
#define RTC_DATA   0x71

#define RTC_F_NMI 0x80

#define RTC_REG_B_EI 0x40

#define RTC_FREQ 1024

#define RTC_INC (1000000000ULL / RTC_FREQ) /* ~512 ns off per second */

static const int g_mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static struct timespec monotonic_ts;
static struct timespec realtime_ts;

static const struct clock_source realtime_clock_source;
static const struct clock_source monotonic_clock_source;

static struct irq_handle rtc_irq_handle;

static inline uint8_t rdreg(uint8_t reg)
{
	outb(RTC_REGSEL, reg);
	return inb(RTC_DATA);
}

static inline void wrreg(uint8_t reg, uint8_t v)
{
	outb(RTC_REGSEL, reg);
	outb(RTC_DATA, v);
}

static void read_rtc(struct timespec *ts)
{
	ts->tv_sec = 0;
	ts->tv_nsec = 0;
	time_t sec = rdreg(0x00);
	time_t min = rdreg(0x02);
	time_t hour = rdreg(0x04);
	time_t mday = rdreg(0x07);
	time_t mon = rdreg(0x08);
	time_t year = rdreg(0x09);
	uint8_t regb = rdreg(0xB);
	if (!(regb & 0x4))
	{
#define DAA(n) (((n) & 0xF) + (((n) / 16) * 10))
		sec = DAA(sec);
		min = DAA(min);
		hour = DAA(hour & 0x7F) | (hour & 0x80);
		mday = DAA(mday);
		mon = DAA(mon);
		year = DAA(year);
#undef DAA
	}
	mday--;
	mon--;
	if (!(regb & 0x2) && (hour & 0x80))
		hour = ((hour & 0x7F) + 12) % 24;
	if (year < 70)
		year = 100 + year;
	time_t yday = mday;
	for (time_t i = 0; i < mon; ++i)
	{
		int mdays;
		if (i == 1)
		{
			if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
				mdays = 29;
			else
				mdays = 28;
		}
		else
		{
			mdays = g_mdays[i];
		}
		yday += mdays;
	}
	ts->tv_sec = sec + min * 60 + hour * 3600
	           + yday * 86400 + (year - 70) * 31536000
	           + ((year - 69) / 4) * 86400
	           - ((year - 1) / 100) * 86400
	           + ((year + 299) / 400) * 86400;
}

static void rtc_interrupt(void *userdata)
{
	(void)userdata;
	monotonic_ts.tv_nsec += RTC_INC;
	timespec_normalize(&monotonic_ts);
	rdreg(0xC); /* must be read to enable int again */
}

void rtc_init(void)
{
	if (!clock_register(CLOCK_MONOTONIC, &monotonic_clock_source))
	{
		wrreg(0xB | RTC_F_NMI, rdreg(0xB | RTC_F_NMI) | RTC_REG_B_EI);
		if (register_isa_irq(ISA_IRQ_CMOS, rtc_interrupt, NULL,
		                     &rtc_irq_handle))
			panic("rtc: failed to enable IRQ\n");
	}
	if (!clock_register(CLOCK_REALTIME, &realtime_clock_source))
		read_rtc(&realtime_ts);
}

static int realtime_getres(struct timespec *ts)
{
	ts->tv_sec = 1;
	ts->tv_nsec = 0;
	return 0;
}

static int realtime_gettime(struct timespec *ts)
{
	read_rtc(ts);
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

static int monotonic_getres(struct timespec *ts)
{
	ts->tv_sec = 0;
	ts->tv_nsec = RTC_INC;
	return 0;
}

static int monotonic_gettime(struct timespec *ts)
{
	*ts = monotonic_ts;
	return 0;
}

static int monotonic_settime(const struct timespec *ts)
{
	(void)ts; /* XXX */
	return 0;
}

static const struct clock_source monotonic_clock_source =
{
	.name = "RTC",
	.precision = 512,
	.getres = monotonic_getres,
	.gettime = monotonic_gettime,
	.settime = monotonic_settime,
};
