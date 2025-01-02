#ifndef X86_IRQ_H
#define X86_IRQ_H

#include <queue.h>
#include <types.h>

#define IRQ_COUNT 256

#define IRQ_ID_SYSCALL  0x80
#define IRQ_ID_IPI      0xFE
#define IRQ_ID_SPURIOUS 0xFF

struct irq_ctx
{
	struct trapframe *trapframe;
	uintptr_t err;
};

struct arch_irq_native_handle
{
	uint8_t ioapic;
	uint8_t line;
};

#endif
