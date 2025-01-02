#define ENABLE_TRACE

#include "arch/x86/x86.h"
#include "arch/x86/asm.h"
#include "arch/x86/msr.h"
#include "arch/x86/cr.h"

#include <errno.h>
#include <proc.h>
#include <std.h>
#include <cpu.h>
#include <mem.h>

#define PTE_MASK 0x1FF
#define ADDR_ALIGN(addr, mask) (((uint64_t)(addr) + (mask)) & ~(mask))

#define DIR_SHIFT(n) (12 + (n) * 9)
#define DIR_MASK(n) ((uint64_t)PTE_MASK << DIR_SHIFT(n))
#define DIR_ID(n, addr) ((((uint64_t)(addr)) & DIR_MASK(n)) >> DIR_SHIFT(n))
#define DIR_ALIGN(n, addr) ADDR_ALIGN(addr, ((uint64_t)1 << DIR_SHIFT(n + 1)) - 1)

#define DIR_FLAG_P     (1ULL << 0) /* is present */
#define DIR_FLAG_RW    (1ULL << 1) /* enable write */
#define DIR_FLAG_US    (1ULL << 2) /* userspace */
#define DIR_FLAG_PWT   (1ULL << 3) /* page write through */
#define DIR_FLAG_PCD   (1ULL << 4) /* page cache disabled */
#define DIR_FLAG_A     (1ULL << 5) /* accessed */
#define DIR_FLAG_D     (1ULL << 6) /* dirty */
#define DIR_FLAG_PS    (1ULL << 7) /* page size */
#define DIR_FLAG_G     (1ULL << 8) /* global */
#define DIR_FLAG_XD    (1ULL << 63) /* execute disable */
#define DIR_FLAG_MASK  (0xFFF0000000000FFFULL)

#define TBL_FLAG_P     (1ULL << 0) /* is present */
#define TBL_FLAG_RW    (1ULL << 1) /* enable write */
#define TBL_FLAG_US    (1ULL << 2) /* userspace */
#define TBL_FLAG_PWT   (1ULL << 3) /* page write through */
#define TBL_FLAG_PCD   (1ULL << 4) /* page cache disabled */
#define TBL_FLAG_A     (1ULL << 5) /* accessed */
#define TBL_FLAG_D     (1ULL << 6) /* dirty */
#define TBL_FLAG_PAT   (1ULL << 7) /* page attribute table */
#define TBL_FLAG_G     (1ULL << 8) /* global */
#define TBL_FLAG_XD    (1ULL << 63) /* execute disable */
#define TBL_FLAG_MASK  (0xFFF0000000000FFFULL)

#define DIR_POFF(val)  (DIR_PADDR(val) >> DIR_SHIFT(0))
#define DIR_PADDR(val) ((uint64_t)(val) & ~DIR_FLAG_MASK)

#define PMAP(addr) ((void*)(VADDR_PMAP_BEGIN + (uint64_t)(addr)))

extern uint8_t _kernel_end;
extern uint64_t kern_pml_page;

static uint64_t *early_vmap_pge = (uint64_t*)0xFFFFFFFFFFE00000;
static uint64_t *early_vmap_tbl = (uint64_t*)0xFFFFFFFFFFFFF000;

static inline uint64_t mkentry(uint64_t poff, uint64_t flags)
{
	return (poff << DIR_SHIFT(0)) | flags;
}

static void set_dir(struct vm_space *space, uint64_t addr, uint64_t *dir,
                    uint64_t poff, uint32_t prot)
{
	uint64_t f = poff ? TBL_FLAG_P : 0;
	if (space)
		f |= TBL_FLAG_US;
	if (prot & VM_PROT_W)
		f |= TBL_FLAG_RW;
	switch (prot & VM_TYPE_MASK)
	{
		case VM_WB:
			f |= 0; /* PAT 0 */
			break;
		case VM_WC:
			f |= TBL_FLAG_PAT; /* PAT 4 */
			break;
		case VM_UC:
		case VM_MMIO:
			f |= TBL_FLAG_PCD | TBL_FLAG_PWT; /* PAT 3 */
			break;
		case VM_WT:
			f |= TBL_FLAG_PWT; /* PAT 1 */
			break;
	}
	if (!(prot & VM_PROT_X))
		f |= TBL_FLAG_XD;
	*dir = mkentry(poff, f);
	if (!space)
	{
		invlpg(addr);
		return;
	}
	struct thread *thread = curcpu()->thread;
	if (thread && space == thread->proc->vm_space)
		invlpg(addr);
}

