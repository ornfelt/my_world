#include "../_lock.h"
#include "_alloc.h"

#include <sys/queue.h>
#include <sys/mman.h>

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* allocate "chunks" of 4MB size
 * 4MB will make use of large pages when implemented
 * chunks must be aligned on 4MB boundary
 *
 * huge allocations (>= 4MB):
 * - give N chunks directly
 * - if free() of a 4MB-aligned pointer, the pointer is considered as a huge
 *   allocation because other allocations contains metadata at the beginning
 *   of the chunk, making malloc() never return a 4MB-aligned pointer
 * - store them into a global tree (>= 4MB allocations are very rare, lock
 *   contention shouldn't be a big problem)
 *
 * large allocations (>= PAGE_SIZE):
 * - get pages (aligned) from the buddy allocator
 * - on free, retrieve the chunk metadata (stored at the 4MB low boundary) to
 *   get the used buddy for the allocation
 *
 * small allocations (everything else):
 * - use a slab memory allocation to split pages allocated from the buddy
 *   allocator
 * - on free, retrieve the chunk metadata (stored at the 4MB low boundary) to
 *   get the slab used for the allocation
 *
 * on allocation, if the buddy allocator doesn't have anymore pages,
 * create a new chunk
 *
 * overall, allocations and deallocations should be O(1)
 * lock contention may happen during:
 * - huge allocations, when inserting entry into the tree
 * - huge deallocations, when removing entry from the tree
 * - large allocations, when searching for free buddy (may be reduced by using
 *   an arena for each thread)
 * - large deallocations, when searching for the buddy (may be reduced a little
 *   by using an arena for each thread, if and only if allocations and
 *   deallocations are done on the same thread)
 * - small allocations
 * - small deallocations
 *
 * nomenclature comes from jemalloc paper
 *
 * sma are composed of a list of struct meta
 * a meta is a PAGE_SIZE memory block containing a list of slab
 * a slab is a structure representing a PAGE_SIZE multiple memory block
 * each one of this memory block is containing only payload data
 *
 * overhead for 100000000 allocations:
 * 1 byte:   424%
 * 4 bytes:  5.91%
 * 8 bytes:  4.33%
 * 16 bytes: 3.80%
 * 64 bytes: 3.21%
 *
 * overhead for 10000000 allocations:
 * 1024 bytes: 1.99%
 * 2048 bytes: 1.79%
 * 4096 bytes: 1.58%
 */

#define CHUNK_SIZE_LOG2      (22)
#define CHUNK_SIZE           (1 << CHUNK_SIZE_LOG2)
#define PAGE_SIZE_LOG2       (12)
#define PAGE_SIZE            (1 << PAGE_SIZE_LOG2)
#define CHUNK_MAGIC          ((uint64_t)0xDEAD0A2AE11E1337)
#define CHUNK_ORDERS_COUNT   (CHUNK_SIZE_LOG2 - PAGE_SIZE_LOG2 + 1)
#define CHUNK_EXTENT_COUNT   (CHUNK_SIZE / PAGE_SIZE * 2 - 1)
#define CHUNK_EXTENT_BYTES   (CHUNK_EXTENT_COUNT * sizeof(struct extent))
#define CHUNK_METADATA_BYTES (sizeof(struct chunk) + CHUNK_EXTENT_BYTES)
#define CHUNK_METADATA_PAGES ((CHUNK_METADATA_BYTES + PAGE_SIZE - 1) / PAGE_SIZE)
#define BITMAP_BPW           (sizeof(size_t) * 8)
#define BITMAP_MIN_SIZE      16 /* must not be 1 to keep partial / full distinction */
#define SMALL_SIZES_COUNT    (sizeof(small_sizes) / sizeof(*small_sizes))
#define LARGE_SIZES_COUNT    (sizeof(large_sizes) / sizeof(*large_sizes))
#define ARENA_MMAP_SIZE      ((sizeof(struct arena) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define SLAB_FIRST(meta)     (&meta->slabs[0])
#define SLAB_FOREACH(slab, meta, sma) \
	for (slab = SLAB_FIRST(meta); \
	     slab < (struct slab*)((uint8_t*)meta + PAGE_SIZE - sma->meta_size); \
	     slab = (struct slab*)((uint8_t*)slab + sma->meta_size))

#define ALLOC_ABORT(fmt, ...) \
do \
{ \
	fprintf(stderr, "%s:%d " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); \
	abort(); \
} while (0)

struct extent;
struct arena;
struct chunk;
struct slab;
struct meta;
struct sma;

enum small_type
{
	SMALL_4,    SMALL_8,    SMALL_16,   SMALL_32,
	SMALL_48,   SMALL_64,   SMALL_80,   SMALL_96,
	SMALL_112,  SMALL_128,  SMALL_144,  SMALL_160,
	SMALL_176,  SMALL_192,  SMALL_208,  SMALL_224,
	SMALL_240,  SMALL_256,  SMALL_272,  SMALL_288,
	SMALL_304,  SMALL_320,  SMALL_336,  SMALL_352,
	SMALL_368,  SMALL_384,  SMALL_400,  SMALL_416,
	SMALL_432,  SMALL_448,  SMALL_464,  SMALL_480,
	SMALL_496,  SMALL_512,  SMALL_768,  SMALL_1024,
	SMALL_1280, SMALL_1536, SMALL_1792, SMALL_2048,
	SMALL_2304, SMALL_2560, SMALL_2816, SMALL_3072,
	SMALL_3328, SMALL_3584, SMALL_3840,
};

static const size_t
small_sizes[] =
{
	4,    8,    16,   32,
	48,   64,   80,   96,
	112,  128,  144,  160,
	176,  192,  208,  224,
	240,  256,  272,  288,
	304,  320,  336,  352,
	368,  384,  400,  416,
	432,  448,  464,  480,
	496,  512,  768,  1024,
	1280, 1536, 1792, 2048,
	2304, 2560, 2816, 3072,
	3328, 3584, 3840,
};

enum large_type
{
	LARGE_4K,
	LARGE_8K,
	LARGE_16K,
	LARGE_32K,
	LARGE_64K,
	LARGE_128K,
	LARGE_256K,
	LARGE_512K,
	LARGE_1024K,
	LARGE_2048K,
};

static const size_t
large_sizes[] =
{
	1024 * 4,
	1024 * 8,
	1024 * 16,
	1024 * 32,
	1024 * 64,
	1024 * 128,
	1024 * 256,
	1024 * 512,
	1024 * 1024,
	1024 * 2048,
};

enum slab_state
{
	SLAB_EMPTY,
	SLAB_PARTIAL,
	SLAB_FULL,
};

struct slab
{
	struct meta *meta;
	uint8_t *addr;
	TAILQ_ENTRY(slab) chain;
	enum slab_state state;
	size_t used;
	size_t first_free;
	size_t bitmap[];
};

struct meta
{
	struct sma *sma;
	TAILQ_ENTRY(meta) chain;
	TAILQ_HEAD(, slab) slab_full;
	TAILQ_HEAD(, slab) slab_partial;
	TAILQ_HEAD(, slab) slab_empty;
	struct slab slabs[];
};

struct sma
{
	struct arena *arena;
	TAILQ_HEAD(, meta) meta_full;
	TAILQ_HEAD(, meta) meta_partial;
	struct meta *meta_free;
	size_t bitmap_words;
	size_t bitmap_count;
	size_t bitmap_bytes;
	size_t slab_pages;
	size_t data_size;
	size_t meta_size;
	struct _libc_lock lock;
	int initialized;
};

struct arena
{
	struct sma sma[SMALL_SIZES_COUNT];
	TAILQ_HEAD(, chunk) chunks_lists[CHUNK_ORDERS_COUNT];
	TAILQ_HEAD(, chunk) chunks;
	struct chunk *free_chunk;
	struct _libc_lock lock;
};

struct extent
{
	union
	{
		struct slab *slab;
		size_t large_size;
		uintptr_t userdata;
	};
	uint32_t split;
	/* NB: while index 0 is technically a valid extent id,
	 *     it's reserved for metadata storage, so we can use 0
	 *     as a NULL marker
	 * NB: use index to lower (divide by 4 on 64bit) the overhead
	 */
	uint16_t free_prev;
	uint16_t free_next;
};

struct chunk
{
	uint64_t magic;
	struct arena *arena;
	size_t used;
	uint16_t free_lists[CHUNK_ORDERS_COUNT];
	TAILQ_ENTRY(chunk) lists_chains[CHUNK_ORDERS_COUNT];
	TAILQ_ENTRY(chunk) chain;
	struct extent extents[];
};

struct huge_alloc
{
	void *data;
	size_t size;
	TAILQ_ENTRY(huge_alloc) chain;
};

static TAILQ_HEAD(, huge_alloc) huge_allocs = TAILQ_HEAD_INITIALIZER(huge_allocs);
static struct _libc_lock huge_lock;

/* XXX use a pool of arena to get better multithread performances */
static struct arena *g_arena;
static struct _libc_lock init_lock;

static void *arena_alloc(struct arena *arena, size_t pages, uintptr_t userdata);
static void arena_free(struct arena *arena, void *ptr);

static void chunk_destroy(struct chunk *chunk);

static void sma_init(struct sma *sma, size_t data_size, struct arena *arena);
static void sma_destroy(struct sma *sma);
static void *sma_alloc(struct sma *sma);
static void sma_free(struct slab *slab, void *ptr);

static uint32_t
npot32(uint32_t val)
{
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	return ++val;
}

static void *
mem_alloc(size_t size)
{
	return mmap(NULL,
	            size,
	            PROT_READ | PROT_WRITE,
	            MAP_ANONYMOUS | MAP_PRIVATE,
	            -1,
	            0);
}

static struct arena *
arena_new(void)
{
	struct arena *arena;

	arena = mem_alloc(ARENA_MMAP_SIZE);
	if (!arena)
		return NULL;
	for (size_t i = 0; i < SMALL_SIZES_COUNT; ++i)
		sma_init(&arena->sma[i], small_sizes[i], arena);
	for (size_t i = 0; i < CHUNK_ORDERS_COUNT; ++i)
		TAILQ_INIT(&arena->chunks_lists[i]);
	TAILQ_INIT(&arena->chunks);
	arena->free_chunk = NULL;
	_libc_lock_init(&arena->lock);
	return arena;
}

__attribute__((unused))
static void
arena_destroy(struct arena *arena)
{
	struct chunk *chunk;

	for (size_t i = 0; i < SMALL_SIZES_COUNT; ++i)
		sma_destroy(&arena->sma[i]);
	chunk = TAILQ_FIRST(&arena->chunks);
	while (chunk)
	{
		TAILQ_REMOVE(&chunk->arena->chunks, chunk, chain);
		chunk_destroy(chunk);
		chunk = TAILQ_FIRST(&arena->chunks);
	}
	if (arena->free_chunk)
		chunk_destroy(arena->free_chunk);
	munmap(arena, ARENA_MMAP_SIZE);
}

static void
print_extent(struct chunk *chunk,
             struct extent *extent,
             size_t order,
             size_t indent)
{
	printf("%.*sextent %p(0x%zx) %s\n",
	       (int)indent * 2, "                                   ",
	       extent,
	       (size_t)(extent - chunk->extents),
	       extent->split ? "split" : (extent->userdata ? "used" : "free"));
	if (!extent->split)
		return;
	size_t off = extent - chunk->extents;
	size_t mask_bit = 1 << (CHUNK_ORDERS_COUNT - order);
	size_t mask = mask_bit - 1;
	size_t left_off = (off & ~((mask << 1) | 1))
	                | ((off & (mask >> 1)) << 1);
	struct extent *left = &chunk->extents[left_off];
	struct extent *right = &chunk->extents[left_off + 1];
	print_extent(chunk, left, order - 1, indent + 1);
	print_extent(chunk, right, order - 1, indent + 1);
}

static void
print_chunk(struct chunk *chunk)
{
	printf("    chunk %p\n", chunk);
	printf("      free lists\n");
	for (size_t i = 0; i < CHUNK_ORDERS_COUNT; ++i)
	{
		if (!chunk->free_lists[i])
			continue;
		printf("        %2zu\n", i);
		uint16_t id = chunk->free_lists[i];
		while (id)
		{
			struct extent *extent = &chunk->extents[id];
			printf("          [0x%" PRIx16 "] %p (0x%" PRIx16 " - 0x%" PRIx16 ")\n",
			      id, extent, extent->free_prev, extent->free_next);
			id = extent->free_next;
		}
	}
	printf("      extents\n");
	print_extent(chunk, &chunk->extents[CHUNK_SIZE / PAGE_SIZE * 2 - 2],
	             CHUNK_ORDERS_COUNT - 1, 4);
}

static void
print_slab(struct slab *slab)
{
	printf("          slab %p: USED=%zu FIRST_FREE=%zu STATE=%d ADDR=%p\n",
	      slab, slab->used, slab->first_free, slab->state, slab->addr);
}

static void
print_meta(struct meta *meta)
{
	struct slab *slab;

	printf("      meta %p\n", meta);
	printf("        slab full\n");
	TAILQ_FOREACH(slab, &meta->slab_full, chain)
		print_slab(slab);
	printf("        slab partial\n");
	TAILQ_FOREACH(slab, &meta->slab_partial, chain)
		print_slab(slab);
	printf("        slab empty\n");
	TAILQ_FOREACH(slab, &meta->slab_empty, chain)
		print_slab(slab);
}

static void
print_sma(struct sma *sma)
{
	struct meta *meta;

	if (!sma->initialized)
		return;
	printf("  sma %p [%zu bytes]\n", sma, sma->data_size);
	printf("    meta full\n");
	TAILQ_FOREACH(meta, &sma->meta_full, chain)
		print_meta(meta);
	printf("    meta partial\n");
	TAILQ_FOREACH(meta, &sma->meta_partial, chain)
		print_meta(meta);
	printf("    meta free %p\n", sma->meta_free);
}

static void
print_arena_chunks_lists(struct arena *arena)
{
	struct chunk *chunk;

	printf("  chunks_lists\n");
	for (size_t i = 0; i < CHUNK_ORDERS_COUNT; ++i)
	{
		if (TAILQ_EMPTY(&arena->chunks_lists[i]))
			continue;
		printf("    [%zu]", i);
		TAILQ_FOREACH(chunk, &arena->chunks_lists[i], chain)
			printf(" %p", chunk);
		printf("\n");
	}
}

static void
print_arena_chunks(struct arena *arena)
{
	struct chunk *chunk;

	printf("  chunks\n");
	TAILQ_FOREACH(chunk, &arena->chunks, chain)
		print_chunk(chunk);
}

static void
print_arena_sma(struct arena *arena)
{
	for (size_t i = 0; i < SMALL_SIZES_COUNT; ++i)
		print_sma(&arena->sma[i]);
}

__attribute__((unused))
static void
print_arena(struct arena *arena)
{
	printf("arena %p\n", arena);
	print_arena_chunks_lists(arena);
	print_arena_chunks(arena);
	print_arena_sma(arena);
}

static struct chunk *
chunk_get(void *ptr)
{
	struct chunk *chunk;

	chunk = (struct chunk*)((uintptr_t)ptr & ~(CHUNK_SIZE - 1));
	if (chunk->magic != CHUNK_MAGIC)
		return NULL;
	return chunk;
}

static struct extent *
chunk_get_extent(struct chunk *chunk, void *ptr)
{
	size_t id = ((uint8_t*)ptr - (uint8_t*)chunk) / PAGE_SIZE;
	size_t off = 0;

	for (size_t i = 0; i < CHUNK_ORDERS_COUNT; ++i)
	{
		struct extent *extent = &chunk->extents[off | id];
		if (extent->userdata)
			return extent;
		if (extent->split)
			ALLOC_ABORT("corrupted split");
		off |= 1 << (CHUNK_ORDERS_COUNT - i - 1);
		id >>= 1;
	}
	return NULL;
}

static struct extent *
extent_get(void *ptr, struct chunk **chunkp)
{
	struct extent *extent;
	struct chunk *chunk;

	chunk = chunk_get(ptr);
	if (!chunk)
		return NULL;
	extent = chunk_get_extent(chunk, ptr);
	if (!extent)
		return NULL;
	*chunkp = chunk;
	return extent;
}

static void
chunk_add_free(struct chunk *chunk, struct extent *extent, size_t order)
{
	size_t off;

	off = extent - chunk->extents;
	extent->free_prev = 0;
	extent->free_next = chunk->free_lists[order];
	if (extent->free_next)
		chunk->extents[extent->free_next].free_prev = off;
	else
		TAILQ_INSERT_TAIL(&chunk->arena->chunks_lists[order], chunk, lists_chains[order]);
	chunk->free_lists[order] = off;
}

static void
chunk_remove_free(struct chunk *chunk, struct extent *extent, size_t order)
{
	if (!extent->free_prev && !extent->free_next)
		TAILQ_REMOVE(&chunk->arena->chunks_lists[order], chunk, lists_chains[order]);
	if (extent->free_next)
		chunk->extents[extent->free_next].free_prev = extent->free_prev;
	if (extent->free_prev)
		chunk->extents[extent->free_prev].free_next = extent->free_next;
	else
		chunk->free_lists[order] = extent->free_next;
}

static void
extent_coalesce(struct chunk *chunk, size_t off, size_t order)
{
	struct extent *extent;
	struct extent *buddy;

	if (order == CHUNK_ORDERS_COUNT)
		return;
	extent = &chunk->extents[off];
	extent->split = 0;
	extent->userdata = 0;
	buddy = &chunk->extents[off ^ 1];
	if (buddy->split || buddy->userdata)
	{
		chunk_add_free(chunk, extent, order);
		return;
	}
	chunk_remove_free(chunk, buddy, order);
	size_t mask_bit = 1 << (CHUNK_ORDERS_COUNT - order - 1);
	size_t mask = mask_bit - 1;
	size_t parent_off = (off & ~mask) | mask_bit | ((off & mask) >> 1);
	extent_coalesce(chunk, parent_off, order + 1);
}

static void
chunk_free(struct chunk *chunk, struct extent *extent)
{
	size_t off = extent - chunk->extents;
	size_t mask = 1 << (CHUNK_ORDERS_COUNT - 1);
	size_t order = 0;
	while (off & mask)
	{
		order++;
		mask >>= 1;
	}
	chunk->used -= 1 << order;
	extent_coalesce(chunk, off, order);
	if (chunk->used != npot32(CHUNK_METADATA_PAGES))
		return;
	for (size_t i = 0; i < CHUNK_ORDERS_COUNT; ++i)
	{
		if (!chunk->free_lists[i])
			continue;
		TAILQ_REMOVE(&chunk->arena->chunks_lists[i],
		             chunk, lists_chains[i]);
	}
	TAILQ_REMOVE(&chunk->arena->chunks, chunk, chain);
	if (chunk->arena->free_chunk)
		chunk_destroy(chunk);
	else
		chunk->arena->free_chunk = chunk;
}

static struct extent *
extent_split(struct chunk *chunk,
             struct extent *extent,
             size_t split_order,
             size_t order)
{
	if (split_order == order)
		return extent;
	extent->split = 1;
	size_t off = extent - chunk->extents;
	size_t mask_bit = 1 << (CHUNK_ORDERS_COUNT - split_order);
	size_t mask = mask_bit - 1;
	size_t left_off = (off & ~((mask << 1) | 1))
	                | ((off & (mask >> 1)) << 1);
	struct extent *left = &chunk->extents[left_off];
	struct extent *right = &chunk->extents[left_off + 1];
	chunk_add_free(chunk, right, split_order - 1);
	return extent_split(chunk, left, split_order - 1, order);
}

static size_t
pages_to_order(size_t pages)
{
	size_t order = 0;

	while (1)
	{
		if (order == CHUNK_ORDERS_COUNT)
			ALLOC_ABORT("invalid allocation pages count: %zu", pages);
		if (pages == (1u << order))
			return order;
		order++;
	}
}

static void *
chunk_alloc(struct chunk *chunk, size_t pages, uintptr_t userdata)
{
	struct extent *extent;
	size_t order;

	order = pages_to_order(pages);
	if (!chunk->free_lists[order])
	{
		size_t split_order = order;
		while (1)
		{
			split_order++;
			if (split_order == CHUNK_ORDERS_COUNT)
				return NULL;
			if (!chunk->free_lists[split_order])
				continue;
			extent = &chunk->extents[chunk->free_lists[split_order]];
			break;
		}
		chunk_remove_free(chunk, extent, split_order);
		extent = extent_split(chunk, extent, split_order, order);
	}
	else
	{
		extent = &chunk->extents[chunk->free_lists[order]];
		chunk_remove_free(chunk, extent, order);
	}
	extent->userdata = userdata;
	chunk->used += 1 << order;
	size_t off = extent - chunk->extents;
	size_t mask_bit = 1 << (CHUNK_ORDERS_COUNT - order - 1);
	size_t mask = mask_bit - 1;
	return &((uint8_t*)chunk)[PAGE_SIZE * ((off & mask) << order)];
}

static struct chunk *
chunk_new(struct arena *arena)
{
	struct chunk *chunk;

	chunk = mmap(NULL, CHUNK_SIZE, PROT_READ | PROT_WRITE,
	             MAP_ANONYMOUS | MAP_PRIVATE | MAP_ALIGNED(CHUNK_SIZE_LOG2),
	             -1, 0);;
	if (!chunk)
		return NULL;
	chunk->magic = CHUNK_MAGIC;
	chunk->arena = arena;
	chunk->used = 0;
	memset(chunk->extents, 0, CHUNK_EXTENT_BYTES);
	memset(chunk->free_lists, 0, sizeof(chunk->free_lists));
	chunk->free_lists[CHUNK_ORDERS_COUNT - 1] = CHUNK_EXTENT_COUNT - 1;
	TAILQ_INSERT_TAIL(&chunk->arena->chunks_lists[CHUNK_ORDERS_COUNT - 1],
	                  chunk, lists_chains[CHUNK_ORDERS_COUNT - 1]);
	if (!chunk_alloc(chunk, npot32(CHUNK_METADATA_PAGES), -1))
		ALLOC_ABORT("failed to allocate metadata pages");
	return chunk;
}

static void
chunk_destroy(struct chunk *chunk)
{
	munmap(chunk, CHUNK_SIZE);
}

static void *
arena_alloc(struct arena *arena, size_t pages, uintptr_t owner)
{
	struct chunk *chunk;
	size_t order;

	order = pages_to_order(pages);
	_libc_lock(&arena->lock);
	chunk = TAILQ_FIRST(&arena->chunks_lists[order]);
	if (!chunk)
	{
		for (size_t i = order + 1; i < CHUNK_ORDERS_COUNT; ++i)
		{
			chunk = TAILQ_FIRST(&arena->chunks_lists[i]);
			if (chunk)
				break;
		}
	}
	if (chunk)
	{
		void *ptr = chunk_alloc(chunk, pages, owner);
		_libc_unlock(&arena->lock);
		if (!ptr)
			ALLOC_ABORT("chunk allocation failed");
		return ptr;
	}
	if (arena->free_chunk)
	{
		chunk = arena->free_chunk;
		arena->free_chunk = NULL;
		for (size_t i = 0; i < CHUNK_ORDERS_COUNT; ++i)
		{
			if (!chunk->free_lists[i])
				continue;
			TAILQ_INSERT_TAIL(&chunk->arena->chunks_lists[i],
			                  chunk, lists_chains[i]);
		}
	}
	else
	{
		chunk = chunk_new(arena);
		if (!chunk)
		{
			_libc_unlock(&arena->lock);
			return NULL;
		}
	}
	void *ptr = chunk_alloc(chunk, pages, owner);
	if (!ptr)
		ALLOC_ABORT("failed to allocate pages from new chunk");
	TAILQ_INSERT_TAIL(&arena->chunks, chunk, chain);
	_libc_unlock(&arena->lock);
	return ptr;
}

static void
arena_free(struct arena *arena, void *ptr)
{
	struct chunk *chunk;
	struct extent *extent;

	(void)arena;
	extent = extent_get(ptr, &chunk);
	if (!extent)
		ALLOC_ABORT("invalid chunk free");
	_libc_lock(&arena->lock);
	chunk_free(chunk, extent);
	_libc_unlock(&arena->lock);
}

static struct arena *
get_arena(void)
{
	_libc_lock(&init_lock);
	if (!g_arena)
	{
		struct arena *arena = arena_new();
		if (!arena)
			ALLOC_ABORT("failed to create arena");
		g_arena = arena;
	}
	_libc_unlock(&init_lock);
	return g_arena;
}

static void *
alloc_huge_alloc(size_t *size)
{
	*size += CHUNK_SIZE - 1;
	*size -= *size % CHUNK_SIZE;
	return mem_alloc(*size);
}

static void
bitmap_set(struct slab *slab, size_t offset)
{
	slab->bitmap[offset / BITMAP_BPW] |= ((size_t)1 << (offset % BITMAP_BPW));
}

static void
bitmap_clr(struct slab *slab, size_t offset)
{
	slab->bitmap[offset / BITMAP_BPW] &= ~((size_t)1 << (offset % BITMAP_BPW));
}

static size_t
bitmap_get(struct slab *slab, size_t offset)
{
	return slab->bitmap[offset / BITMAP_BPW] & ((size_t)1 << (offset % BITMAP_BPW));
}

static int
slab_ctr(struct sma *sma, struct meta *meta, struct slab *slab)
{
	slab->meta = meta;
	slab->addr = arena_alloc(sma->arena, sma->slab_pages, (uintptr_t)slab);
	if (!slab->addr)
		return -ENOMEM;
	memset(slab->bitmap, 0, sma->bitmap_bytes);
	return 0;
}

static void
slab_destroy(struct sma *sma, struct slab *slab)
{
	void *addr;

	addr = slab->addr;
	if (!addr)
		return;
	slab->addr = NULL;
	arena_free(sma->arena, addr);
}

static struct meta *
meta_new(struct sma *sma)
{
	struct meta *meta;
	struct slab *slab;

	meta = sma->meta_free;
	if (meta)
	{
		sma->meta_free = NULL;
		return meta;
	}
	meta = arena_alloc(sma->arena, 1, -1);
	if (!meta)
		return NULL;
	meta->sma = sma;
	TAILQ_INIT(&meta->slab_empty);
	TAILQ_INIT(&meta->slab_partial);
	TAILQ_INIT(&meta->slab_full);
	SLAB_FOREACH(slab, meta, sma)
	{
		slab->addr = NULL;
		slab->state = SLAB_EMPTY;
		slab->used = 0;
		slab->first_free = 0;
		TAILQ_INSERT_TAIL(&meta->slab_empty, slab, chain);
	}
	return meta;
}

static void
meta_destroy(struct sma *sma, struct meta *meta)
{
	struct slab *slab;

	if (!sma->meta_free)
	{
		sma->meta_free = meta;
		return;
	}
	TAILQ_FOREACH(slab, &meta->slab_full, chain)
		slab_destroy(sma, slab);
	TAILQ_FOREACH(slab, &meta->slab_partial, chain)
		slab_destroy(sma, slab);
	slab = TAILQ_FIRST(&meta->slab_empty);
	if (slab)
		slab_destroy(sma, slab);
	arena_free(sma->arena, meta);
}

static void
check_free_slab(struct sma *sma, struct meta *meta, struct slab *slab)
{
	if (slab->state == SLAB_FULL)
	{
		if (TAILQ_EMPTY(&meta->slab_partial)
		 && TAILQ_EMPTY(&meta->slab_empty))
		{
			TAILQ_REMOVE(&sma->meta_full, meta, chain);
			TAILQ_INSERT_TAIL(&sma->meta_partial, meta, chain);
		}
		TAILQ_REMOVE(&meta->slab_full, slab, chain);
		TAILQ_INSERT_HEAD(&meta->slab_partial, slab, chain);
		slab->state = SLAB_PARTIAL;
		return;
	}
	if (slab->used)
		return;
	TAILQ_REMOVE(&meta->slab_partial, slab, chain);
	slab->state = SLAB_EMPTY;
	if (!TAILQ_EMPTY(&meta->slab_empty)
	 && TAILQ_FIRST(&meta->slab_empty)->addr)
	{
		slab_destroy(sma, slab);
		TAILQ_INSERT_TAIL(&meta->slab_empty, slab, chain);
	}
	else
	{
		TAILQ_INSERT_HEAD(&meta->slab_empty, slab, chain);
	}
	if (TAILQ_EMPTY(&meta->slab_full)
	 && TAILQ_EMPTY(&meta->slab_partial))
	{
		TAILQ_REMOVE(&sma->meta_partial, meta, chain);
		meta_destroy(sma, meta);
	}
}

static void
update_first_free(struct sma *sma,
                  struct meta *meta,
                  struct slab *slab,
                  size_t start)
{
	if (slab->used == sma->bitmap_count)
		goto full;
	for (size_t i = start / BITMAP_BPW; i < sma->bitmap_words; ++i)
	{
		size_t bitmap = slab->bitmap[i];
		if (bitmap == SIZE_MAX)
			continue;
		if (!bitmap)
		{
			slab->first_free = i * BITMAP_BPW;
			return;
		}
		for (size_t j = 0; j < BITMAP_BPW; ++j)
		{
			if (bitmap & ((size_t)1 << j))
				continue;
			size_t ret = i * BITMAP_BPW + j;
			if (ret >= sma->bitmap_count)
				goto full;
			slab->first_free = ret;
			return;
		}
		ALLOC_ABORT("block != SIZE_MAX but no bit found");
	}

full:
	TAILQ_REMOVE(&meta->slab_partial, slab, chain);
	TAILQ_INSERT_TAIL(&meta->slab_full, slab, chain);
	slab->state = SLAB_FULL;
	slab->first_free = -1;
	if (TAILQ_EMPTY(&meta->slab_partial)
	 && TAILQ_EMPTY(&meta->slab_empty))
	{
		TAILQ_REMOVE(&sma->meta_partial, meta, chain);
		TAILQ_INSERT_TAIL(&sma->meta_full, meta, chain);
	}
}

/* lazy loading because there is a lot of sma for each arena
 * and most of them won't be used
 */
static void
sma_initialize(struct sma *sma)
{
	TAILQ_INIT(&sma->meta_full);
	TAILQ_INIT(&sma->meta_partial);
	sma->meta_free = NULL;
	sma->bitmap_count = PAGE_SIZE / sma->data_size;
	if (sma->bitmap_count < BITMAP_MIN_SIZE)
		sma->bitmap_count = BITMAP_MIN_SIZE;
	sma->bitmap_words = (sma->bitmap_count + BITMAP_BPW - 1) / BITMAP_BPW;
	sma->bitmap_bytes = sizeof(size_t) * sma->bitmap_words;
	sma->meta_size = sizeof(struct slab) + sma->bitmap_bytes;
	sma->slab_pages = sma->data_size * sma->bitmap_count;
	sma->slab_pages += PAGE_SIZE - 1;
	sma->slab_pages /= PAGE_SIZE;
	/* always allocate a power of two number of pages because pages
	 * are managed by a buddy allocator
	 */
	sma->slab_pages = npot32(sma->slab_pages);
	/* XXX
	 * maybe we could store even more data here because of the extra
	 * size of the page align + power of two
	 */
	sma->initialized = 1;
}

static void *
sma_alloc(struct sma *sma)
{
	struct meta *meta;
	struct slab *slab;
	void *addr = NULL;

	_libc_lock(&sma->lock);
	if (!sma->initialized)
		sma_initialize(sma);
	meta = TAILQ_FIRST(&sma->meta_partial);
	if (!meta)
	{
		meta = meta_new(sma);
		if (!meta)
			goto end;
		TAILQ_INSERT_TAIL(&sma->meta_partial, meta, chain);
	}
	slab = TAILQ_FIRST(&meta->slab_partial);
	if (slab)
	{
		size_t ret = slab->first_free;
		bitmap_set(slab, ret);
		slab->used++;
		update_first_free(sma, meta, slab, ret + 1);
		addr = slab->addr + ret * sma->data_size;
		goto end;
	}
	slab = TAILQ_FIRST(&meta->slab_empty);
	if (!slab)
		ALLOC_ABORT("meta without available slab");
	if (!slab->addr)
	{
		if (slab_ctr(sma, meta, slab))
			goto end;
	}
	TAILQ_REMOVE(&meta->slab_empty, slab, chain);
	TAILQ_INSERT_HEAD(&meta->slab_partial, slab, chain);
	slab->state = SLAB_PARTIAL;
	bitmap_set(slab, 0);
	slab->used = 1;
	slab->first_free = 1;
	addr = slab->addr;

end:
	_libc_unlock(&sma->lock);
	return addr;
}

static void
sma_free(struct slab *slab, void *ptr)
{
	struct meta *meta = slab->meta;
	struct sma *sma = meta->sma;
	size_t item;

	_libc_lock(&sma->lock);
	if (!sma->initialized)
		ALLOC_ABORT("uninitialized sma?");
	if ((uint8_t*)ptr < slab->addr
	 || (uint8_t*)ptr >= slab->addr + sma->data_size * sma->bitmap_count)
		ALLOC_ABORT("invalid slab pointer");
	item = ((uint8_t*)ptr - slab->addr) / sma->data_size;
	if (slab->addr + sma->data_size * item != (uint8_t*)ptr)
		ALLOC_ABORT("invalid slab pointer");
	if (!bitmap_get(slab, item))
		ALLOC_ABORT("double free %p", ptr);
	bitmap_clr(slab, item);
	slab->used--;
	if (item < slab->first_free)
		slab->first_free = item;
	check_free_slab(sma, meta, slab);
	_libc_unlock(&sma->lock);
}

static void
sma_init(struct sma *sma, size_t data_size, struct arena *arena)
{
	sma->arena = arena;
	sma->initialized = 0;
	sma->data_size = data_size;
	_libc_lock_init(&sma->lock);
}

static void
sma_destroy(struct sma *sma)
{
	struct meta *meta;

	if (!sma->initialized)
		return;
	meta = TAILQ_FIRST(&sma->meta_full);
	while (meta)
	{
		meta_destroy(sma, meta);
		meta = TAILQ_FIRST(&sma->meta_full);
	}
	meta = TAILQ_FIRST(&sma->meta_partial);
	while (meta)
	{
		meta_destroy(sma, meta);
		meta = TAILQ_FIRST(&sma->meta_partial);
	}
	if (sma->meta_free)
		meta_destroy(sma, sma->meta_free);
	sma->initialized = 0;
}

static void *
_malloc_huge(size_t size)
{
	struct huge_alloc *huge_alloc;
	void *data;

	data = alloc_huge_alloc(&size);
	if (!data)
		return NULL;
	huge_alloc = malloc(sizeof(*huge_alloc));
	if (!huge_alloc)
	{
		munmap(data, size);
		return NULL;
	}
	huge_alloc->data = data;
	huge_alloc->size = size;
	_libc_lock(&huge_lock);
	TAILQ_INSERT_TAIL(&huge_allocs, huge_alloc, chain);
	_libc_unlock(&huge_lock);
	return data;
}

static void *
_malloc_large(struct arena *arena, size_t size)
{
	for (size_t i = 0; i < LARGE_SIZES_COUNT; ++i)
	{
		if (size > large_sizes[i])
			continue;
		return arena_alloc(arena, large_sizes[i] / PAGE_SIZE, 1 + i);
	}
	ALLOC_ABORT("bogus large size");
}

static void *
_malloc_small(struct arena *arena, size_t size)
{
	for (size_t i = 0; i < SMALL_SIZES_COUNT; ++i)
	{
		if (size > small_sizes[i])
			continue;
		return sma_alloc(&arena->sma[i]);
	}
	ALLOC_ABORT("bogus small size");
}

void *
_malloc(size_t size)
{
	struct arena *arena;

	arena = get_arena();
	if (size >= CHUNK_SIZE)
		return _malloc_huge(size);
	if (size >= PAGE_SIZE)
		return _malloc_large(arena, size);
	return _malloc_small(arena, size);
}

static void
_free_huge(void *ptr)
{
	struct huge_alloc *huge_alloc;

	_libc_lock(&huge_lock);
	TAILQ_FOREACH(huge_alloc, &huge_allocs, chain)
	{
		if (huge_alloc->data != ptr)
			continue;
		TAILQ_REMOVE(&huge_allocs, huge_alloc, chain);
		_libc_unlock(&huge_lock);
		munmap(huge_alloc->data, huge_alloc->size);
		free(huge_alloc);
		return;
	}
	_libc_unlock(&huge_lock);
	ALLOC_ABORT("free unknown addr: %p", ptr);
}

void
_free(void *ptr)
{
	struct chunk *chunk;
	struct extent *extent;
	struct arena *arena;

	if (!((uintptr_t)ptr & (CHUNK_SIZE - 1)))
	{
		_free_huge(ptr);
		return;
	}
	extent = extent_get(ptr, &chunk);
	if (!extent
	 || extent->userdata == (uintptr_t)-1)
		ALLOC_ABORT("free unknown addr: %p", ptr);
	arena = chunk->arena;
	if (extent->large_size <= LARGE_SIZES_COUNT)
	{
		_libc_lock(&arena->lock);
		chunk_free(chunk, extent);
		_libc_unlock(&arena->lock);
		return;
	}
	sma_free(extent->slab, ptr);
}

static void *
_realloc_huge(void *ptr, size_t size)
{
	struct huge_alloc *huge_alloc;
	void *data;

	_libc_lock(&huge_lock);
	TAILQ_FOREACH(huge_alloc, &huge_allocs, chain)
	{
		if (huge_alloc->data != ptr)
			continue;
		/* we don't touch list pointers in this sequence,
		 * so it's safe to unlock at this point
		 */
		_libc_unlock(&huge_lock);
		/* XXX we should resize here if the difference is too big */
		if (size <= huge_alloc->size)
			return ptr;
		data = alloc_huge_alloc(&size);
		if (!data)
			return NULL;
		memmove(data, huge_alloc->data, huge_alloc->size);
		munmap(huge_alloc->data, huge_alloc->size);
		huge_alloc->data = data;
		huge_alloc->size = size;
		return 0;
	}
	_libc_unlock(&huge_lock);
	ALLOC_ABORT("free unknown addr: %p", ptr);
}

static void *
_realloc_large(struct chunk *chunk,
               struct extent *extent,
               void *ptr,
               size_t size)
{
	struct arena *arena = chunk->arena;
	void *new_data;
	size_t large_size;

	large_size = large_sizes[extent->large_size - 1];
	if (size > large_size / 2 && size <= large_size)
		return ptr;
	/* XXX could probably try to grow the allocation in the chunk
	 * but it's okay for now
	 */
	new_data = _malloc(size);
	if (!new_data)
		return NULL;
	if (size > large_size)
		memcpy(new_data, ptr, large_size);
	else
		memcpy(new_data, ptr, size);
	_libc_lock(&arena->lock);
	chunk_free(chunk, extent);
	_libc_unlock(&arena->lock);
	return new_data;
}

static void *
_realloc_small(struct chunk *chunk,
               struct extent *extent,
               void *ptr,
               size_t size)
{
	struct slab *slab = extent->slab;
	struct sma *sma = slab->meta->sma;
	void *new_data;

	(void)chunk;
	if (size <= sma->data_size
	 && size > sma->data_size / 2)
		return ptr;
	new_data = _malloc(size);
	if (!new_data)
		return NULL;
	if (size > sma->data_size)
		memcpy(new_data, ptr, sma->data_size);
	else
		memcpy(new_data, ptr, size);
	sma_free(slab, ptr);
	return new_data;
}

void *
_realloc(void *ptr, size_t size)
{
	struct chunk *chunk;
	struct extent *extent;

	if (!((uintptr_t)ptr & (CHUNK_SIZE - 1)))
		return _realloc_huge(ptr, size);
	extent = extent_get(ptr, &chunk);
	if (!extent
	 || extent->userdata == (uintptr_t)-1)
		ALLOC_ABORT("free unknown addr: %p", ptr);
	if (extent->large_size <= LARGE_SIZES_COUNT)
		return _realloc_large(chunk, extent, ptr, size);
	return _realloc_small(chunk, extent, ptr, size);
}
