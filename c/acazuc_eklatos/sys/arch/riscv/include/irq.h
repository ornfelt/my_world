#ifndef RISCV_IRQ_H
#define RISCV_IRQ_H

#define IRQ_COUNT 1024

struct arch_irq_native_handle
{
	size_t line;
};

struct irq_ctx
{
	struct trapframe *trapframe;
};

#endif