static int get_dir(struct vm_space *space, uint64_t addr, int create,
                   uint64_t **dirp)
{
	uint64_t pte_id = DIR_ID(3, addr);
	uint64_t *pte = space ? PMAP(pm_page_addr(space->arch.dir_page)) : PMAP(kern_pml_page);
	uint64_t f = DIR_FLAG_P | DIR_FLAG_RW;
	if (space)
		f |= DIR_FLAG_US;
	uint8_t shift = DIR_SHIFT(2);
	for (size_t i = 0; i < 3; ++i)
	{
		if (!(pte[pte_id] & DIR_FLAG_P))
		{
			if (!create)
				return -EINVAL;
			struct page *page;
			int ret = pm_alloc_page(&page);
			if (ret)
				return ret;
			pte[pte_id] = mkentry(page->offset, f);
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
		setcr3(pm_page_addr(space->arch.dir_page));
	else
		setcr3(kern_pml_page);
}

int arch_vm_space_init(struct vm_space *space)
{
	int ret = pm_alloc_page(&space->arch.dir_page);
	if (ret)
		return ret;
	uint64_t *dir = PMAP(pm_page_addr(space->arch.dir_page));
	memset(dir, 0, 256 * sizeof(uint64_t));
	memcpy(&dir[256],
	       &((uint64_t*)PMAP(kern_pml_page))[256],
	       256 * sizeof(uint64_t));
	return 0;
}

static void cleanup_level(uint64_t *pte, size_t min, size_t max, uint8_t level)
{
	for (size_t i = min; i < max; ++i)
	{
		uint64_t entry = pte[i];
		if (!(entry & DIR_FLAG_P))
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
		*dir0_dst = mkentry(src_poff, *dir0_src & TBL_FLAG_MASK);
		return 0;
	}
	int ret = pm_alloc_page(&page);
	if (ret)
		return ret;
	*dir0_dst = mkentry(page->offset, *dir0_src & TBL_FLAG_MASK);
	struct arch_copy_zone *src_zone = &curcpu()->copy_src_page;
	struct arch_copy_zone *dst_zone = &curcpu()->copy_dst_page;
	void *src = arch_set_copy_zone(src_zone, src_poff);
	void *dst = arch_set_copy_zone(dst_zone, page->offset);
	memcpy(__builtin_assume_aligned(dst, PAGE_SIZE),
	       __builtin_assume_aligned(src, PAGE_SIZE),
	       PAGE_SIZE);
	return 0;
}

static int copy_level(uint64_t *dst, const uint64_t *src, size_t min, size_t max,
                      uint8_t level)
{
	for (size_t i = min; i < max; ++i)
	{
		if (!(src[i] & DIR_FLAG_P))
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
		TRACE("failed to get vmap dir");
		return ret;
	}
	if (*dir & TBL_FLAG_P)
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
	if (*dir & TBL_FLAG_P)
		pm_free_pt(DIR_POFF(*dir));
	*dir = mkentry(0, 0);
	invlpg(addr);
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
		TRACE("failed to get dir");
		return ret;
	}
	uint64_t poff;
	if (*dir & TBL_FLAG_P)
	{
		if (prot & VM_PROT_X)
		{
			if (*dir & TBL_FLAG_XD)
				return -EFAULT;
		}
		else if (prot & VM_PROT_W)
		{
			if (!(*dir & TBL_FLAG_RW))
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
		panic("failed to get zero page dir\n");
}

void *arch_set_copy_zone(struct arch_copy_zone *zone, uintptr_t poff)
{
	*zone->dir0_ptr = mkentry(poff, TBL_FLAG_P | TBL_FLAG_RW | TBL_FLAG_XD);
	invlpg((uintptr_t)zone->ptr);
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
		uint64_t *pte = (uint64_t*)kern_pml_page;
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
			if (!(pte[pte_id] & DIR_FLAG_P))
			{
				uint64_t pte_poff = base + (*used)++;
				uint64_t pte_val = mkentry(pte_poff,
				                           DIR_FLAG_P | DIR_FLAG_RW);
				pte[pte_id] = pte_val;
				prev_pte[j] = pte_val;
				early_vmap_tbl[j + 1] = pte_val;
				pte = &early_vmap_pge[512 * (j + 1)];
				invlpg((uintptr_t)pte);
				memset(pte, 0, PAGE_SIZE);
			}
			else if (pte[pte_id] != prev_pte[j])
			{
				prev_pte[j] = pte[pte_id];
				early_vmap_tbl[j + 1] = pte[pte_id];
				pte = &early_vmap_pge[512 * (j + 1)];
				invlpg((uintptr_t)pte);
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
			pte[pte_id] = mkentry(poff, DIR_FLAG_P | DIR_FLAG_RW | DIR_FLAG_PS | DIR_FLAG_G | DIR_FLAG_XD);
			i += 512 * 512;
		}
		else if (n == 2)
		{
			pte[pte_id] = mkentry(poff, DIR_FLAG_P | DIR_FLAG_RW | DIR_FLAG_PS | DIR_FLAG_G | DIR_FLAG_XD);
			i += 512;
		}
		else
		{
			pte[pte_id] = mkentry(poff, TBL_FLAG_P | TBL_FLAG_RW | TBL_FLAG_G | TBL_FLAG_XD);
			i++;
		}
	}
}

size_t arch_pm_init(void *addr, uintptr_t base, size_t count, size_t pool_pages)
{
	size_t used = pool_pages;
	map_early(PMAP(PAGE_SIZE * base), base, count, 0, &used);
	map_early(addr, base, pool_pages, 0, &used);
	return used;
}

void arch_paging_init(void)
{
	pm_init(DIR_ALIGN(0, (uint64_t)&_kernel_end - VADDR_CODE_BEGIN),
	        VADDR_HEAP_BEGIN, VADDR_HEAP_END);
	uint64_t *dir3 = PMAP(kern_pml_page);
	for (size_t i = 256; i < 512; ++i)
	{
		if (dir3[i])
			continue;
		struct page *page;
		if (pm_alloc_page(&page))
			panic("failed to allocate pdp page\n");
		dir3[i] = mkentry(page->offset, DIR_FLAG_P | DIR_FLAG_RW);
		memset(PMAP(page->offset * PAGE_SIZE), 0, PAGE_SIZE);
	}
	/* remove identity paging */
	dir3[0] = 0;
	setcr4(getcr4() & ~CR4_PGE);
	setcr3(getcr3());
	setcr4(getcr4() | CR4_PGE);
}
