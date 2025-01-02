#define ENABLE_TRACE

#include <endian.h>
#include <std.h>
#include <fdt.h>
#include <irq.h>
#include <tty.h>
#include <mem.h>

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

struct uart
{
	uint8_t *data;
	struct tty *tty;
	struct irq_handle irq;
};

static inline uint8_t uart_read(struct uart *uart, uint32_t reg)
{
	return *(uint8_t volatile*)&uart->data[reg];
}

static inline void uart_write(struct uart *uart, uint32_t reg, uint8_t val)
{
	*(uint8_t volatile*)&uart->data[reg] = val;
}

static void uart_interrupt(void *userdata)
{
	struct uart *uart = userdata;
	if (uart_read(uart, REG_LSR) & REG_LSR_RDY)
	{
		char c = uart_read(uart, REG_RBR);
		tty_input(uart->tty, &c, 1);
	}
}

static ssize_t uart_tty_write(struct tty *tty, const void *data, size_t len)
{
	struct uart *uart = tty->userdata;
	for (size_t i = 0; i < len; ++i)
	{
		size_t n = 0;
		while (!(uart_read(uart, REG_LSR) & REG_LSR_ETR))
		{
			if (++n == 4096)
				return -ETIMEDOUT;
			arch_spin_yield();
		}
		uart_write(uart, REG_THR, ((uint8_t*)data)[i]);
	}
	return len;
}

static const struct tty_op tty_op =
{
	.write = uart_tty_write,
};

static void uart_free(struct uart *uart)
{
	if (!uart)
		return;
	if (uart->data)
		vm_unmap(uart->data, PAGE_SIZE);
	free(uart);
}

static int uart_init(uintptr_t addr, size_t int_id)
{
	struct uart *uart = NULL;
	int ret;

	uart = malloc(sizeof(*uart), M_ZERO);
	if (!uart)
	{
		TRACE("uart: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ret = arch_register_native_irq(int_id, uart_interrupt, uart,
	                               &uart->irq);
	if (ret)
	{
		TRACE("uart: failed to register IRQ");
		goto err;
	}
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	uart->data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!uart->data)
	{
		TRACE("uart: failed to map data");
		ret = -ENOMEM;
		goto err;
	}
	for (int i = 0; ; ++i)
	{
		if (i == 64)
		{
			TRACE("uart: no more ttyS available");
			ret = -EXDEV;
			goto err;
		}
		char name[64];
		snprintf(name, sizeof(name), "ttyS%d", i);
		ret = tty_alloc(name, makedev(4, 64 + i), &tty_op, &uart->tty);
		if (!ret)
			break;
		if (ret != -EEXIST)
		{
			TRACE("uart: failed to create TTY: %s", strerror(ret));
			goto err;
		}
	}

	uart_write(uart, REG_LCR, 0x00); /* disable DLAB */
	uart_write(uart, REG_IER, 0x00); /* disable interrupts */
	uart_write(uart, REG_LCR, 0x80); /* enable DLAB */
	uart_write(uart, REG_DLL, 0x01); /* set baud rate LSB (115200) */
	uart_write(uart, REG_DLH, 0x00); /* set baud rate MSB */
	uart_write(uart, REG_LCR, 0x03); /* disable DLAB, 8 bits, no parity, 1 stop bit */
	uart_write(uart, REG_FCR, 0xE7); /* enable fifo */
	uart_write(uart, REG_MCR, 0x0B); /* enable IRQ */
#if 0
	uart_write(uart, EG_MCR, 0x1E); /* enable loopback */
	uart_write(uart, EG_THR, 0xAE); /* send test */
	if (uart_read(uart, REG_RBR) != 0xAE)
		return 1;
#endif
	uart_write(uart, REG_MCR, 0x0E); /* disable loopback */
	uart_write(uart, REG_IER, 0x01); /* enable received data interrupts */

	uart->tty->userdata = uart;
	uart->tty->flags |= TTY_NOCTRL;
	uart->tty->termios.c_oflag &= ~OPOST;
	uart->tty->termios.c_oflag |= ONLCR;
	uart->tty->termios.c_iflag &= ~IGNCR;
	uart->tty->termios.c_iflag |= INLCR;
	printf_addtty(uart->tty);
	return 0;

err:
	uart_free(uart);
	return ret;
}

int uart_init_fdt(struct fdt_node *node)
{
	int ret;
	struct fdt_prop *reg = fdt_get_prop(node, "reg");
	if (!reg)
	{
		TRACE("uart: no 'reg' property");
		return -EINVAL;
	}
	uintptr_t mmio_base;
	size_t mmio_size;
	ret = fdt_get_base_size_reg(reg, 0, &mmio_base, &mmio_size);
	if (ret)
	{
		TRACE("uart: invalid reg");
		return ret;
	}
	struct fdt_prop *interrupts = fdt_get_prop(node, "interrupts");
	if (!interrupts)
	{
		TRACE("uart: no 'interrupts' property");
		return -EINVAL;
	}
	if (interrupts->len != 4)
	{
		TRACE("uart: invalid interrupts property length");
		return -EINVAL;
	}
	uint32_t irq_id = be32toh(*(uint32_t*)&interrupts->data[0]);
	return uart_init(mmio_base, irq_id);
}
