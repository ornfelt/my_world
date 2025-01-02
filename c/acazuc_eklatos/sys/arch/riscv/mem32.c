#define ENABLE_TRACE

#include <arch/asm.h>
#include <arch/csr.h>

#include <proc.h>
#include <cpu.h>
#include <mem.h>

#define DIR_MASK 0xFFC00000
#define TBL_MASK 0x003FF000
#define PGE_MASK 0x00000FFF

#define DIR_SHIFT 22
#define TBL_SHIFT 12
#define PGE_SHIFT 0

#define DIR_ID(addr) ((((uint32_t)(addr)) & DIR_MASK) >> DIR_SHIFT)
#define TBL_ID(addr) ((((uint32_t)(addr)) & TBL_MASK) >> TBL_SHIFT)
#define PGE_ID(addr) ((((uint32_t)(addr)) & PGE_MASK) >> PGE_SHIFT)

#define ADDR_ALIGN(addr, mask) (((uint32_t)(addr) + (mask)) & ~(mask))
#define TBL_ALIGN(addr) ADDR_ALIGN(addr, (1 << DIR_SHIFT) - 1)
#define PGE_ALIGN(addr) ADDR_ALIGN(addr, (1 << TBL_SHIFT) - 1)

#define PTE_FLAG_D    (1 << 7) /* dirty */
#define PTE_FLAG_A    (1 << 6) /* accessed */
#define PTE_FLAG_G    (1 << 5) /* global */
#define PTE_FLAG_U    (1 << 4) /* user */
#define PTE_FLAG_X    (1 << 3) /* executable */
#define PTE_FLAG_W    (1 << 2) /* writable */
#define PTE_FLAG_R    (1 << 1) /* readable */
#define PTE_FLAG_V    (1 << 0) /* valid */
#define PTE_FLAG_MASK (0x000003FF)

#define PTE_POFF(val)  (PTE_PADDR(val) >> 12)
#define PTE_PADDR(val) (((uint32_t)(val) & ~PTE_FLAG_MASK) << 2)

#define TBL_VADDR(id)     ((uint32_t*)0xFFC00000 + (id) * 0x400)
#define DIR_VADDR         ((uint32_t*)0xFFBFF000)
#define TBL_VADDR_TBL(id) ((uint32_t*)0xFFFFF000 + (id))

extern uint8_t _kernel_end;
extern uint32_t kern_satp_page;

static inline void invalidate(uintptr_t addr)
{
	sfence_vma(addr, 0);
}

static inline uint32_t mkentry(uint32_t poff, uint32_t flags)
{
	return (poff << 10) | flags;
}

static void set_tbl(struct vm_space *space, uint32_t addr, uint32_t *tbl_ptr,
                    uintptr_t poff, uint32_t prot)
{
	uint32_t f = poff ? PTE_FLAG_V : 0;
	if (space)
		f |= PTE_FLAG_U;
	if (prot & VM_PROT_W)
		f |= PTE_FLAG_R | PTE_FLAG_W;
	else if (prot & VM_PROT_R)
		f |= PTE_FLAG_R;
	if (prot & VM_PROT_X)
		f |= PTE_FLAG_X;
	*tbl_ptr = mkentry(poff, f);
	if (!space)
	{
		invalidate(addr);
		return;
	}
	struct thread *thread = curcpu()->thread;
	if (thread && space == thread->proc->vm_space)
		invalidate(addr);
}

static int get_tbl_ptr(struct vm_space *space, uintptr_t addr, int create,
                       uint32_t **tbl_ptr)
{
	uint32_t dir_id = DIR_ID(addr);
	uint32_t *dir = space ? &space->arch.dir[dir_id] : &DIR_VADDR[dir_id];
	if (*dir & PTE_FLAG_V)
	{
		if (!space)
		{
			*tbl_ptr = &TBL_VADDR(dir_id)[TBL_ID(addr)];
			return 0;
		}
		if (!space->arch.tbl[dir_id])
			panic("present table without map 0x%" PRIx32 "\n", dir_id);
		*tbl_ptr = &space->arch.tbl[dir_id][TBL_ID(addr)];
		return 0;
	}
	if (!space)
		panic("unexisting tbl in kernel space\n");
	if (!create)
		return -EINVAL;
	struct page *tbl_page;
	int ret = pm_alloc_page(&tbl_page);
	if (ret)
		return ret;
	if (space->arch.tbl[dir_id])
		panic("already present table map 0x%" PRIx32 "\n", dir_id);
	space->arch.tbl[dir_id] = vm_map(tbl_page, PAGE_SIZE, VM_PROT_RW);
	pm_free_page(tbl_page);
	if (!space->arch.tbl[dir_id])
	{
		TRACE("failed to map page table");
		return -ENOMEM;
	}
	*tbl_ptr = space->arch.tbl[dir_id];
	*dir = mkentry(tbl_page->offset, PTE_FLAG_V);
	memset(*tbl_ptr, 0, PAGE_SIZE);
	*tbl_ptr += TBL_ID(addr);
	return 0;
}

