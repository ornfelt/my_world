#ifndef AARCH64_PSCI_H
#define AARCH64_PSCI_H

#include <types.h>

struct fdt_node;

void psci_init(int use_hvc);
#if WITH_FDT
int psci_init_fdt(struct fdt_node *node);
#endif
void psci_add_cpu(uintptr_t mpidr, uint32_t giccid);
void psci_start_smp(void);
int psci_shutdown(void);
int psci_reboot(void);
int psci_suspend(void);

#endif
