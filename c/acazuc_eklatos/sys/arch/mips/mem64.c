#define ENABLE_TRACE

#include <arch/asm.h>

#include <errno.h>
#include <proc.h>
#include <mem.h>

void arch_vm_setspace(const struct vm_space *space)
{
	/* XXX */
}

int arch_vm_space_init(struct vm_space *space)
{
	/* XXX */
}

void arch_vm_space_cleanup(struct vm_space *space)
{
	/* XXX */
}

int arch_vm_space_copy(struct vm_space *dst, struct vm_space *src)
{
	/* XXX */
}

int arch_vm_map(struct vm_space *space, uintptr_t addr, uintptr_t poff,
                size_t size, uint32_t prot)
{
	/* XXX */
}

int arch_vm_unmap(struct vm_space *space, uintptr_t addr, size_t size)
{
	/* XXX */
}

int arch_vm_protect(struct vm_space *space, uintptr_t addr, size_t size,
                    uint32_t prot)
{
	/* XXX */
}

int arch_vm_populate_page(struct vm_space *space, uintptr_t addr,
                          uint32_t prot, uintptr_t *poffp)
{
	/* XXX */
}

void arch_init_copy_zone(struct arch_copy_zone *zone)
{
	/* XXX */
}

void arch_set_copy_zone(struct arch_copy_zone *zone, uintptr_t poff)
{
	/* XXX */
}

size_t arch_pm_init(void *addr, uintptr_t base, size_t count, size_t pool_pages)
{
	/* XXX */
}

void arch_paging_init(void)
{
	/* XXX */
}