void arch_vm_setspace(const struct vm_space *space)
{
	if (space)
		csrw(CSR_SATP, (1UL << 31) | (pm_page_addr(space->arch.dir_page) >> 12));
	else
		csrw(CSR_SATP, (1UL << 31) | (kern_satp_page >> 12));
	sfence_vma(0, 0);
}

int arch_vm_space_init(struct vm_space *space)
{
	int ret = pm_alloc_page(&space->arch.dir_page);
	if (ret)
		return ret;
	space->arch.dir = vm_map(space->arch.dir_page, PAGE_SIZE, VM_PROT_RW);
	if (!space->arch.dir)
	{
		pm_free_page(space->arch.dir_page);
		space->arch.dir_page = NULL;
		return -ENOMEM;
	}
	memset(space->arch.dir, 0, 768 * sizeof(uint32_t));
	memcpy(&space->arch.dir[768], &DIR_VADDR[768], 256 * sizeof(uint32_t));
	memset(space->arch.tbl, 0, sizeof(space->arch.tbl));
	return 0;
}

void arch_vm_space_cleanup(struct vm_space *space)
{
	if (!space->arch.dir)
		return;
	for (size_t i = 0; i < 768; ++i)
	{
		if (!(space->arch.dir[i] & PTE_FLAG_V))
			continue;
		uint32_t *tbl_ptr = space->arch.tbl[i];
		if (!tbl_ptr)
			panic("present tbl without map 0x%zx\n", i);
		for (size_t j = 0; j < 1024; ++j)
		{
			if (!(tbl_ptr[j] & PTE_FLAG_V))
				continue;
			pm_free_pt(PTE_POFF(tbl_ptr[j]));
		}
		vm_unmap(tbl_ptr, PAGE_SIZE);
	}
	vm_unmap(space->arch.dir, PAGE_SIZE);
	space->arch.dir = NULL;
	pm_free_page(space->arch.dir_page);
	space->arch.dir_page = NULL;
}

static int dup_table(uint32_t *tbl_dst, uint32_t *tbl_src, uint32_t tbl_id)
{
	uint32_t src_poff = PTE_POFF(tbl_src[tbl_id]);
	struct page *page = pm_get_page(src_poff);
	if (!page)
	{
		/* XXX update refcount somehow */
		tbl_dst[tbl_id] = mkentry(src_poff,
		                          tbl_src[tbl_id] & PTE_FLAG_MASK);
		return 0;
	}
	int ret = pm_alloc_page(&page);
	if (ret)
		return ret;
	tbl_dst[tbl_id] = mkentry(page->offset, tbl_src[tbl_id] & PTE_FLAG_MASK);
	struct arch_copy_zone *src_zone = &curcpu()->copy_src_page;
	struct arch_copy_zone *dst_zone = &curcpu()->copy_dst_page;
	void *src = arch_set_copy_zone(src_zone, src_poff);
	void *dst = arch_set_copy_zone(dst_zone, page->offset);
	memcpy(__builtin_assume_aligned(dst, PAGE_SIZE),
	       __builtin_assume_aligned(src, PAGE_SIZE),
	       PAGE_SIZE);
	return 0;
}

