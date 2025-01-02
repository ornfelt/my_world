#include "arch/x86/x86.h"

#include <time.h>
#include <irq.h>
#include <mem.h>

/*
 * IA-PC HPET Specifications 1.0a
 */

#define REG_CAP_ID    (0x000)
#define REG_CONF      (0x010)
#define REG_INT       (0x020)
#define REG_CNTV      (0x0F0)
#define REG_T_CONF(n) (0x100 + (n) * 0x20)
#define REG_T_CMPV(n) (0x108 + (n) * 0x20)
#define REG_T_FSB(n)  (0x110 + (n) * 0x20)

/* REG_CAP_ID */
#define COUNTER_CLK_PERIOD(n) (((n) >> 32) & 0xFFFFFFFF) /* period of tick in femtoseconds */
#define VENDOR_ID(n)          (((n) >> 16) & 0xFFFF) /* vendor id */
#define LEG_RT_CAP            (1 << 15) /* legacy replacement route capable */
#define COUNT_SIZE_CAP        (1 << 13) /* counter size (64bit is 1, 32bit if 0) */
#define NUM_TIM_CAP(n)        (((n) >> 8) & 0x1F) /* number of timers */
#define REV_ID(n)             (((n) >> 0) & 0xFF) /* revision id */

/* REG_CONF */
#define LEG_RT_CNF (1 << 1) /* legacy replacement route */
#define ENABLE_CNF (1 << 0) /* overall enable */

/* REG_INT */
#define TN_INT_STS(n) (1 << (n)n) /* timer n interrupt state */

/* REG_T_CONF */
#define TN_INT_TYPE_CNF     (1 << 1) /* interrupt type (1 = level trigger, 0 = edge trigger) */
#define TN_INT_ENB_CNF      (1 << 2) /* interrupt enable */
#define TN_TYPE_CNF         (1 << 3) /* enable / disable periodic mode */
#define TN_PER_INT_CAP      (1 << 4) /* periodic interrupt capable */
#define TN_SIZE_CAP         (1 << 5) /* size (1 - 64bit, 0 = 32bit) */
#define TN_VAL_SET_CNF      (1 << 6) /* value set */
#define TN_32MODE_CNF       (1 << 8) /* 32bit mode */
#define TN_INT_ROUTE_CNF(n) (((n) >> 9) & 0x1F) /* interrupt route */
#define TN_FSB_EN_CNF       (1 << 14) /* FSB interrupt enable */
#define TN_FSB_INT_DEL_CAP  (1 << 15) /* FSB interrupt delivery */
#define TN_INT_ROUTE_CAP(n) (((n) >> 32) & 0xFFFFFFFF) /* interrupt routing capability */

#define TARGET_INTERVAL 1000 /* target interval in microseconds */

static const struct clock_source clock_source;

static struct page hpet_page;
static uint8_t *hpet_addr;

static uint64_t tick_len; /* in femtoseconds; 10ns on QEMU, ~69ns on my laptop */
static uint32_t freq; /* number of ticks between interrupts */

static struct irq_handle hpet_irq_handle;

static inline void hpet_wr(uint32_t reg, uint64_t v)
{
	*(uint64_t volatile*)&hpet_addr[reg] = v;
}

static inline uint64_t hpet_rd(uint32_t reg)
{
	return *(uint64_t volatile*)&hpet_addr[reg];
}

static inline uint64_t get_stable_ticks(void)
{
#if __SIZE_WIDTH__ == 32
	/* somehow avoid 32bit overflow race condition on 64bit read */
	while (1)
	{
		uint64_t ticks = hpet_rd(REG_CNTV);
		if ((ticks & 0xFFFFFFFF) > 0x00000010
		 && (ticks & 0xFFFFFFFF) < 0xFFFFFFF0)
			return ticks;
	}
#elif __SIZE_WIDTH__ == 64
	return hpet_rd(REG_CNTV);
#else
# error "unknown arch"
#endif
}

static void hpet_interrupt(void *userdata)
{
	(void)userdata;
}

