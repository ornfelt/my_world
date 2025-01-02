#include <queue.h>
#include <mutex.h>
#include <sma.h>
#include <std.h>
#include <mem.h>

#define MALLOC_LOCK() mutex_lock(&g_ctx.mutex)
#define MALLOC_UNLOCK() mutex_unlock(&g_ctx.mutex)

enum block_type
{
	BLOCK_1, /* XXX only at least 4 ? */
	BLOCK_2,
	BLOCK_4,
	BLOCK_8,
	BLOCK_16,
	BLOCK_32,
	BLOCK_64,
	BLOCK_128,
	BLOCK_256,
	BLOCK_512,
	BLOCK_1024,
	BLOCK_2048,
	BLOCK_LARGE
};

static const size_t block_sizes[] =
{
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128,
	256,
	512,
	1024,
	2048,
	0
};

static const char *sma_names[] =
{
	"malloc_1",
	"malloc_2",
	"malloc_4",
	"malloc_8",
	"malloc_16",
	"malloc_32",
	"malloc_64",
	"malloc_128",
	"malloc_256",
	"malloc_512",
	"malloc_1024",
	"malloc_2048",
};

struct large_page
{
	size_t page_size; /* size of vmalloc */
	size_t data_size; /* size of payload */
	uint8_t *addr;
	TAILQ_ENTRY(large_page) chain;
};

struct malloc_ctx
{
	struct sma sma[BLOCK_LARGE];
	struct sma large_sma;
	TAILQ_HEAD(, large_page) large_pages;
	struct mutex mutex;
};

static struct malloc_ctx g_ctx;

static enum block_type get_block_type(size_t size)
{
	for (size_t i = 0; i < sizeof(block_sizes) / sizeof(*block_sizes); ++i)
	{
		if (size <= block_sizes[i])
			return i;
	}
	return BLOCK_LARGE;
}

static struct large_page *alloc_large_page(size_t size)
{
	struct large_page *page;
	size_t alloc_size;

	page = sma_alloc(&g_ctx.large_sma, 0);
	if (!page)
		return NULL;
	alloc_size = size;
	alloc_size += PAGE_SIZE - 1;
	alloc_size -= alloc_size % PAGE_SIZE;
	page->addr = vmalloc(alloc_size);
	if (!page->addr)
	{
		sma_free(&g_ctx.large_sma, page);
		return NULL;
	}
	page->page_size = alloc_size;
	page->data_size = size;
	return page;
}

static void free_large_page(struct large_page *page)
{
	vfree(page->addr, page->page_size);
	sma_free(&g_ctx.large_sma, page);
}

static void *create_new_large_page(size_t size)
{
	struct large_page *page;

	page = alloc_large_page(size);
	if (!page)
		return NULL;
	TAILQ_INSERT_TAIL(&g_ctx.large_pages, page, chain);
	return page->addr;
}

static void *realloc_large(struct large_page *page, void *ptr, size_t size,
                           uint32_t flags)
{
	void *addr;
	size_t len;

	addr = create_new_large_page(size);
	if (!addr)
	{
		MALLOC_UNLOCK();
		return NULL;
	}
	len = page->data_size;
	if (size < len)
		len = size;
	if (flags & M_ZERO)
		memset((uint8_t*)addr + len, 0, size - len);
	memcpy(addr, ptr, len);
	TAILQ_REMOVE(&g_ctx.large_pages, page, chain);
	free_large_page(page);
	MALLOC_UNLOCK();
	return addr;
}

void *malloc(size_t size, uint32_t flags)
{
	enum block_type type;
	void *addr;

	if (!size)
		return NULL;
	type = get_block_type(size);
	if (type != BLOCK_LARGE)
		return sma_alloc(&g_ctx.sma[type], flags);
	MALLOC_LOCK();
	addr = create_new_large_page(size);
	MALLOC_UNLOCK();
	if (!addr)
		return NULL;
	if (flags & M_ZERO)
		memset(addr, 0, size);
	return addr;
}

void free(void *ptr)
{
	if (!ptr)
		return;
	for (size_t i = 0; i < sizeof(g_ctx.sma) / sizeof(*g_ctx.sma); ++i)
	{
		if (!sma_free(&g_ctx.sma[i], ptr))
			return;
	}
	MALLOC_LOCK();
	struct large_page *page;
	TAILQ_FOREACH(page, &g_ctx.large_pages, chain)
	{
		if (ptr != page->addr)
			continue;
		TAILQ_REMOVE(&g_ctx.large_pages, page, chain);
		free_large_page(page);
		MALLOC_UNLOCK();
		return;
	}
	MALLOC_UNLOCK();
	panic("free unknown addr: %p\n", ptr);
}

