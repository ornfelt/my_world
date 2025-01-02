#define ENABLE_TRACE

#include <endian.h>
#if WITH_ACPI
#include <acpi.h>
#endif
#include <tty.h>
#include <irq.h>
#if WITH_FDT
#include <fdt.h>
#endif
#include <mem.h>

#define REG_UARTDR        0x000 /* data */
#define REG_UARTRSR       0x004 /* receive status / error clear */
#define REG_UARTFR        0x018 /* flag */
#define REG_UARTILPR      0x020 /* IrDA low-power counter */
#define REG_UARTIBRD      0x024 /* integer baud rate */
#define REG_UARTFBRD      0x028 /* fractional baud rate */
#define REG_UARTLCR_H     0x02C /* line control */
#define REG_UARTCR        0x030 /* control */
#define REG_UARTIFLS      0x034 /* interrupt fifo level select */
#define REG_UARTIMSC      0x038 /* interrupt mask set/clear */
#define REG_UARTRIS       0x03C /* raw interrupt status */
#define REG_UARTMIS       0x040 /* masked interrupt status */
#define REG_UARTICR       0x044 /* interrupt clear */
#define REG_UARTDMACR     0x048 /* DMA control */
#define REG_UARTPeriphID0 0xFE0
#define REG_UARTPeriphID1 0xFE4
#define REG_UARTPeriphID2 0xFE8
#define REG_UARTPeriphID3 0xFEC
#define REG_UARTCellID0   0xFF0
#define REG_UARTCellID1   0xFF4
#define REG_UARTCellID2   0xFF8
#define REG_UARTCellID3   0xFFC

#define INT_RIM  (1 << 0) /* RI modem */
#define INT_CTSM (1 << 1) /* CTS modem */
#define INT_DCDM (1 << 2) /* DCD modem */
#define INT_DSRM (1 << 3) /* DSR modem */
#define INT_RX   (1 << 4) /* receive */
#define INT_TX   (1 << 5) /* transmit */
#define INT_RT   (1 << 6) /* receive timeout */
#define INT_FE   (1 << 7) /* framing error */
#define INT_PE   (1 << 8) /* parity error */
#define INT_BE   (1 << 9) /* break error */
#define INT_OE   (1 << 10) /* overrun error */

#define FR_CTS  (1 << 0) /* clear to send */
#define FR_DSR  (1 << 1) /* data set read */
#define FR_DCD  (1 << 2) /* data carrier detect */
#define FR_BUSY (1 << 3) /* UART busy */
#define FR_RXFE (1 << 4) /* receive fifo empty */
#define FR_TXFF (1 << 5) /* transmit fifo full */
#define FR_RXFF (1 << 6) /* receive fifo full */
#define FR_TXFE (1 << 7) /* transmit fifo empty */
#define FR_RI   (1 << 8) /* ring indicator */

#define SR_FE (1 << 0) /* framing error */
#define SR_PE (1 << 1) /* parity error */
#define SR_BE (1 << 2) /* break error */
#define SR_OE (1 << 3) /* overrun error */

#define DMAC_RXDMAE   (1 << 0) /* receive DMA enable */
#define DMAC_TXDMAE   (1 << 1) /* transmit DMA enable */
#define DMAC_DMAONERR (1 << 2) /* DMA on error */

struct pl011
{
	uint8_t *data;
	struct tty *tty;
	struct irq_handle irq;
};

static inline uint32_t pl011_read(struct pl011 *pl011, uint32_t reg)
{
	return *(uint32_t volatile*)&pl011->data[reg];
}

static inline void pl011_write(struct pl011 *pl011, uint32_t reg, uint32_t val)
{
	*(uint32_t volatile*)&pl011->data[reg] = val;
}

static ssize_t pl011_tty_write(struct tty *tty, const void *data, size_t len)
{
	struct pl011 *pl011 = tty->userdata;
	for (size_t i = 0; i < len; ++i)
	{
		while (pl011_read(pl011, REG_UARTFR) & FR_TXFF)
			;
		pl011_write(pl011, REG_UARTDR, ((uint8_t*)data)[i]);
	}
	return len;
}

static const struct tty_op g_tty_op =
{
	.write = pl011_tty_write,
};

static void pl011_interrupt(void *userdata)
{
	struct pl011 *pl011 = userdata;
	uint32_t mis = pl011_read(pl011, REG_UARTMIS);
	pl011_write(pl011, REG_UARTICR, mis);
	if (mis & INT_RX)
	{
		while (!(pl011_read(pl011, REG_UARTFR) & FR_RXFE))
		{
			char c = pl011_read(pl011, REG_UARTDR);
			tty_input(pl011->tty, &c, 1);
		}
	}
}

static void pl011_free(struct pl011 *pl011)
{
	if (!pl011)
		return;
	if (pl011->data)
		vm_unmap(pl011->data, PAGE_SIZE);
	free(pl011);
}

