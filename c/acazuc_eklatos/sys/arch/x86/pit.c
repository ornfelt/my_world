#include "arch/x86/asm.h"
#include "arch/x86/x86.h"

#include <std.h>

#define DATA0 0x40
#define DATA1 0x41
#define DATA2 0x42
#define CMD   0x43

#define CMD_CHAN_0  0x00
#define CMD_CHAN_1  0x40
#define CMD_CHAN_2  0x80
#define CMD_CHAN_RB 0xC0

#define CMD_ACCESS_LATCH  0x00
#define CMD_ACCESS_LOBYTE 0x10
#define CMD_ACCESS_HIBYTE 0x20
#define CMD_ACCESS_MBYTE  0x30

#define CMD_OP_MODE0 0x00
#define CMD_OP_MODE1 0x02
#define CMD_OP_MODE2 0x04
#define CMD_OP_MODE3 0x06
#define CMD_OP_MODE4 0x08
#define CMD_OP_MODE5 0x09

#define PIT_FREQ 1193182

#define PIT_DIV  1193 /* ~1kHz */
#define PIT_INC  (1000000000ULL * PIT_DIV / PIT_FREQ)

static struct timespec monotonic_ts;
static struct irq_handle pit_irq_handle;

static const struct clock_source clock_source;

static void pit_interrupt(void *userdata)
{
	(void)userdata;
	monotonic_ts.tv_nsec += PIT_INC / 2; /* XXX fix /2 by changing lapic flags */
	timespec_normalize(&monotonic_ts);
}

void pit_init(void)
{
	return;
	outb(CMD, CMD_CHAN_0 | CMD_ACCESS_MBYTE | CMD_OP_MODE3);
	outb(DATA0, PIT_DIV & 0xFF);
	outb(DATA0, (PIT_DIV >> 8) & 0xFF);
	if (!clock_register(CLOCK_MONOTONIC, &clock_source))
	{
		if (register_isa_irq(ISA_IRQ_PIT, pit_interrupt, NULL,
		                     &pit_irq_handle))
			panic("pit: failed to enable IRQ\n");
	}
}

static int getres(struct timespec *ts)
{
	ts->tv_sec = 0;
	ts->tv_nsec = PIT_INC;
	return 0;
}

static int gettime(struct timespec *ts)
{
	*ts = monotonic_ts;
	return 0;
}

static int settime(const struct timespec *ts)
{
	(void)ts; /* XXX */
	return 0;
}

static const struct clock_source clock_source =
{
	.name = "PIT",
	.precision = 100,
	.getres = getres,
	.gettime = gettime,
	.settime = settime,
};
