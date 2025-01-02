#ifndef IRQ_H
#define IRQ_H

#include <arch/irq.h>

#include <queue.h>
#include <types.h>

enum irq_type
{
	IRQ_USR,
	IRQ_NATIVE,
	IRQ_MSI,
	IRQ_MSIX,
};

struct irq_handle;
struct irq_ctx;

typedef void (*irq_fn_t)(void *userdata);

struct irq_handle
{
	enum irq_type type;
	size_t id;
	size_t cpuid;
	union
	{
		struct arch_irq_native_handle native;
		struct
		{
			uint16_t vector;
		} msix;
	};
	irq_fn_t fn;
	void *userdata;
	TAILQ_ENTRY(irq_handle) chain;
};

int arch_get_msi_addr(size_t cpuid, uint64_t *addr);
int arch_get_msi_data(size_t irq, uint32_t *data);
int arch_get_msi_irq(size_t *cpuid, size_t *irq);
/* XXX merge with arch_get_msi_irq? */
void arch_register_msi_irq(struct irq_handle *handle);

int arch_register_native_irq(size_t irq, irq_fn_t fn, void *userdata,
                             struct irq_handle *handle);
void arch_disable_native_irq(struct irq_handle *handle);
void arch_trap_handle(size_t id, struct irq_ctx *ctx);
void arch_set_trap_stack(void *ptr);
__attribute__ ((noreturn))
void arch_trap_return(void);

void register_irq(struct irq_handle *handle, enum irq_type type,
                  size_t id, size_t cpuid, irq_fn_t fn, void *userdata);
void unregister_irq(struct irq_handle *handle);
void irq_execute(size_t id);
int irq_register_sysfs(void);
__attribute__ ((noreturn))
void trap_handle(size_t id, struct irq_ctx *ctx);
__attribute__ ((noreturn))
void trap_return(void);

#endif
