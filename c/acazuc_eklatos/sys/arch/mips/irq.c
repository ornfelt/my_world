#include <irq.h>

void arch_trap_handle(size_t cause, struct irq_ctx *ctx)
{
	/* XXX */
}

int arch_register_native_irq(size_t irq, irq_fn_t fn, void *userdata,
                             struct irq_handle *handle)
{
	/* XXX */
}

void arch_disable_native_irq(struct irq_handle *handle)
{
	/* XXX */
}
