#ifndef X86_APIC_H
#define X86_APIC_H

#include "arch/x86/x86.h"

#include <types.h>

enum lapic_ipi_dst_type
{
	LAPIC_IPI_DST,
	LAPIC_IPI_SELF,
	LAPIC_IPI_BROADCAST,
	LAPIC_IPI_OTHER,
};

extern uint32_t g_lapics[256];
extern size_t g_lapics_count;

void ioapic_init(uint8_t id, uint32_t addr, uint32_t gsib);
void lapic_init(void);
void lapic_init_smp(void);
void ioapic_enable_irq(uint8_t ioapic, uint8_t irq, int active_low,
                       int level_trigger);
void ioapic_disable_irq(uint8_t ioapic, uint8_t irq);
uint8_t ioapic_default_id(void);
void lapic_eoi(void);
void lapic_sendipi(enum lapic_ipi_dst_type dst_type, uint8_t dst, uint8_t id);
void lapic_send_init_ipi(uint8_t dst);
void lapic_send_startup_ipi(uint8_t dst, uintptr_t addr);
void lapic_send_init_deassert(uint8_t dst);

#endif
