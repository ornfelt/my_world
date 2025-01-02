#ifndef ARCH_MEM_H
#define ARCH_MEM_H

struct page;

struct arch_vm_space
{
	struct page *dir_page;
};

#endif