int arch_vm_space_copy(struct vm_space *dst, struct vm_space *src)
{
	int ret;
	for (size_t i = 0; i < 768; ++i)
	{
		if (!(src->arch.dir[i] & PTE_FLAG_V))
		{
			dst->arch.dir[i] = src->arch.dir[i];
			dst->arch.tbl[i] = NULL;
			continue;
		}
		if (!src->arch.tbl[i])
			panic("present table without map 0x%zx\n", i);
		struct page *tbl_page;
		ret = pm_alloc_page(&tbl_page);
		if (ret)
			return ret;
		dst->arch.dir[i] = mkentry(tbl_page->offset,
		                           src->arch.dir[i] & PTE_FLAG_MASK);
		uint32_t *tbl_dst = vm_map(tbl_page, PAGE_SIZE, VM_PROT_W);
		pm_free_page(tbl_page);
		if (!tbl_dst)
		{
			TRACE("failed to vmap dst tbl");
			return ret;
		}
		uint32_t *tbl_src = src->arch.tbl[i];
		if (!tbl_src)
		{
			TRACE("failed to vmap src tbl");
			vm_unmap(tbl_dst, PAGE_SIZE);
			return ret;
		}
		for (size_t j = 0; j < 1024; ++j)
		{
			if (!(tbl_src[j] & PTE_FLAG_V))
			{
				tbl_dst[j] = tbl_src[j];
				continue;
			}
			ret = dup_table(tbl_dst, tbl_src, j);
			if (ret)
			{
				vm_unmap(tbl_dst, PAGE_SIZE);
				return ret;
			}
		}
		dst->arch.tbl[i] = tbl_dst;
	}
	return 0;
}

static int map_page(struct vm_space *space, uintptr_t addr, uintptr_t poff,
                    uint32_t prot)
{
	uint32_t *tbl_ptr;
	int ret = get_tbl_ptr(space, addr, 1, &tbl_ptr);
	if (ret)
		return ret;
	if (*tbl_ptr & PTE_FLAG_V)
	{
		TRACE("vmap already created page %p: 0x%08" PRIx32,
		      (void*)addr, *tbl_ptr);
		return -EINVAL;
	}
	set_tbl(space, addr, tbl_ptr, poff, prot);
	if (poff)
	{
		struct page *page = pm_get_page(poff);
		if (page)
			pm_ref_page(page);
	}
	return 0;
}

int arch_vm_map(struct vm_space *space, uintptr_t addr, uintptr_t poff,
                size_t size, uint32_t prot)
{
	/* XXX optimize pte tree */
	for (size_t i = 0; i < size; i += PAGE_SIZE)
	{
		int ret = map_page(space, addr + i, poff + i / PAGE_SIZE, prot);
		if (ret)
		{
			arch_vm_unmap(NULL, addr, i);
			TRACE("failed to map page");
			return ret;
		}
	}
	return 0;
}

static int unmap_page(struct vm_space *space, uintptr_t addr)
{
	uint32_t *tbl_ptr;
	if (get_tbl_ptr(space, addr, 0, &tbl_ptr))
		return 0;
	if (*tbl_ptr & PTE_FLAG_V)
		pm_free_pt(PTE_POFF(*tbl_ptr));
	*tbl_ptr = mkentry(0, 0);
	invalidate(addr);
	return 0;
}

int arch_vm_unmap(struct vm_space *space, uintptr_t addr, size_t size)
{
	/* XXX optimize pte tree */
	for (size_t i = 0; i < size; i += PAGE_SIZE)
		unmap_page(space, addr + i);
	return 0;
}

static int protect_page(struct vm_space *space, uintptr_t addr, uint32_t prot)
{
	uint32_t *tbl_ptr;
	int ret = get_tbl_ptr(space, addr, 0, &tbl_ptr);
	if (ret)
		return ret;
	set_tbl(space, addr, tbl_ptr, PTE_POFF(*tbl_ptr), prot);
	return 0;
}

int arch_vm_protect(struct vm_space *space, uintptr_t addr, size_t size,
                    uint32_t prot)
{
	/* XXX optimize pte tree */
	for (size_t i = 0; i < size; i += PAGE_SIZE)
		protect_page(space, addr + i, prot);
	return 0;
}

int arch_vm_populate_page(struct vm_space *space, uintptr_t addr,
                          uint32_t prot, uintptr_t *poffp)
{
	uint32_t *tbl_ptr;
	int ret = get_tbl_ptr(space, addr, 1, &tbl_ptr);
	if (ret)
		return ret;
	uint32_t poff;
	if (*tbl_ptr & PTE_FLAG_V)
	{
		if (prot & VM_PROT_X)
		{
			if (!(*tbl_ptr & PTE_FLAG_X))
				return -EFAULT;
		}
		else if (prot & VM_PROT_W)
		{
			if (!(*tbl_ptr & PTE_FLAG_W))
				return -EFAULT;
		}
		else
		{
			if (!(*tbl_ptr & PTE_FLAG_R))
				return -EFAULT;
		}
		poff = PTE_POFF(*tbl_ptr);
	}
	else
	{
		struct vm_zone *zone;
		struct page *page;
		ret = vm_fault_page(space, addr, &page, &zone);
		if (ret)
			return ret;
		poff = page->offset;
		set_tbl(space, addr, tbl_ptr, poff, zone->prot);
	}
	if (poffp)
		*poffp = poff;
	return 0;
}