static int pl011_init(uintptr_t addr, size_t int_id)
{
	struct pl011 *pl011 = NULL;
	int ret;

	pl011 = malloc(sizeof(*pl011), M_ZERO);
	if (!pl011)
	{
		TRACE("pl011: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	ret = arch_register_native_irq(int_id, pl011_interrupt, pl011,
	                               &pl011->irq);
	if (ret)
	{
		TRACE("pl011: failed to register IRQ");
		goto err;
	}
	struct page page;
	pm_init_page(&page, addr / PAGE_SIZE);
	pl011->data = vm_map(&page, PAGE_SIZE, VM_PROT_RW);
	if (!pl011->data)
	{
		TRACE("pl011: failed to map data");
		ret = -ENOMEM;
		goto err;
	}
	for (int i = 0; ; ++i)
	{
		if (i == 64)
		{
			TRACE("pl011: no more ttyS available");
			ret = -EXDEV;
			goto err;
		}
		char name[64];
		snprintf(name, sizeof(name), "ttyS%d", i);
		ret = tty_alloc(name, makedev(4, 64 + i), &g_tty_op, &pl011->tty);
		if (!ret)
			break;
		if (ret != -EEXIST)
		{
			TRACE("pl011: failed to create TTY: %s", strerror(ret));
			goto err;
		}
	}
	pl011->tty->userdata = pl011;
	pl011_write(pl011, REG_UARTIMSC, INT_RX | INT_RT | INT_FE | INT_PE | INT_BE | INT_OE);
	pl011->tty->flags |= TTY_NOCTRL;
	pl011->tty->termios.c_oflag &= ~OPOST;
	pl011->tty->termios.c_oflag |= ONLCR;
	pl011->tty->termios.c_iflag &= ~IGNCR;
	pl011->tty->termios.c_iflag |= INLCR;
	printf_addtty(pl011->tty);
	return 0;

err:
	pl011_free(pl011);
	return ret;
}

#if WITH_FDT
int pl011_init_fdt(struct fdt_node *node)
{
	int ret;
	struct fdt_prop *reg = fdt_get_prop(node, "reg");
	if (!reg)
	{
		TRACE("pl011: no 'reg' property");
		return -EINVAL;
	}
	uintptr_t mmio_base;
	size_t mmio_size;
	ret = fdt_get_base_size_reg(reg, 0, &mmio_base, &mmio_size);
	if (ret)
	{
		TRACE("pl011: invalid reg");
		return ret;
	}
	struct fdt_prop *interrupts = fdt_get_prop(node, "interrupts");
	if (!interrupts)
	{
		TRACE("pl011: no 'interrupts' property");
		return -EINVAL;
	}
	if (interrupts->len != 3 * 4)
	{
		TRACE("pl011: invalid interrupts property length");
		return -EINVAL;
	}
	uint32_t type = be32toh(*(uint32_t*)&interrupts->data[0]);
	uint32_t irq_id = be32toh(*(uint32_t*)&interrupts->data[4]);
	if (type == 0) /* spi */
		irq_id += 32; /* XXX use spi-base from gicv2 ? */
	return pl011_init(mmio_base, irq_id);
}
#endif

#if WITH_ACPI
int pl011_init_acpi(struct acpi_obj *device)
{
	struct acpi_obj *crs = aml_get_child(&device->device.ns, "_CRS"); /* ACAB */
	if (!crs)
	{
		TRACE("pl011: no _CRS found");
		return -EINVAL;
	}
	if (crs->type != ACPI_OBJ_NAME)
	{
		TRACE("pl011: _CRS isn't a name");
		return -EINVAL;
	}
	if (!crs->namedef.data)
	{
		TRACE("pl011: _CRS has no data");
		return -EINVAL;
	}
	uint8_t mmio_info;
	uint32_t mmio_base;
	uint32_t mmio_size;
	if (acpi_resource_get_fixed_memory_range_32(crs->namedef.data,
	                                            &mmio_info,
	                                            &mmio_base,
	                                            &mmio_size))
	{
		TRACE("pl011: _CRS has no memory range 32");
		return -EINVAL;
	}
	uint8_t interrupts_flags;
	uint32_t interrupts[2];
	uint8_t interrupts_count = sizeof(interrupts) / sizeof(*interrupts);
	if (acpi_resource_get_ext_interrupt(crs->namedef.data,
	                                    &interrupts_flags,
	                                    interrupts,
	                                    &interrupts_count))
	{
		TRACE("pl011: _CRS has no interrupt");
		return -EINVAL;
	}
	if (!interrupts_count)
	{
		TRACE("pl011: no interrupts");
		return -EINVAL;
	}
	return pl011_init(mmio_base, interrupts[0]);
}
#endif