void hpet_init(uint32_t hw_id, uint32_t addr, uint8_t number,
               uint16_t min_clock_ticks)
{
	(void)hw_id;
	(void)number;
	pm_init_page(&hpet_page, addr / PAGE_SIZE);
	hpet_addr = vm_map(&hpet_page, PAGE_SIZE, VM_PROT_RW);
	if (!hpet_addr)
	{
		printf("hpet: failed to vmap\n");
		return;
	}
	uint64_t cap = hpet_rd(REG_CAP_ID);
	if (!REV_ID(cap))
	{
		printf("hpet: null revision ID\n");
		goto err;
	}
#if 0
	printf("min clock ticks: %u\n", min_clock_ticks);
	printf("cap: 0x%016" PRIx64 "\n", cap);
#endif
	if (COUNTER_CLK_PERIOD(cap) > 0x05F5E100)
	{
		printf("hpet: invalid period\n");
		goto err;
	}
	if (!NUM_TIM_CAP(cap))
	{
		printf("hpet: no timer available\n");
		goto err;
	}
	tick_len = COUNTER_CLK_PERIOD(cap);
	if (!(cap & LEG_RT_CAP)) /* XXX support it */
	{
		printf("hpet: doesn't support legact IRQ routing\n");
		goto err;
	}
	if (!(cap & COUNT_SIZE_CAP)) /* XXX support it */
	{
		printf("hpet: doesn't support 64bit counter\n");
		goto err;
	}
	uint64_t conf0 = hpet_rd(REG_T_CONF(0));
	if (!(conf0 & TN_PER_INT_CAP))
	{
		printf("hpet: timer0 doesn't support periodic int\n");
		goto err;
	}
	if (!(conf0 & TN_SIZE_CAP))
	{
		printf("hpet: timer0 doesn't support 64bit counter\n");
		goto err;
	}
	if (!clock_register(CLOCK_MONOTONIC, &clock_source))
	{
		if (register_isa_irq(ISA_IRQ_PIT, hpet_interrupt, NULL,
		                     &hpet_irq_handle))
			panic("hpet: failed to enable IRQ\n");
		hpet_wr(REG_T_CONF(0), TN_INT_ENB_CNF | TN_TYPE_CNF | TN_VAL_SET_CNF);
		freq = (TARGET_INTERVAL * (uint64_t)1000000000) / tick_len;
		if (!freq)
			freq = 1;
		if (freq < min_clock_ticks)
			freq = min_clock_ticks;
		hpet_wr(REG_T_CMPV(0), freq);
		hpet_wr(REG_T_CMPV(0), freq);
		hpet_wr(REG_CNTV, 0);
		hpet_wr(REG_CONF, ENABLE_CNF | LEG_RT_CNF);
	}
	return;

err:
	vm_unmap(hpet_addr, PAGE_SIZE);
}

static int getres(struct timespec *ts)
{
	ts->tv_sec = 0;
	ts->tv_nsec = tick_len * freq / 1000000;
	return 0;
}

static int gettime(struct timespec *ts)
{
	/* XXX it's the best solution to provide real monotonicity for now */
	ts->tv_sec = 0;
	ts->tv_nsec = 0;
	uint64_t ticks = get_stable_ticks();
	uint64_t add;
	add = (((ticks % 1000000000)) * tick_len) / 1000000;
	ts->tv_nsec += add % 1000000000;
	ts->tv_sec += add / 1000000000;
	ticks /= 1000000000;
	add = ((ticks % 1000000000) * tick_len);
	ts->tv_nsec += add % 1000000;
	ts->tv_sec += add / 1000000;
	ticks /= 1000000000;
	add = (ticks * tick_len);
	ts->tv_sec += add * 1000;
	return 0;
}

static int settime(const struct timespec *ts)
{
	(void)ts; /* XXX */
	return 0;
}

static const struct clock_source clock_source =
{
	.name = "HPET",
	.precision = 50,
	.getres = getres,
	.gettime = gettime,
	.settime = settime,
};
