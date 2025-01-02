#define ENABLE_TRACE

#include <arch/asm.h>

#include <proc.h>
#include <cpu.h>
#include <mem.h>

#define L1T_MASK 0xFFF00000
#define L2T_MASK 0x000FF000

#define L1T_SHIFT 20
#define L2T_SHIFT 12

#define L1T_ID(addr) ((((uint32_t)(addr)) & L1T_MASK) >> L1T_SHIFT)
#define L2T_ID(addr) ((((uint32_t)(addr)) & L2T_MASK) >> L2T_SHIFT)

#define ADDR_ALIGN(addr, mask) (((uint32_t)(addr) + (mask)) & ~(mask))
#define L2T_ALIGN(addr) ADDR_ALIGN(addr, (1 << L1T_SHIFT) - 1)
#define PGE_ALIGN(addr) ADDR_ALIGN(addr, (1 << L2T_SHIFT) - 1)

#define L1T_FLAG_P (1 << 0) /* present */

#define L2T_FLAG_NX     (1 << 0) /* non-executable (for l2t) */
#define L2T_FLAG_P      (1 << 1) /* present */
#define L2T_FLAG_B      (1 << 2) /* bufferable */
#define L2T_FLAG_C      (1 << 3) /* cacheable */
#define L2T_FLAG_AP(x)  (((uint32_t)(x)) << 4) /* access privilege */
#define L2T_FLAG_TEX(x) (((uint32_t)(x)) << 6) /* type extension */
#define L2T_FLAG_APX    (1 << 9) /* extended access privilege */
#define L2T_FLAG_S      (1 << 10) /* shareable */
#define L2T_FLAG_NG     (1 << 11) /* non-global */

#define L2T_FLAG_MASK 0x00000FFF
#define L1T_FLAG_MASK 0x000003FF

#define L1T_POFF(val)  (L1T_PADDR(val) >> 12)
#define L1T_PADDR(val) ((uint32_t)(val) & ~L1T_FLAG_MASK)

#define L2T_POFF(val)  (L2T_PADDR(val) >> 12)
#define L2T_PADDR(val) ((uint32_t)(val) & ~L2T_FLAG_MASK)

#define L2T_VADDR(id)     ((uint32_t*)0xFFC00000 + (id) * 0x100)
#define L1T_VADDR         ((uint32_t*)0xFFBFC000)
#define L2T_VADDR_L2T(id) ((uint32_t*)0xFFFFF000 + ((id) >> 2))

extern uint8_t _kernel_end;
extern uint32_t kern_l1t_page;

static inline void invalidate(uintptr_t addr)
{
	dsb();
	tlbivmaa(addr);
	dsb();
}

static inline uint32_t mkentry(uint32_t poff, uint32_t flags)
{
	return (poff << L2T_SHIFT) | flags;
}

static void set_l2t(struct vm_space *space, uint32_t addr, uint32_t *l2t_ptr,
                    uintptr_t poff, uint32_t prot)
{
	uint32_t f = poff ? L2T_FLAG_P : 0;
	if (space)
	{
		if (prot & VM_PROT_W)
			f |= L2T_FLAG_AP(3);
		else if (prot & VM_PROT_R)
			f |= L2T_FLAG_AP(2);
		else
			f |= L2T_FLAG_AP(1);
	}
	else
	{
		if (prot & VM_PROT_W)
			f |= L2T_FLAG_AP(1);
		else if (prot & VM_PROT_R)
			f |= L2T_FLAG_APX | L2T_FLAG_AP(1);
		else
			f |= L2T_FLAG_AP(0);
	}
	if (!(prot & VM_PROT_X))
		f |= L2T_FLAG_NX;
	switch (prot & VM_TYPE_MASK)
	{
		case VM_WB:
			f |= L2T_FLAG_B | L2T_FLAG_C;
			break;
		case VM_WC:
			f |= 0; /* XXX */
			break;
		case VM_UC:
			f |= 0;
			break;
		case VM_MMIO:
			f |= L2T_FLAG_B;
			break;
		case VM_WT:
			f |= L2T_FLAG_C;
			break;
	}
	*l2t_ptr = mkentry(poff, f);
	if (!space)
	{
		invalidate(addr);
		return;
	}
	struct thread *thread = curcpu()->thread;
	if (thread && space == thread->proc->vm_space)
		invalidate(addr);
}

