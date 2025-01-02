#define ENABLE_TRACE

#include <arch/asm.h>
#include <arch/csr.h>

#include <errno.h>
#include <proc.h>
#include <mem.h>

#define PTE_MASK 0x1FF
#define ADDR_ALIGN(addr, mask) (((uint64_t)(addr) + (mask)) & ~(mask))

#define DIR_SHIFT(n) (12 + (n) * 9)
#define DIR_MASK(n) ((uint64_t)PTE_MASK << DIR_SHIFT(n))
#define DIR_ID(n, addr) ((((uint64_t)(addr)) & DIR_MASK(n)) >> DIR_SHIFT(n))
#define DIR_ALIGN(n, addr) ADDR_ALIGN(addr, ((uint64_t)1 << DIR_SHIFT(n + 1)) - 1)

#define DIR_FLAG_D    (1 << 7) /* dirty */
#define DIR_FLAG_A    (1 << 6) /* accessed */
#define DIR_FLAG_G    (1 << 5) /* global */
#define DIR_FLAG_U    (1 << 4) /* user */
#define DIR_FLAG_X    (1 << 3) /* executable */
#define DIR_FLAG_W    (1 << 2) /* writable */
#define DIR_FLAG_R    (1 << 1) /* readable */
#define DIR_FLAG_V    (1 << 0) /* valid */
#define DIR_FLAG_MASK (0xFFC00000000003FFULL)

#define DIR_POFF(val)  (DIR_PADDR(val) >> 12)
#define DIR_PADDR(val) (((uint64_t)(val) & ~DIR_FLAG_MASK) << 2)

#define PMAP(addr) ((void*)(VADDR_PMAP_BEGIN + (uint64_t)(addr)))

extern uint8_t _kernel_end;
extern uint8_t _kernel_size;
extern uint64_t kern_satp_page;

static uint64_t *early_vmap_pge = (uint64_t*)0xFFFFFFFFFFE00000;
static uint64_t *early_vmap_tbl = (uint64_t*)0xFFFFFFFFFFFFF000;

static inline void invalidate(uintptr_t addr)
{
	sfence_vma(addr, 0);
}

static inline uint64_t mkentry(uint64_t poff, uint64_t flags)
{
	return (poff << 10) | flags;
}

static void set_dir(struct vm_space *space, uint64_t addr, uint64_t *dir,
                    uintptr_t poff, uint32_t prot)
{
	uint64_t f = poff ? DIR_FLAG_V : 0;
	if (space)
		f |= DIR_FLAG_U;
	if (prot & VM_PROT_W)
		f |= DIR_FLAG_R | DIR_FLAG_W;
	else if (prot & VM_PROT_R)
		f |= DIR_FLAG_R;
	if (prot & VM_PROT_X)
		f |= DIR_FLAG_X;
	*dir = mkentry(poff, f);
	if (!space)
	{
		invalidate(addr);
		return;
	}
	struct thread *thread = curcpu()->thread;
	if (thread && space == thread->proc->vm_space)
		invalidate(addr);
}

static int get_dir(struct vm_space *space, uintptr_t addr, int create,
                   uint64_t **dirp)
{
	uint64_t pte_id = DIR_ID(3, addr);
	uint64_t *pte = space ? PMAP(pm_page_addr(space->arch.dir_page)) : (uint64_t*)(VADDR_CODE_BEGIN + (uint64_t)kern_satp_page - 0x80000000);
	uint8_t shift = DIR_SHIFT(2);
	for (size_t i = 0; i < 3; ++i)
	{
		if (!(pte[pte_id] & DIR_FLAG_V))
		{
			if (!create)
				return -EINVAL;
			struct page *page;
			int ret = pm_alloc_page(&page);
			if (ret)
				return ret;
			pte[pte_id] = mkentry(page->offset, DIR_FLAG_V);
			memset(PMAP(page->offset * PAGE_SIZE), 0, PAGE_SIZE);
		}
		pte = PMAP(DIR_PADDR(pte[pte_id]));
		pte_id = (addr >> shift) & PTE_MASK;
		shift -= 9;
	}
	*dirp = &pte[pte_id];
	return 0;
}

void arch_vm_setspace(const struct vm_space *space)
{
	if (space)
		csrw(CSR_SATP, (9ULL << 60) | (pm_page_addr(space->arch.dir_page) >> 12));
	else
		csrw(CSR_SATP, (9ULL << 60) | (kern_satp_page >> 12));
	sfence_vma(0, 0);
}

int arch_vm_space_init(struct vm_space *space)
{
	int ret = pm_alloc_page(&space->arch.dir_page);
	if (ret)
		return ret;
	uint64_t *dir = PMAP(pm_page_addr(space->arch.dir_page));
	memset(dir, 0, 256 * sizeof(uint64_t));
	memcpy(&dir[256],
	       &((uint64_t*)(VADDR_CODE_BEGIN + (uint64_t)kern_satp_page - 0x80000000))[256],
	       256 * sizeof(uint64_t));
	return 0;
}

