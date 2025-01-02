#ifndef RISCV_PLIC_H
#define RISCV_PLIC_H

#include <types.h>

struct fdt_node;
struct cpu;

int plic_init_fdt(const struct fdt_node *node);
int plic_setup_cpu(struct cpu *cpu);

void plic_enable_interrupt(size_t id);
void plic_disable_interrupt(size_t id);
void plic_enable_cpu_int(struct cpu *cpu, size_t id);
void plic_eoi(struct cpu *cpu, size_t id);
int plic_get_active_interrupt(struct cpu *cpu, size_t *irq);

#endif
