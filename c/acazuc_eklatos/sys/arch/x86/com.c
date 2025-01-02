#include "arch/x86/asm.h"
#include "arch/x86/x86.h"

#include <std.h>
#include <tty.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

#define REG_THR 0x0 /* W / DLAB 0 / Transmitter Holding Buffer */
#define REG_RBR 0x0 /* R / DLAB 0 / Receiver Buffer */
#define REG_DLL 0x0 /* RW / DLAB 1 / Divisor Latch Low Byte */
#define REG_IER 0x1 /* RW / DLAB 0 / Interrupt Enable Register */
#define REG_DLH 0x1 /* RW / DLAB 1 / Divisor Latch High Byte */
#define REG_IIR 0x1 /* R / Interrupt Identification Register */
#define REG_FCR 0x2 /* W / FIFO Control Register */
#define REG_LCR 0x3 /* RW / Line Control Register */
#define REG_MCR 0x4 /* RW / Modem Control Register */
#define REG_LSR 0x5 /* R / Line Status Register */
#define REG_MSR 0x6 /* R / Modem Status Register */
#define REG_SR  0x7 /* RW / Scratch Register */

#define REG_LSR_FRE (1 << 7) /* FIFO Receive Error */
#define REG_LSR_EDR (1 << 6) /* Empty Data Holding Registers */
#define REG_LSR_ETR (1 << 5) /* Empty Transmitter Holding Registers */
#define REG_LSR_BRK (1 << 4) /* Break Interrupt */
#define REG_LSR_FRM (1 << 3) /* Framing Error */
#define REG_LSR_PRT (1 << 2) /* Parity Error */
#define REG_LSR_OVR (1 << 1) /* Overrun Error */
#define REG_LSR_RDY (1 << 0) /* Data Ready */

static struct tty *ttys[4];
static uint16_t ports[4] = {COM1, COM2, COM3, COM4};
static struct irq_handle irq_handles[2];
static int com_status[4];

static int com_mktty(const char *name, int id, struct tty **tty);

static int init_port(uint16_t port)
{
	outb(port + REG_LCR, 0x00); /* disable DLAB */
	outb(port + REG_IER, 0x00); /* disable interrupts */
	outb(port + REG_LCR, 0x80); /* enable DLAB */
	outb(port + REG_DLL, 0x01); /* set baud rate LSB (115200) */
	outb(port + REG_DLH, 0x00); /* set baud rate MSB */
	outb(port + REG_LCR, 0x03); /* disable DLAB, 8 bits, no parity, 1 stop bit */
	outb(port + REG_FCR, 0xE7); /* enable fifo */
	outb(port + REG_MCR, 0x0B); /* enable IRQ */
#if 0
	outb(port + REG_MCR, 0x1E); /* enable loopback */
	outb(port + REG_THR, 0xAE); /* send test */
	if (inb(port + REG_RBR) != 0xAE)
		return 1;
#endif
	outb(port + REG_MCR, 0x0E); /* disable loopback */
	outb(port + REG_IER, 0x01); /* enable received data interrupts */
	return 0;
}

static uint8_t com_read(uint8_t id)
{
	return inb(ports[id] + REG_RBR);
}

static void com_write(uint8_t id, uint8_t c)
{
	size_t i = 0;
	while (!(inb(ports[id] + REG_LSR) & REG_LSR_ETR))
	{
		if (++i == 4096)
			return;
		io_wait();
	}
	outb(ports[id] + REG_THR, c);
}

static void com_early_printf(const char *s, size_t n)
{
	for (size_t i = 0; i < 4; ++i)
	{
		if (!com_status[i])
			continue;
		for (size_t j = 0; j < n; ++j)
		{
			if (s[j] == '\n')
				com_write(i, '\r');
			com_write(i, s[j]);
		}
	}
}

static void handle_read(uint8_t id)
{
	char c = com_read(id);
	if (ttys[id])
		tty_input(ttys[id], &c, 1);
}

static void com1_interrupt(void *userdata)
{
	(void)userdata;
	if (inb(COM1 + REG_LSR) & REG_LSR_RDY)
		handle_read(0);
	if (inb(COM3 + REG_LSR) & REG_LSR_RDY)
		handle_read(2);
}

static void com2_interrupt(void *userdata)
{
	(void)userdata;
	if (inb(COM2 + REG_LSR) & REG_LSR_RDY)
		handle_read(1);
	if (inb(COM4 + REG_LSR) & REG_LSR_RDY)
		handle_read(3);
}

void com_init(void)
{
	if (!init_port(COM1))
		com_status[0] = 1;
	if (!init_port(COM2))
		com_status[1] = 1;
	if (!init_port(COM3))
		com_status[2] = 1;
	if (!init_port(COM4))
		com_status[3] = 1;
	g_early_printf = com_early_printf;
}

void com_init_tty(void)
{
	int com13 = 0;
	int com24 = 0;
	if (com_status[0])
	{
		com13 = 1;
		if (com_mktty("ttyS0", 0, &ttys[0]))
			printf("failed create ttyS0\n");
		else
			printf_addtty(ttys[0]);
	}
	if (com_status[1])
	{
		com24 = 1;
		if (com_mktty("ttyS1", 1, &ttys[1]))
			printf("failed to create ttyS1\n");
	}
	if (com_status[2])
	{
		com13 = 1;
		if (com_mktty("ttyS2", 2, &ttys[2]))
			printf("failed to create ttyS2\n");
	}
	if (com_status[3])
	{
		com24 = 1;
		if (com_mktty("ttyS3", 3, &ttys[3]))
			printf("failed to create ttyS3\n");
	}
	if (com13)
	{
		if (register_isa_irq(ISA_IRQ_COM1, com1_interrupt, NULL,
		                     &irq_handles[0]))
			panic("com: failed to enable IRQ1\n");
	}
	if (com24)
	{
		if (register_isa_irq(ISA_IRQ_COM2, com2_interrupt, NULL,
		                     &irq_handles[1]))
			panic("com: failed to enable IRQ2\n");
	}
}

struct com_tty
{
	uint8_t id;
};

static ssize_t com_tty_write(struct tty *tty, const void *data, size_t len)
{
	struct com_tty *com_tty = tty->userdata;
	for (size_t i = 0; i < len; ++i)
		com_write(com_tty->id, ((uint8_t*)data)[i]);
	return len;
}

static const struct tty_op g_tty_op =
{
	.write = com_tty_write,
};

static int com_mktty(const char *name, int id, struct tty **tty)
{
	struct com_tty *com_tty = malloc(sizeof(*com_tty), M_ZERO);
	if (!com_tty)
		return -ENOMEM;
	com_tty->id = id;
	int res = tty_alloc(name, makedev(4, 64 + id), &g_tty_op, tty);
	if (res)
	{
		free(com_tty);
		return res;
	}
	(*tty)->flags |= TTY_NOCTRL;
	(*tty)->termios.c_oflag &= ~OPOST;
	(*tty)->termios.c_oflag |= ONLCR;
	(*tty)->termios.c_iflag &= ~IGNCR;
	(*tty)->termios.c_iflag |= INLCR;
	(*tty)->userdata = com_tty;
	return 0;
}
