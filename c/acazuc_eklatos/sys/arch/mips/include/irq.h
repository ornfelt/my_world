#ifndef MIPS_IRQ_H
#define MIPS_IRQ_H

#include <types.h>

#define IRQ_COUNT 256

struct arch_irq_native_handle
{
	size_t line;
};

struct irq_ctx
{
	struct trapframe *trapframe;
};

#endif
