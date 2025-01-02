#ifndef ARCH_MEM_H
#define ARCH_MEM_H

#include <types.h>

struct arch_vm_space
{
#if __riscv_xlen == 64
	struct page *dir_page;
#else
	struct page *dir_page;
	uint32_t *dir;
	uint32_t *tbl[1024];
#endif
};

#endif
