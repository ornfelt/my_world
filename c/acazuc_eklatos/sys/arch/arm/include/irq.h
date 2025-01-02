#ifndef ARM_IRQ_H
#define ARM_IRQ_H

#include <types.h>

#define IRQ_COUNT 1024

#define IRQ_ID_IPI 0x01

struct arch_irq_native_handle
{
	size_t line;
};

struct irq_ctx
{
	struct trapframe *trapframe;
};

#endif
