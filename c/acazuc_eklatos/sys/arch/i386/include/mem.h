#ifndef ARCH_MEM_H
#define ARCH_MEM_H

#include <types.h>

struct arch_vm_space
{
	struct page *dir_page;
	uint32_t *dir;
	uint32_t *tbl[1024];
};

#endif