void *realloc(void *ptr, size_t size, uint32_t flags)
{
	if (!ptr)
		return malloc(size, flags);
	if (!size)
	{
		free(ptr);
		return NULL;
	}
	enum block_type type = get_block_type(size);
	if (type == BLOCK_LARGE)
	{
		for (size_t i = 0; i < sizeof(g_ctx.sma) / sizeof(*g_ctx.sma); ++i)
		{
			if (!sma_own(&g_ctx.sma[i], ptr))
				continue;
			void *addr = create_new_large_page(size);
			if (!addr)
				return NULL;
			memcpy(addr, ptr, block_sizes[i]);
			sma_free(&g_ctx.sma[i], ptr);
			return addr;
		}
		MALLOC_LOCK();
		struct large_page *page;
		TAILQ_FOREACH(page, &g_ctx.large_pages, chain)
		{
			if (ptr != page->addr)
				continue;
			return realloc_large(page, ptr, size, flags);
		}
		MALLOC_UNLOCK();
		panic("realloc unknown addr %p\n", ptr);
		return NULL;
	}
	struct sma *dst_sma = &g_ctx.sma[type];
	for (size_t i = 0; i < sizeof(g_ctx.sma) / sizeof(*g_ctx.sma); ++i)
	{
		if (!sma_own(&g_ctx.sma[i], ptr))
			continue;
		if (i == type)
			return ptr;
		return sma_move(dst_sma, &g_ctx.sma[i], ptr, flags);
	}
	MALLOC_LOCK();
	struct large_page *page;
	TAILQ_FOREACH(page, &g_ctx.large_pages, chain)
	{
		if (ptr != page->addr)
			continue;
		void *addr = sma_alloc(dst_sma, flags);
		memcpy(addr, ptr, size);
		TAILQ_REMOVE(&g_ctx.large_pages, page, chain);
		free_large_page(page);
		MALLOC_UNLOCK();
		return addr;
	}
	MALLOC_UNLOCK();
	panic("realloc unknown addr %p\n", ptr);
	return NULL;
}

void alloc_init(void)
{
	for (size_t i = 0; i < BLOCK_LARGE; ++i)
	{
		if (sma_init(&g_ctx.sma[i], block_sizes[i], NULL, NULL, sma_names[i]))
			panic("failed to create sma %d\n", (int)i);
	}
	if (sma_init(&g_ctx.large_sma, sizeof(struct large_page), NULL, NULL, "malloc_large"))
		panic("failed to create large sma\n");
	TAILQ_INIT(&g_ctx.large_pages);
	mutex_init(&g_ctx.mutex, 0);
}

static ssize_t print_block(struct uio *uio, size_t start, size_t end, size_t len)
{
	return uprintf(uio, " 0x%zx - 0x%zx : 0x%zx bytes\n", start, end, len);
}

static ssize_t print_page(struct large_page *page, struct uio *uio)
{
	ssize_t ret = print_block(uio, (size_t)page->addr,
	                          (size_t)(page->addr + page->data_size),
	                          page->data_size);
	if (ret < 0)
		return ret;
	return page->data_size;
}

int alloc_print(struct uio *uio)
{
	struct large_page *page;
	size_t total;
	ssize_t ret;

	for (size_t i = 0; i < sizeof(g_ctx.sma) / sizeof(*g_ctx.sma); ++i)
	{
		ret = sma_print(&g_ctx.sma[i], uio);
		if (ret < 0)
			return ret;
	}
	MALLOC_LOCK();
	total = 0;
	TAILQ_FOREACH(page, &g_ctx.large_pages, chain)
	{
		ret = uprintf(uio, "LARGE : %p (0x%zx)\n", page, page->page_size);
		if (ret < 0)
			goto end;
		ret = print_page(page, uio);
		if (ret < 0)
			goto end;
		total += ret;
	}
	ret = uprintf(uio, "total: %zu bytes\n", total);
	if (ret < 0)
		goto end;
	ret = 0;

end:
	MALLOC_UNLOCK();
	return ret;
}