static int get_l2t_ptr(struct vm_space *space, uintptr_t addr, int create,
                       uint32_t **l2t_ptr)
{
	uint32_t l1t_id = L1T_ID(addr);
	uint32_t *l1t = space ? space->arch.l1t : L1T_VADDR;
	if (l1t[l1t_id] & L1T_FLAG_P)
	{
		if (!space)
		{
			*l2t_ptr = &L2T_VADDR(l1t_id)[L2T_ID(addr)];
			return 0;
		}
		if (!space->arch.l2t[l1t_id])
			panic("present l2t without map 0x%" PRIx32 "\n", l1t_id);
		*l2t_ptr = &space->arch.l2t[l1t_id][L2T_ID(addr)];
		return 0;
	}
	if (!space)
		panic("unexisting l2t in kernel space\n");
	if (!create)
		return -EINVAL;
	if (space->arch.l2t[l1t_id])
		panic("already present l2t map 0x%" PRIx32 "\n", l1t_id);
	struct page *l2t_page;
	int ret = pm_alloc_page(&l2t_page);
	if (ret)
		return ret;
	uint32_t *ptr = vm_map(l2t_page, PAGE_SIZE, VM_PROT_RW);
	pm_free_page(l2t_page);
	if (!ptr)
	{
		TRACE("failed to map l2t");
		return -ENOMEM;
	}
	memset(ptr, 0, PAGE_SIZE);
	for (size_t i = l1t_id & ~3; i < (l1t_id & ~3) + 4; ++i)
	{
		space->arch.l2t[i] = &ptr[256 * (i & 3)];
		l1t[i] = (l2t_page->offset << L2T_SHIFT)
		       | (1024 * (i & 3))
		       | L1T_FLAG_P;
	}
	*l2t_ptr = &space->arch.l2t[l1t_id][L2T_ID(addr)];
	return 0;
}

void arch_vm_setspace(const struct vm_space *space)
{
	if (space)
		set_ttbr0(space->arch.l1t_paddr);
	else
		set_ttbr0(kern_l1t_page);
	dsb();
	tlbiall();
	dsb();
}

static int alloc_l1t(struct vm_space *space)
{
	struct page *page;
	size_t npage = 7;
	int ret = pm_alloc_pages(&page, npage);
	if (ret)
	{
		TRACE("ttbr0 allocation failed");
		return ret;
	}
	while (page->offset & 3)
	{
		pm_free_page(page);
		page++;
		npage--;
	}
	space->arch.l1t_paddr = pm_page_addr(page);
	space->arch.l1t = vm_map(page, PAGE_SIZE * 4, VM_PROT_RW);
	pm_free_pages(page, npage);
	if (!space->arch.l1t)
		return -ENOMEM;
	return 0;
}

int arch_vm_space_init(struct vm_space *space)
{
	int ret = alloc_l1t(space);
	if (ret)
		return ret;
	memset(space->arch.l1t, 0, 3072 * sizeof(uint32_t));
	memcpy(&space->arch.l1t[3072], &L1T_VADDR[3072], 1024 * sizeof(uint32_t));
	memset(space->arch.l2t, 0, sizeof(space->arch.l2t));
	return 0;
}

void arch_vm_space_cleanup(struct vm_space *space)
{
	if (!space->arch.l1t)
		return;
	for (size_t i = 0; i < 768; ++i)
	{
		if (!(space->arch.l1t[4 * i] & L1T_FLAG_P))
			continue;
		uint32_t *l2t_ptr = space->arch.l2t[4 * i];
		if (!l2t_ptr)
			panic("present l2t without map 0x%zx\n", i * 4);
		for (size_t j = 0; j < 1024; ++j)
		{
			if (!(l2t_ptr[j] & L2T_FLAG_P))
				continue;
			pm_free_pt(L2T_POFF(l2t_ptr[j]));
		}
		vm_unmap(l2t_ptr, PAGE_SIZE);
	}
	vm_unmap(space->arch.l1t, PAGE_SIZE * 4);
	space->arch.l1t = NULL;
}

