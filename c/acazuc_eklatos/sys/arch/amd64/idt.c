#include "arch/x86/x86.h"
#include "arch/x86/asm.h"

extern void *g_isr_table[];

struct idt_entry
{
	uint16_t isr_low;    /* the lower 16 bits of the ISR's address */
	uint16_t kernel_cs;  /* the GDT segment selector that the CPU will load into CS before calling the ISR */
	uint8_t  ist;        /* 3-bit offset into IST */
	uint8_t  attributes; /* type and attributes; see the IDT page */
	uint16_t isr_high;   /* the higher 16 bits of the ISR's address */
	uint32_t isr_quad;   /* the highest 32 bits of the ISR's address */
	uint32_t reserved;   /* set to zero */
} __attribute__((packed));

struct idtr
{
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

static struct idtr idtr;
static struct idt_entry idt[256];

static void set_descriptor(struct idt_entry *descriptor, void *isr,
                           uint8_t flags, uint8_t ist)
{
	descriptor->isr_low    = (uint64_t)isr & 0xFFFF;
	descriptor->kernel_cs  = 0x08;
	descriptor->ist        = ist;
	descriptor->attributes = flags;
	descriptor->isr_high   = (uint64_t)isr >> 16;
	descriptor->isr_quad   = (uint64_t)isr >> 32;
	descriptor->reserved   = 0;
}

void idt_init(void)
{
	for (int i = 0; i < 256; ++i)
		set_descriptor(&idt[i], g_isr_table[i], (i == 0x3 || i == 0x80) ? 0xEE : 0x8E, 0); /* XXX ist */
	idtr.base = (uint64_t)&idt[0];
	idtr.limit = (uint16_t)sizeof(idt) - 1;
}

void idt_load(void)
{
	lidt(&idtr);
}
