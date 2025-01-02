#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <multiboot2.h>

int multiboot_register_sysfs(void);
void multiboot_iterate_memory(uintptr_t min, uintptr_t max,
                              void (*cb)(uintptr_t addr, size_t size, void *userdata),
                              void *userdata);

const struct multiboot_tag *multiboot_find_tag(uint16_t type);

#endif
