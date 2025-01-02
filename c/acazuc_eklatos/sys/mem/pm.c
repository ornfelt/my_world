#include <multiboot.h>
#include <errno.h>
#include <mem.h>

#define BITMAP_BPW (sizeof(size_t) * 8)

struct pm_pool
{
	size_t *bitmap;
	size_t bitmap_size; /* number of size_t */
	size_t bitmap_first_free;
	size_t offset; /* page offset */
	size_t count; /* pages count */
	size_t used; /* used pages */
	size_t admin; /* administrative pages (bitmap, struct page) */
	struct page *pages;
	struct mutex mutex;
	TAILQ_ENTRY(pm_pool) chain;
};

static TAILQ_HEAD(, pm_pool) pm_pools = TAILQ_HEAD_INITIALIZER(pm_pools);
static void *init_pm_vaddr;

void pm_init_page(struct page *page, uintptr_t poff)
{
	page->offset = poff;
	page->flags = 0;
	refcount_init(&page->refcount, 1);
}

static void update_pm_bitmap_first_free(struct pm_pool *pm_pool, size_t start)
{
	for (size_t i = start / BITMAP_BPW; i < pm_pool->bitmap_size; ++i)
	{
		if (pm_pool->bitmap[i] == (size_t)-1)
			continue;
		for (size_t j = 0; j < BITMAP_BPW; ++j)
		{
			if (pm_pool->bitmap[i] & ((size_t)1 << j))
				continue;
			pm_pool->bitmap_first_free = i * BITMAP_BPW + j;
			return;
		}
		panic("no empty bits\n");
	}
	pm_pool->bitmap_first_free = pm_pool->bitmap_size * BITMAP_BPW;
}

int pm_alloc_page(struct page **page)
{
	struct pm_pool *pm_pool;
	TAILQ_FOREACH(pm_pool, &pm_pools, chain)
	{
		mutex_spinlock(&pm_pool->mutex);
		if (pm_pool->bitmap_first_free >= pm_pool->bitmap_size * sizeof(size_t) * 8)
		{
			mutex_unlock(&pm_pool->mutex);
			continue;
		}
		size_t i = pm_pool->bitmap_first_free / BITMAP_BPW;
		size_t j = pm_pool->bitmap_first_free % BITMAP_BPW;
		if (pm_pool->bitmap[i] & ((size_t)1 << j))
			panic("invalid first page 0x%0*zu: 0x%0*zu\n",
			      (int)(sizeof(size_t) * 2), pm_pool->bitmap_first_free,
			      (int)(sizeof(size_t) * 2), pm_pool->bitmap[i]);
		pm_pool->bitmap[i] |= ((size_t)1 << j);
		*page = &pm_pool->pages[pm_pool->bitmap_first_free];
		assert(!refcount_get(&(*page)->refcount), "allocating referenced page (%p, %" PRIu32 " references)\n", (void*)(*page)->offset, refcount_get(&(*page)->refcount));
		pm_ref_page(*page);
		update_pm_bitmap_first_free(pm_pool, pm_pool->bitmap_first_free);
		pm_pool->used++;
		mutex_unlock(&pm_pool->mutex);
		return 0;
	}
	return -ENOMEM;
}

int pm_alloc_pages(struct page **page, size_t nb)
{
	if (!nb)
		return -EINVAL;
	struct pm_pool *pm_pool;
	TAILQ_FOREACH(pm_pool, &pm_pools, chain)
	{
		mutex_spinlock(&pm_pool->mutex);
		if (pm_pool->bitmap_first_free >= pm_pool->bitmap_size * BITMAP_BPW)
		{
			mutex_unlock(&pm_pool->mutex);
			continue;
		}
		for (size_t i = pm_pool->bitmap_first_free / BITMAP_BPW; i < pm_pool->bitmap_size; ++i)
		{
			if (pm_pool->bitmap[i] == (size_t)-1)
				continue;
			for (size_t j = 0; j < BITMAP_BPW; ++j)
			{
				if (pm_pool->bitmap[i] & ((size_t)1 << j))
					continue;
				size_t off = i * sizeof(size_t) * 8 + j;
				for (size_t k = 0; k < nb; ++k)
				{
					size_t v = off + k;
					if (pm_pool->bitmap[v / BITMAP_BPW] & ((size_t)1 << (v % BITMAP_BPW)))
						goto next_page;
				}
				for (size_t k = 0; k < nb; ++k)
				{
					size_t v = off + k;
					assert(!refcount_get(&pm_pool->pages[v].refcount), "allocating referenced page\n");
					pm_ref_page(&pm_pool->pages[v]);
				}
				for (size_t k = 0; k < nb; ++k)
				{
					size_t v = off + k;
					pm_pool->bitmap[v / BITMAP_BPW] |= ((size_t)1 << (v % BITMAP_BPW));
				}
				if (pm_pool->bitmap_first_free == off)
					update_pm_bitmap_first_free(pm_pool, off + nb);
				*page = &pm_pool->pages[off];
				pm_pool->used += nb;
				mutex_unlock(&pm_pool->mutex);
				return 0;
next_page:
				continue;
			}
		}
		mutex_unlock(&pm_pool->mutex);
	}
	return -ENOMEM;
}