static void cleanup_level(uint64_t *pte, size_t min, size_t max, uint8_t level)
{
	for (size_t i = min; i < max; ++i)
	{
		uint64_t entry = pte[i];
		if (!(entry & DIR_FLAG_V))
			continue;
		if (level)
			cleanup_level(PMAP(DIR_PADDR(entry)), 0, 512, level - 1);
		pm_free_pt(DIR_POFF(entry));
	}
}

void arch_vm_space_cleanup(struct vm_space *space)
{
	if (!space->arch.dir_page)
		return;
	cleanup_level(PMAP(pm_page_addr(space->arch.dir_page)), 0, 256, 3);
	pm_free_page(space->arch.dir_page);
	space->arch.dir_page = NULL;
}

static int dup_table(uint64_t *dir0_dst, const uint64_t *dir0_src)
{
	uint64_t src_poff = DIR_POFF(*dir0_src);
	struct page *page = pm_get_page(src_poff);
	if (!page)
	{
		/* XXX update refcount somehow */
		*dir0_dst = mkentry(src_poff, *dir0_src & DIR_FLAG_MASK);
		return 0;
	}
	int ret = pm_alloc_page(&page);
	if (ret)
		return ret;
	*dir0_dst = mkentry(page->offset, *dir0_src & DIR_FLAG_MASK);
	struct arch_copy_zone *src_zone = &curcpu()->copy_src_page;
	struct arch_copy_zone *dst_zone = &curcpu()->copy_dst_page;
	void *src = arch_set_copy_zone(src_zone, src_poff);
	void *dst = arch_set_copy_zone(dst_zone, page->offset);
	memcpy(__builtin_assume_aligned(dst, PAGE_SIZE),
	       __builtin_assume_aligned(src, PAGE_SIZE),
	       PAGE_SIZE);
	return 0;
}

static int copy_level(uint64_t *dst, uint64_t *src, size_t min, size_t max,
                      uint8_t level)
{
	for (size_t i = min; i < max; ++i)
	{
		if (!(src[i] & DIR_FLAG_V))
		{
			dst[i] = src[i];
			continue;
		}
		if (level)
		{
			struct page *pte_page;
			int ret = pm_alloc_page(&pte_page);
			if (ret)
				return ret;
			dst[i] = mkentry(pte_page->offset,
			                 src[i] & DIR_FLAG_MASK);
			uint64_t *nxt_src = PMAP(DIR_PADDR(src[i]));
			uint64_t *nxt_dst = PMAP(DIR_PADDR(dst[i]));
			ret = copy_level(nxt_dst, nxt_src, 0, 512, level - 1);
			if (ret)
				return ret;
		}
		else
		{
			int ret = dup_table(&dst[i], &src[i]);
			if (ret)
				return ret;
		}
	}
	return 0;
}

int arch_vm_space_copy(struct vm_space *dst, struct vm_space *src)
{
	return copy_level(PMAP(pm_page_addr(dst->arch.dir_page)),
	                  PMAP(pm_page_addr(src->arch.dir_page)),
	                  0, 256, 3);
}