static int dup_l2t(uint32_t *l2t_dst, uint32_t *l2t_src, uint32_t l2t_id)
{
	uint32_t src_poff = L2T_POFF(l2t_src[l2t_id]);
	struct page *page = pm_get_page(src_poff);
	if (!page)
	{
		/* XXX update refcount somehow */
		l2t_dst[l2t_id] = mkentry(src_poff,
		                          l2t_src[l2t_id] & L2T_FLAG_MASK);
		return 0;
	}
	int ret = pm_alloc_page(&page);
	if (ret)
		return ret;
	l2t_dst[l2t_id] = mkentry(page->offset, l2t_src[l2t_id] & L2T_FLAG_MASK);
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
		if (!(src->arch.l1t[i * 4] & L1T_FLAG_P))
		{
			for (size_t j = 0; j < 4; ++j)
			{
				dst->arch.l1t[i * 4 + j] = src->arch.l1t[i * 4 + j];
				dst->arch.l2t[i * 4 + j] = NULL;
			}
			continue;
		}
		if (!src->arch.l2t[i * 4])
			panic("present table without map 0x%zx\n", i);
		struct page *l2t_page;
		ret = pm_alloc_page(&l2t_page);
		if (ret)
			return ret;
		uint32_t *l2t_dst = vm_map(l2t_page, PAGE_SIZE, VM_PROT_W);
		pm_free_page(l2t_page);
		if (!l2t_dst)
		{
			TRACE("failed to vmap dst l2t");
			return ret;
		}
		uint32_t *l2t_src = src->arch.l2t[i * 4];
		if (!l2t_src)
		{
			TRACE("failed to vmap src l2t");
			vm_unmap(l2t_dst, PAGE_SIZE);
			return ret;
		}
		for (size_t j = 0; j < 1024; ++j)
		{
			if (!(l2t_src[j] & L2T_FLAG_P))
			{
				l2t_dst[j] = l2t_src[j];
				continue;
			}
			ret = dup_l2t(l2t_dst, l2t_src, j);
			if (ret)
			{
				vm_unmap(l2t_dst, PAGE_SIZE);
				return ret;
			}
		}
		for (size_t j = 0; j < 4; ++j)
		{
			dst->arch.l2t[i * 4 + j] = &l2t_dst[256 * j];
			dst->arch.l1t[i * 4 + j] = (l2t_page->offset << L2T_SHIFT)
			                         | (1024 * j)
			                         | (src->arch.l1t[i * 4 + j] & L1T_FLAG_MASK);
		}
	}
	return 0;
}

