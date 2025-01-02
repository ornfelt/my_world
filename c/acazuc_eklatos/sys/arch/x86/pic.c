#include "arch/x86/asm.h"
#include "arch/x86/x86.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define ICW1_ICW4      0x01 /* ICW4 (not) needed */
#define ICW1_SINGLE    0x02 /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL     0x08 /* Level triggered (edge) mode */
#define ICW1_INIT      0x10 /* Initialization - required! */

#define ICW4_8086       0x01 /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02 /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08 /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM       0x10 /* Special fully nested (not) */

#define PIC_EOI         0x20

void pic_init(uint8_t offset1, uint8_t offset2)
{
	outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
	outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset1);
	outb(PIC2_DATA, offset2);
	io_wait();
	outb(PIC1_DATA, 4);
	outb(PIC2_DATA, 2);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, ~0x4); /* always enable PIC2 cascade IRQ */
	outb(PIC2_DATA, ~0x0);
}

void pic_enable_irq(enum isa_irq_id id)
{
	uint8_t line = id < 8 ? PIC1_DATA : PIC2_DATA;
	outb(line, inb(line) & ~(1 << (id % 8)));
}

void pic_disable_irq(enum isa_irq_id id)
{
	uint8_t line = id < 8 ? PIC1_DATA : PIC2_DATA;
	outb(line, inb(line) | (1 << (id % 8)));
}

void pic_eoi(enum isa_irq_id id)
{
	if (id >= 8)
		outb(PIC2_CMD, PIC_EOI);
	outb(PIC1_CMD, PIC_EOI);
}