void pm_free_page(struct page *page)
{
	if (!page)
		return;
	struct pm_pool *pm_pool;
	TAILQ_FOREACH(pm_pool, &pm_pools, chain)
	{
		if (page < pm_pool->pages
		 || page >= pm_pool->pages + pm_pool->count)
			continue;
		mutex_spinlock(&pm_pool->mutex);
		if (!refcount_get(&page->refcount))
			panic("page double free %p\n", page);
		if (refcount_dec(&page->refcount))
		{
			mutex_unlock(&pm_pool->mutex);
			return;
		}
		size_t delta = page - pm_pool->pages;
		size_t *bitmap = &pm_pool->bitmap[delta / BITMAP_BPW];
		size_t mask = ((size_t)1 << (delta % BITMAP_BPW));
		assert(*bitmap & mask, "free_page of unallocated page: %p\n", page);
		*bitmap &= ~mask;
		if (delta < pm_pool->bitmap_first_free)
			pm_pool->bitmap_first_free = delta;
		pm_pool->used--;
		mutex_unlock(&pm_pool->mutex);
		return;
	}
	panic("free_page of invalid address: %p\n", page);
}

void pm_free_pages(struct page *pages, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		pm_free_page(&pages[i]);
}

void pm_ref_page(struct page *page)
{
	refcount_inc(&page->refcount);
}

struct page *pm_get_page(uintptr_t off)
{
	struct pm_pool *pm_pool;

	TAILQ_FOREACH(pm_pool, &pm_pools, chain)
	{
		if (off >= pm_pool->offset
		 && off < pm_pool->offset + pm_pool->count)
			return &pm_pool->pages[off - pm_pool->offset];
	}
	return NULL;
}

static struct page *pm_fetch_page(size_t off)
{
	struct pm_pool *pm_pool;
	TAILQ_FOREACH(pm_pool, &pm_pools, chain)
	{
		if (off < pm_pool->offset
		 || off >= pm_pool->offset + pm_pool->count)
			continue;
		size_t pool_off = off - pm_pool->offset;
		size_t i = pool_off / BITMAP_BPW;
		size_t j = pool_off % BITMAP_BPW;
		if (!(pm_pool->bitmap[i] & ((size_t)1 << j)))
		{
			pm_pool->bitmap[i] |= ((size_t)1 << j);
			pm_pool->used++;
		}
		struct page *page = &pm_pool->pages[pool_off];
		pm_ref_page(page);
		if (pool_off == pm_pool->bitmap_first_free)
			update_pm_bitmap_first_free(pm_pool, pm_pool->bitmap_first_free);
		mutex_unlock(&pm_pool->mutex);
		return page;
	}
	return NULL;
}

int pm_fetch_pages(size_t off, size_t n, struct page **pages)
{
	for (size_t i = 0; i < n; ++i)
	{
		pages[i] = pm_fetch_page(off + i);
		if (!pages[i])
			return -ENOMEM; /* XXX */
	}
	return 0;
}

void pm_free_pt(uintptr_t off)
{
	struct page *page = pm_get_page(off);
	if (page)
		pm_free_page(page);
}