static int map_page(struct vm_space *space, uintptr_t addr, uintptr_t poff,
                    uint32_t prot)
{
	uint32_t *l2t_ptr;
	int ret = get_l2t_ptr(space, addr, 1, &l2t_ptr);
	if (ret)
	{
		TRACE("failed to get vmap ptr");
		return ret;
	}
	if (*l2t_ptr & L2T_FLAG_P)
	{
		TRACE("vmap already created page %p: 0x%08" PRIx32,
		      (void*)addr, *l2t_ptr);
		return -EINVAL;
	}
	set_l2t(space, addr, l2t_ptr, poff, prot);
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
	uint32_t *l2t_ptr;
	if (get_l2t_ptr(space, addr, 0, &l2t_ptr))
		return 0;
	if (*l2t_ptr & L2T_FLAG_P)
		pm_free_pt(L2T_POFF(*l2t_ptr));
	*l2t_ptr = mkentry(0, 0);
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
	uint32_t *l2t_ptr;
	int ret = get_l2t_ptr(space, addr, 0, &l2t_ptr);
	if (ret)
		return 0;
	set_l2t(space, addr, l2t_ptr, L2T_POFF(*l2t_ptr), prot);
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
	uint32_t *l2t_ptr;
	int ret = get_l2t_ptr(space, addr, 1, &l2t_ptr);
	if (ret)
	{
		TRACE("failed to get l2t ptr");
		return ret;
	}
	uint32_t poff;
	if (*l2t_ptr & L2T_FLAG_P)
	{
		if (prot & VM_PROT_X)
		{
			if (*l2t_ptr & L2T_FLAG_NX)
				return -EFAULT;
		}
		else if (prot & VM_PROT_W)
		{
			if (((*l2t_ptr >> 4) & 0x3) < 3)
				return -EFAULT;
		}
		else
		{
			if (((*l2t_ptr >> 4) & 0x3) < 2)
				return -EFAULT;
		}
		poff = L2T_POFF(*l2t_ptr);
	}
	else
	{
		struct vm_zone *zone;
		struct page *page;
		ret = vm_fault_page(space, addr, &page, &zone);
		if (ret)
			return ret;
		poff = page->offset;
		set_l2t(space, addr, l2t_ptr, poff, zone->prot);
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
	ret = get_l2t_ptr(NULL, (uintptr_t)zone->ptr, 1, &zone->l2t_ptr);
	if (ret)
		panic("failed to get zero page tbl ptr\n");
}

void *arch_set_copy_zone(struct arch_copy_zone *zone, uintptr_t poff)
{
	*zone->l2t_ptr = mkentry(poff, L2T_FLAG_AP(1) | L2T_FLAG_C | L2T_FLAG_P);
	invalidate((uintptr_t)zone->ptr);
	return zone->ptr;
}

static void set_l1t_ptr(uint32_t *l1t_ptr, size_t poff)
{
	*l1t_ptr = mkentry(poff, L1T_FLAG_P);
	dsb();
}

static void set_l2t_ptr(uint32_t *l2t_ptr, size_t poff)
{
	*l2t_ptr = mkentry(poff, L2T_FLAG_AP(1) | L2T_FLAG_C | L2T_FLAG_P);
	dsb();
}

static void init_l2t_group(uint32_t *l1t_ptr, uint32_t l1t_id, size_t poff)
{
	set_l1t_ptr(l1t_ptr, poff);
	set_l2t_ptr(L2T_VADDR_L2T(l1t_id), poff);
	memset(L2T_VADDR(l1t_id), 0, PAGE_SIZE);
}

size_t arch_pm_init(void *addr, uintptr_t base, size_t count, size_t pool_pages)
{
	(void)count;
	size_t used = pool_pages;
	for (size_t i = 0; i < pool_pages; ++i)
	{
		uint32_t l1t_id = L1T_ID(addr);
		uint32_t l2t_id = L2T_ID(addr);
		uint32_t *l1t_ptr = &L1T_VADDR[l1t_id];
		if (!*l1t_ptr)
		{
			uint32_t l1t_base_id = l1t_id & ~3;
			uint32_t *l1t_base_ptr = &L1T_VADDR[l1t_base_id];
			if (!*l1t_base_ptr)
			{
				init_l2t_group(l1t_base_ptr, l1t_base_id,
				               base + used);
				used++;
			}
			*l1t_ptr = *l1t_base_ptr + 1024 * (l1t_id & 3);
		}
		else
		{
			uint32_t *l2t_l2t = L2T_VADDR_L2T(l1t_id);
			if (!*l2t_l2t)
				set_l2t_ptr(l2t_l2t, L1T_POFF(*l1t_ptr));
		}
		uint32_t *l2t_ptr = L2T_VADDR(l1t_id);
		if (l2t_ptr[l2t_id])
			panic("allocating already-allocated table for %p: 0x%08" PRIx32 "\n", addr, l2t_ptr[l2t_id]);
		set_l2t_ptr(&l2t_ptr[l2t_id], base + i);
		addr = (uint8_t*)addr + PAGE_SIZE;
	}
	return used;
}

static void init_heap_pages_tables(void)
{
	for (uint32_t addr = 0xC0000000; addr < 0xFFF00000; addr += 0x100000)
	{
		uint32_t l1t_id = L1T_ID(addr);
		uint32_t *l1t_ptr = &L1T_VADDR[l1t_id];
		if (*l1t_ptr)
		{
			set_l2t_ptr(L2T_VADDR_L2T(l1t_id & ~3), L1T_POFF(*l1t_ptr));
			continue;
		}
		uint32_t l1t_base_id = l1t_id & ~3;
		uint32_t *l1t_base_ptr = &L1T_VADDR[l1t_base_id];
		if (!*l1t_base_ptr)
		{
			struct page *page;
			int ret = pm_alloc_page(&page);
			if (ret)
				panic("failed to allocate heap page table\n");
			init_l2t_group(l1t_base_ptr, l1t_base_id, page->offset);
		}
		*l1t_ptr = *l1t_base_ptr + 1024 * (l1t_id & 3);
	}
}

void arch_paging_init(void)
{
	pm_init(PGE_ALIGN((uint32_t)&_kernel_end - 0x80000000),
	        (uintptr_t)&_kernel_end, VADDR_HEAP_END);
	init_heap_pages_tables();
	/* XXX remove identity pages */
}