static int map_page(struct vm_space *space, uintptr_t addr, uintptr_t poff,
                    uint32_t prot)
{
	uint64_t *dir;
	int ret = get_dir(space, addr, 1, &dir);
	if (ret)
	{
		TRACE("failed to get vmap ptr");
		return ret;
	}
	if (*dir & DIR_FLAG_V)
	{
		TRACE("vmap already created page %p: 0x%016" PRIx64,
		      (void*)addr, *dir);
		return -EINVAL;
	}
	set_dir(space, addr, dir, poff, prot);
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
	uint64_t *dir;
	if (get_dir(space, addr, 0, &dir))
		return 0;
	if (*dir & DIR_FLAG_V)
		pm_free_pt(DIR_POFF(*dir));
	*dir = mkentry(0, 0);
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
	uint64_t *dir;
	int ret = get_dir(space, addr, 0, &dir);
	if (ret)
		return 0;
	set_dir(space, addr, dir, DIR_POFF(*dir), prot);
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
	uint64_t *dir;
	int ret = get_dir(space, addr, 1, &dir);
	if (ret)
	{
		TRACE("failed to get dir ptr");
		return ret;
	}
	uint64_t poff;
	if (*dir & DIR_FLAG_V)
	{
		if (prot & VM_PROT_X)
		{
			if (!(*dir & DIR_FLAG_X))
				return -EFAULT;
		}
		else if (prot & VM_PROT_W)
		{
			if (!(*dir & DIR_FLAG_W))
				return -EFAULT;
		}
		else
		{
			if (!(*dir & DIR_FLAG_R))
				return -EFAULT;
		}
		poff = DIR_POFF(*dir);
	}
	else
	{
		struct vm_zone *zone;
		struct page *page;
		ret = vm_fault_page(space, addr, &page, &zone);
		if (ret)
			return ret;
		poff = page->offset;
		set_dir(space, addr, dir, poff, zone->prot);
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
	ret = get_dir(NULL, (uintptr_t)zone->ptr, 1, &zone->dir0_ptr);
	if (ret)
		panic("failed to get zero page tbl ptr\n");
}

void *arch_set_copy_zone(struct arch_copy_zone *zone, uintptr_t poff)
{
	*zone->dir0_ptr = mkentry(poff, DIR_FLAG_W | DIR_FLAG_R | DIR_FLAG_V);
	invalidate((uintptr_t)zone->ptr);
	return zone->ptr;
}

static void map_early(void *addr, uintptr_t base, size_t count, int large,
                      size_t *used)
{
	static uint64_t prev_pte[3] = {0, 0, 0};
	for (size_t i = 0; i < count;)
	{
		uint8_t *ptr = (uint8_t*)addr + i * PAGE_SIZE;
		uint64_t pte_id = DIR_ID(3, ptr);
		uint64_t *pte = (uint64_t*)kern_satp_page;
		uint8_t shift = DIR_SHIFT(2);
		uintptr_t poff = base + i;
		size_t n;
		if (large)
		{
			if (count - i >= 512 * 512
			 && !((uintptr_t)ptr % (512 * 512 * 4096))
			 && !(poff % (512 * 512)))
				n = 1;
			else if (count - i >= 512
				      && !((uintptr_t)ptr % (512 * 4096))
			      && !(poff % 512))
				n = 2;
			else
				n = 3;
		}
		else
		{
			n = 3;
		}
		for (size_t j = 0; j < n; ++j)
		{
			if (!(pte[pte_id] & DIR_FLAG_V))
			{
				uint64_t pte_poff = base + (*used)++;
				uint64_t pte_val = mkentry(pte_poff,
				                           DIR_FLAG_V);
				prev_pte[j] = pte_val;
				pte[pte_id] = pte_val;
				early_vmap_tbl[j + 1] = mkentry(DIR_POFF(pte_val),
				                                DIR_FLAG_W | DIR_FLAG_R | DIR_FLAG_V);
				pte = &early_vmap_pge[512 * (j + 1)];
				invalidate((uintptr_t)pte);
				memset(pte, 0, PAGE_SIZE);
			}
			else if (pte[pte_id] != prev_pte[j])
			{
				prev_pte[j] = pte[pte_id];
				early_vmap_tbl[j + 1] = mkentry(DIR_POFF(pte[pte_id]),
				                                DIR_FLAG_W | DIR_FLAG_R | DIR_FLAG_V);
				pte = &early_vmap_pge[512 * (j + 1)];
				invalidate((uintptr_t)pte);
			}
			else
			{
				pte = &early_vmap_pge[512 * (j + 1)];
			}
			pte_id = ((uintptr_t)ptr >> shift) & PTE_MASK;
			shift -= 9;
		}
		if (n == 1)
		{
			pte[pte_id] = mkentry(poff, DIR_FLAG_W | DIR_FLAG_R | DIR_FLAG_V);
			i += 512 * 512;
		}
		else if (n == 2)
		{
			pte[pte_id] = mkentry(poff, DIR_FLAG_W | DIR_FLAG_R | DIR_FLAG_V);
			i += 512;
		}
		else
		{
			pte[pte_id] = mkentry(poff, DIR_FLAG_W | DIR_FLAG_R | DIR_FLAG_V);
			i++;
		}
	}
}

size_t arch_pm_init(void *addr, uintptr_t base, size_t count, size_t pool_pages)
{
	size_t used = pool_pages;
	map_early(PMAP(PAGE_SIZE * base), base, count, 1, &used);
	map_early(addr, base, pool_pages, 0, &used);
	return used;
}

void arch_paging_init(void)
{
	pm_init(DIR_ALIGN(0, (uint64_t)&_kernel_end - VADDR_CODE_BEGIN + 0x80000000),
	        VADDR_HEAP_BEGIN, VADDR_HEAP_END);
	uint64_t *dir3 = (uint64_t*)(VADDR_CODE_BEGIN + (uint64_t)kern_satp_page - 0x80000000);
	for (size_t i = 256; i < 512; ++i)
	{
		if (dir3[i] & DIR_FLAG_V)
			continue;
		struct page *page;
		if (pm_alloc_page(&page))
			panic("failed to allocate dir2 page\n");
		dir3[i] = mkentry(page->offset, DIR_FLAG_V);
		memset(PMAP(page->offset * PAGE_SIZE), 0, PAGE_SIZE);
	}
	for (size_t i = 0; i < (size_t)&_kernel_size; i += PAGE_SIZE)
	{
		uintptr_t addr = 0x80000000 + i;
		uint64_t *dir;
		int ret = get_dir(NULL, (uintptr_t)PMAP(addr), 1, &dir);
		if (ret)
			panic("failed to get kern pmap\n");
		if (*dir)
			panic("pmap already exists\n");
		*dir = mkentry(addr / PAGE_SIZE, DIR_FLAG_W | DIR_FLAG_R | DIR_FLAG_V);
	}
	/* XXX remove identity pages */
}
