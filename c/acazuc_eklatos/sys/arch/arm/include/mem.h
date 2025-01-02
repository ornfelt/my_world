#ifndef ARCH_MEM_H
#define ARCH_MEM_H

#include <types.h>

struct arch_vm_space
{
	uint32_t *l1t;
	uint32_t *l2t[4096];
	uint32_t l1t_paddr;
};

#endif
