#ifndef AARCH64_GICV2_H
#define AARCH64_GICV2_H

#include <types.h>

struct fdt_node;
struct cpu;

#if WITH_FDT
int gicv2_init_fdt(struct fdt_node *node);
#endif

#if WITH_ACPI
int gicv2_init_gicc(uintptr_t addr);
int gicv2_init_gicd(uintptr_t addr);
int gicv2_init_gicm(uintptr_t addr);
#endif

void gicv2_enable_gicc(void);
void gicv2_enable_gicd(void);
int gicv2_get_active_interrupt(size_t *irq);
void gicv2_clear_interrupt(size_t id);
void gicv2_enable_interrupt(size_t id);
void gicv2_disable_interrupt(size_t id);
void gicv2_set_edge_trigger(size_t id);
void gicv2_eoi(size_t id);
uint64_t gicv2_get_msi_addr(void);
uint32_t gicv2_get_msi_data(size_t irq);
void gicv2_set_irq_cpu(size_t irq, size_t cpu);
size_t gicv2_get_msi_min_irq(void);
size_t gicv2_get_msi_max_irq(void);
void gicv2_sgi(struct cpu *cpu, uint8_t irq);

#endif
