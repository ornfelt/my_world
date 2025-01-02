#ifndef EFI_H
#define EFI_H

#include <types.h>

int efi_init(void);
uintptr_t efi_get_fdt(void);

#endif
