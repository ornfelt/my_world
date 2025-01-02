#ifndef RISCV_SYSCON_H
#define RISCV_SYSCON_H

struct fdt_node;

int syscon_init_poweroff_fdt(const struct fdt_node *node);
int syscon_init_reboot_fdt(const struct fdt_node *node);
int syscon_poweroff(void);
int syscon_reboot(void);

#endif