static void create_pool(uintptr_t poff, size_t pages)
{
	size_t pages_bytes = sizeof(struct page) * pages;
	size_t bitmap_words = (pages + (BITMAP_BPW - 1)) / BITMAP_BPW;
	size_t bitmap_bytes = bitmap_words * sizeof(size_t);
	size_t pm_pool_bytes = sizeof(struct pm_pool) + bitmap_bytes + pages_bytes;
	size_t pm_pool_pages = (pm_pool_bytes + PAGE_SIZE - 1) / PAGE_SIZE;

	struct pm_pool *pm_pool = init_pm_vaddr;
	init_pm_vaddr = (uint8_t*)init_pm_vaddr + PAGE_SIZE * pm_pool_pages;
	size_t used = arch_pm_init(pm_pool, poff, pages, pm_pool_pages);

	mutex_init(&pm_pool->mutex, 0);
	pm_pool->bitmap_size = bitmap_words;
	pm_pool->offset = poff;
	pm_pool->count = pages;
	pm_pool->used = used;
	pm_pool->bitmap_first_free = pm_pool->used;
	pm_pool->admin = pm_pool->used;
	pm_pool->bitmap = (void*)((uint8_t*)pm_pool + sizeof(struct pm_pool));
	pm_pool->pages = (void*)((uint8_t*)pm_pool->bitmap + bitmap_bytes);

	memset(pm_pool->bitmap, 0, bitmap_bytes);
	for (size_t i = 0; i < pm_pool->count; ++i)
	{
		struct page *page = &pm_pool->pages[i];
		page->offset = pm_pool->offset + i;
		page->flags = 0;
		refcount_init(&page->refcount, (i < pm_pool->used) ? 1 : 0);
	}

	for (size_t i = 0; i < pm_pool->bitmap_first_free; ++i)
		pm_pool->bitmap[i / BITMAP_BPW] |= ((size_t)1 << (i % BITMAP_BPW));

	TAILQ_INSERT_TAIL(&pm_pools, pm_pool, chain);
}

static void memory_iterator(uintptr_t addr, size_t size, void *userdata)
{
	(void)userdata;
	uintptr_t pad = addr % (16 * 1024 * 1024);
	if (pad >= size)
		return;
	addr += pad;
	size -= pad;
	size -= size % (16 * 1024 * 1024);
	if (!size)
		return;
	create_pool(addr / PAGE_SIZE, size / PAGE_SIZE);
}

void pm_init(uintptr_t kernel_reserved, uintptr_t heap_begin, uintptr_t heap_end)
{
	mutex_init(&g_vm_mutex, MUTEX_RECURSIVE);
	init_pm_vaddr = (void*)(((uintptr_t)heap_begin + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
	multiboot_iterate_memory(kernel_reserved, SIZE_MAX, memory_iterator, NULL);
	if (TAILQ_EMPTY(&pm_pools))
		panic("no pm ranges found\n");
	g_vm_heap.addr = (uintptr_t)init_pm_vaddr;
	g_vm_heap.size = heap_end - g_vm_heap.addr;
	TAILQ_INIT(&g_vm_heap.ranges);
}

void pm_dumpinfo(struct uio *uio)
{
	size_t size = 0;
	size_t used = 0;
	size_t admin = 0;
	struct pm_pool *pm_pool;
	TAILQ_FOREACH(pm_pool, &pm_pools, chain)
	{
		mutex_spinlock(&pm_pool->mutex);
		size += pm_pool->count;
		used += pm_pool->used;
		admin += pm_pool->admin;
		mutex_unlock(&pm_pool->mutex);
	}
	char buf[16];
	uprintf(uio, "PhysicalUsed:      0x%0*zx (%s)\n",
	        (int)sizeof(size_t) * 2, used * PAGE_SIZE,
	        vm_mem_fmt(buf, sizeof(buf), used * PAGE_SIZE));
	uprintf(uio, "PhysicalSize:      0x%0*zx (%s)\n",
	        (int)sizeof(size_t) * 2, size * PAGE_SIZE,
	        vm_mem_fmt(buf, sizeof(buf), size * PAGE_SIZE));
	uprintf(uio, "PhysicalReserved : 0x%0*zx (%s)\n",
	        (int)sizeof(size_t) * 2, admin * PAGE_SIZE,
	        vm_mem_fmt(buf, sizeof(buf), admin * PAGE_SIZE));
}