void arch_init_copy_zone(struct arch_copy_zone *zone)
{
	mutex_spinlock(&g_vm_mutex);
	int ret = vm_region_alloc(&g_vm_heap, 0, PAGE_SIZE, 0,
	                          (uintptr_t*)&zone->ptr);
	if (ret)
		panic("failed to alloc zero page\n");
	mutex_unlock(&g_vm_mutex);
	ret = get_tbl_ptr(NULL, (uintptr_t)zone->ptr, 1, &zone->tbl_ptr);
	if (ret)
		panic("failed to get zero page tbl ptr\n");
}

void *arch_set_copy_zone(struct arch_copy_zone *zone, uintptr_t poff)
{
	*zone->tbl_ptr = mkentry(poff, PTE_FLAG_W | PTE_FLAG_R | PTE_FLAG_V);
	invalidate((uintptr_t)zone->ptr);
	return zone->ptr;
}

size_t arch_pm_init(void *addr, uintptr_t base, size_t count, size_t pool_pages)
{
	(void)count;
	size_t used = pool_pages;
	for (size_t i = 0; i < pool_pages; ++i)
	{
		uint32_t dir_id = DIR_ID(addr);
		uint32_t *dir_ptr = &DIR_VADDR[dir_id];
		if (!*dir_ptr)
		{
			uint32_t poff = base + used;
			used++;
			*dir_ptr = mkentry(poff, PTE_FLAG_V);
			uint32_t *tbl_tbl = TBL_VADDR_TBL(dir_id);
			*tbl_tbl = mkentry(poff, PTE_FLAG_W | PTE_FLAG_R | PTE_FLAG_V);
			memset(TBL_VADDR(dir_id), 0, PAGE_SIZE);
		}
		else
		{
			uint32_t *tbl_tbl = TBL_VADDR_TBL(dir_id);
			if (!*tbl_tbl)
				*tbl_tbl = mkentry(PTE_POFF(*dir_ptr), PTE_FLAG_W | PTE_FLAG_R | PTE_FLAG_V);
		}
		uint32_t *tbl_ptr = TBL_VADDR(dir_id);
		uint32_t tbl_id = TBL_ID(addr);
		if (tbl_ptr[tbl_id])
			panic("allocating already-allocated table for %p: 0x%08" PRIx32 "\n", addr, tbl_ptr[tbl_id]);
		tbl_ptr[tbl_id] = mkentry(base + i, PTE_FLAG_W | PTE_FLAG_R | PTE_FLAG_V);
		addr = (uint8_t*)addr + PAGE_SIZE;
	}
	return used;
}

static void init_heap_pages_tables(void)
{
	for (uint32_t addr = 0xC0000000; addr < 0xFFC00000; addr += 0x400000)
	{
		uint32_t dir_id = DIR_ID(addr);
		uint32_t *dir_ptr = &DIR_VADDR[dir_id];
		int created = 0;
		if (!*dir_ptr)
		{
			struct page *page;
			int ret = pm_alloc_page(&page);
			if (ret)
				panic("failed to allocate heap page table\n");
			*dir_ptr = mkentry(page->offset, PTE_FLAG_V);
			created = 1;
		}
		uint32_t *tbl_tbl = TBL_VADDR_TBL(dir_id);
		*tbl_tbl = mkentry(PTE_POFF(*dir_ptr),
		                   PTE_FLAG_W | PTE_FLAG_R | PTE_FLAG_V);
		if (created)
			memset(TBL_VADDR(dir_id), 0, PAGE_SIZE);
	}
}

void arch_paging_init(void)
{
	pm_init(PGE_ALIGN((uint32_t)&_kernel_end - 0x40000000),
	        (uintptr_t)&_kernel_end, VADDR_HEAP_END);
	init_heap_pages_tables();
	/* XXX remove identity pages */
}
