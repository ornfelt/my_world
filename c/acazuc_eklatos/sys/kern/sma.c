#include <errno.h>
#include <queue.h>
#include <mutex.h>
#include <file.h>
#include <sma.h>
#include <uio.h>
#include <vfs.h>
#include <std.h>
#include <mem.h>

/*
 * SMA (slab memory allocator)
 *
 * sma are composed of a list of struct sma_meta
 * an sma_meta is a PAGE_SIZE memory block containing a list of sma_slab
 * an sma_slab is a structure representing a PAGE_SIZE multiple memory block
 * each one of this memory block is containing only payload data
 */

#define BITMAP_BPW (sizeof(size_t) * 8)

#define BITMAP_MIN_SIZE 8 /* must never ever be 1 (it would make no partial / full distinction) */

#define SLAB_FIRST(meta) ((struct sma_slab*)&meta->slabs[0])

#define SLAB_FOREACH(slab, meta, sma) \
	for (slab = SLAB_FIRST(meta); \
	     slab < (struct sma_slab*)((uint8_t*)meta + PAGE_SIZE - sma->meta_size); \
	     slab = (struct sma_slab*)((uint8_t*)slab + sma->meta_size))

enum sma_slab_state
{
	SMA_SLAB_EMPTY,
	SMA_SLAB_PARTIAL,
	SMA_SLAB_FULL,
};

struct sma_slab
{
	uint8_t *addr;
	TAILQ_ENTRY(sma_slab) chain;
	enum sma_slab_state state;
	size_t first_free;
	size_t bitmap[];
};

struct sma_meta
{
	TAILQ_ENTRY(sma_meta) chain;
	TAILQ_HEAD(, sma_slab) slab_full;
	TAILQ_HEAD(, sma_slab) slab_partial;
	TAILQ_HEAD(, sma_slab) slab_empty;
	uint8_t slabs[];
};

static const char *states_str[] =
{
	[SMA_SLAB_EMPTY]   = "EMPTY",
	[SMA_SLAB_PARTIAL] = "PARTIAL",
	[SMA_SLAB_FULL]    = "FULL",
};

static struct spinlock sma_list_lock = SPINLOCK_INITIALIZER();
static TAILQ_HEAD(, sma) sma_list = TAILQ_HEAD_INITIALIZER(sma_list);
static int sysfs_enabled;

static int create_sysfs(struct sma *sma);

static void *mem_alloc(struct sma *sma, size_t bytes)
{
	void *ret = vmalloc(bytes);
	if (!ret)
		return NULL;
	sma->stats.nallocp += bytes / PAGE_SIZE;
	sma->stats.ncurrentp += bytes / PAGE_SIZE;
	return ret;
}

static void mem_free(struct sma *sma, void *addr, size_t bytes)
{
	if (!addr)
		return;
	sma->stats.nfreep += bytes / PAGE_SIZE;
	sma->stats.ncurrentp -= bytes / PAGE_SIZE;
	vfree(addr, bytes);
}

static void sma_lock(struct sma *sma)
{
	mutex_lock(&sma->mutex);
}

static void sma_unlock(struct sma *sma)
{
	mutex_unlock(&sma->mutex);
}

static void bitmap_set(struct sma_slab *slab, size_t offset)
{
	slab->bitmap[offset / BITMAP_BPW] |= ((size_t)1 << (offset % BITMAP_BPW));
}

static void bitmap_clr(struct sma_slab *slab, size_t offset)
{
	slab->bitmap[offset / BITMAP_BPW] &= ~((size_t)1 << (offset % BITMAP_BPW));
}

static size_t bitmap_get(struct sma_slab *slab, size_t offset)
{
	return slab->bitmap[offset / BITMAP_BPW] & ((size_t)1 << (offset % BITMAP_BPW));
}

static int slab_ctr(struct sma *sma, struct sma_slab *slab)
{
	slab->addr = mem_alloc(sma, sma->slab_size);
	if (!slab->addr)
		return -ENOMEM;
	memset(slab->bitmap, 0, sma->bitmap_size);
	if (sma->ctr)
	{
		for (size_t i = 0; i < sma->bitmap_count; ++i)
			sma->ctr(slab->addr + i * sma->data_size, sma->data_size);
	}
	sma->stats.nslabs++;
	return 0;
}

static void slab_dtr(struct sma *sma, struct sma_slab *slab)
{
	if (sma->dtr)
	{
		for (size_t i = 0; i < sma->bitmap_count; ++i)
			sma->dtr(slab->addr + i * sma->data_size, sma->data_size);
	}
	void *addr = slab->addr;
	slab->addr = NULL;
	mem_free(sma, addr, sma->slab_size);
	sma->stats.nslabs--;
}

static struct sma_meta *sma_meta_new(struct sma *sma)
{
	struct sma_meta *meta = mem_alloc(sma, PAGE_SIZE);
	if (!meta)
		return NULL;
	TAILQ_INIT(&meta->slab_empty);
	TAILQ_INIT(&meta->slab_partial);
	TAILQ_INIT(&meta->slab_full);
	struct sma_slab *slab;
	SLAB_FOREACH(slab, meta, sma)
	{
		slab->addr = NULL;
		slab->state = SMA_SLAB_EMPTY;
		TAILQ_INSERT_TAIL(&meta->slab_empty, slab, chain);
	}
	TAILQ_INSERT_HEAD(&sma->meta, meta, chain);
	sma->stats.nmetas++;
	return meta;
}

static void sma_meta_delete(struct sma *sma, struct sma_meta *meta)
{
	struct sma_slab *slab;
	SLAB_FOREACH(slab, meta, sma)
	{
		switch (slab->state)
		{
			case SMA_SLAB_EMPTY:
				TAILQ_REMOVE(&meta->slab_empty, slab, chain);
				break;
			case SMA_SLAB_PARTIAL:
				TAILQ_REMOVE(&meta->slab_partial, slab, chain);
				break;
			case SMA_SLAB_FULL:
				TAILQ_REMOVE(&meta->slab_full, slab, chain);
				break;
		}
		slab_dtr(sma, slab);
	}
	mem_free(sma, meta, PAGE_SIZE);
	sma->stats.nmetas--;
}

static void check_free_slab(struct sma *sma, struct sma_meta *meta,
                            struct sma_slab *slab)
{
	/* XXX check for free meta */
	if (slab->state == SMA_SLAB_FULL)
	{
		TAILQ_REMOVE(&meta->slab_full, slab, chain);
		slab->state = SMA_SLAB_PARTIAL;
		TAILQ_INSERT_HEAD(&meta->slab_partial, slab, chain);
		return;
	}
	for (size_t i = 0; i < sma->bitmap_words; ++i)
	{
		if (slab->bitmap[i])
			return;
	}
	TAILQ_REMOVE(&meta->slab_partial, slab, chain);
	slab->state = SMA_SLAB_EMPTY;
	if (!TAILQ_EMPTY(&meta->slab_empty)
	 && TAILQ_FIRST(&meta->slab_empty)->addr)
	{
		slab_dtr(sma, slab);
		TAILQ_INSERT_TAIL(&meta->slab_empty, slab, chain);
	}
	else
	{
		TAILQ_INSERT_HEAD(&meta->slab_empty, slab, chain);
	}
}

static void update_first_free(struct sma *sma, struct sma_meta *meta,
                              struct sma_slab *slab)
{
	for (size_t i = 0; i < sma->bitmap_words; ++i)
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
		panic("sma '%s': block != SIZE_MAX but no bit found", sma->name);
	}
full:
	TAILQ_REMOVE(&meta->slab_partial, slab, chain);
	slab->first_free = -1;
	slab->state = SMA_SLAB_FULL;
	TAILQ_INSERT_TAIL(&meta->slab_full, slab, chain);
}

static void *get_free_block(struct sma *sma)
{
	struct sma_meta *meta;
	struct sma_meta *empty_meta;
	struct sma_slab *empty_slab = NULL;

	TAILQ_FOREACH(meta, &sma->meta, chain)
	{
		struct sma_slab *slab;
		TAILQ_FOREACH(slab, &meta->slab_partial, chain)
		{
			size_t ret = slab->first_free;
			bitmap_set(slab, ret);
			update_first_free(sma, meta, slab);
			return slab->addr + ret * sma->data_size;
		}
		if (!empty_slab)
		{
			empty_slab = TAILQ_FIRST(&meta->slab_empty);
			empty_meta = meta;
		}
	}
	if (!empty_slab)
		return NULL;
	if (!empty_slab->addr)
	{
		if (slab_ctr(sma, empty_slab))
			return NULL;
	}
	TAILQ_REMOVE(&empty_meta->slab_empty, empty_slab, chain);
	empty_slab->state = SMA_SLAB_PARTIAL;
	bitmap_set(empty_slab, 0);
	empty_slab->first_free = 1;
	TAILQ_INSERT_HEAD(&empty_meta->slab_partial, empty_slab, chain);
	return empty_slab->addr;
}

void *sma_alloc(struct sma *sma, int flags)
{
	sma_lock(sma);
	void *addr = get_free_block(sma);
	if (!addr)
	{
		struct sma_meta *meta = sma_meta_new(sma);
		if (!meta)
			goto err;
		struct sma_slab *slab = SLAB_FIRST(meta);
		if (slab_ctr(sma, slab))
		{
			sma_meta_delete(sma, meta);
			goto err;
		}
		TAILQ_REMOVE(&meta->slab_empty, slab, chain);
		slab->state = SMA_SLAB_PARTIAL;
		bitmap_set(slab, 0);
		slab->first_free = 1;
		TAILQ_INSERT_HEAD(&meta->slab_partial, slab, chain);
		addr = slab->addr;
	}
	sma->stats.nalloc++;
	sma->stats.ncurrent++;
	sma_unlock(sma);
	if (flags & M_ZERO)
		memset(addr, 0, sma->data_size);
	return addr;

err:
	sma_unlock(sma);
	return NULL;
}

static int slab_contains(struct sma *sma, struct sma_slab *slab, void *ptr,
                         size_t *item)
{
	if ((uint8_t*)ptr < slab->addr
	 || (uint8_t*)ptr >= slab->addr + sma->data_size * sma->bitmap_count)
		return 1;
	*item = ((uint8_t*)ptr - slab->addr) / sma->data_size;
	if (slab->addr + sma->data_size * *item != (uint8_t*)ptr)
		return 1;
	return 0;
}

static struct sma_slab *find_ptr_slab(struct sma *sma, void *ptr,
                                      struct sma_meta **meta, size_t *item)
{
	struct sma_slab *slab;
	TAILQ_FOREACH(*meta, &sma->meta, chain)
	{
		TAILQ_FOREACH(slab, &(*meta)->slab_partial, chain)
		{
			if (!slab_contains(sma, slab, ptr, item))
				return slab;
		}
		TAILQ_FOREACH(slab, &(*meta)->slab_full, chain)
		{
			if (!slab_contains(sma, slab, ptr, item))
				return slab;
		}
	}
	return NULL;
}

int sma_free(struct sma *sma, void *ptr)
{
	struct sma_meta *meta;
	struct sma_slab *slab;
	size_t item;

	if (!ptr)
		return -EINVAL;
	sma_lock(sma);
	slab = find_ptr_slab(sma, ptr, &meta, &item);
	if (!slab)
	{
		sma_unlock(sma);
		return -EINVAL;
	}
	if (!bitmap_get(slab, item))
		panic("sma '%s': double free %p\n", sma->name, ptr);
	bitmap_clr(slab, item);
	if (item < slab->first_free)
		slab->first_free = item;
	check_free_slab(sma, meta, slab);
	sma->stats.nfree++;
	sma->stats.ncurrent--;
	sma_unlock(sma);
	return 0;
}

void *sma_move(struct sma *dst, struct sma *src, void *ptr, int flags)
{
	struct sma_slab *slab;
	struct sma_meta *meta;
	size_t item;

	sma_lock(src);
	slab = find_ptr_slab(src, ptr, &meta, &item);
	if (!slab)
	{
		sma_unlock(src);
		return NULL;
	}
	if (!bitmap_get(slab, item))
		panic("sma '%s': double free %p\n", src->name, ptr);
	void *addr = sma_alloc(dst, flags);
	if (!addr)
	{
		sma_unlock(src);
		return NULL;
	}
	if (dst->data_size >= src->data_size)
	{
		memcpy(addr, ptr, src->data_size);
		if (flags & M_ZERO)
			memset((uint8_t*)addr + src->data_size, 0,
			       dst->data_size - src->data_size);
	}
	else
	{
		memcpy(addr, ptr, dst->data_size);
	}
	bitmap_clr(slab, item);
	if (item < slab->first_free)
		slab->first_free = item;
	src->stats.ncurrent--;
	check_free_slab(src, meta, slab);
	sma_unlock(src);
	return addr;
}

int sma_own(struct sma *sma, void *ptr)
{
	struct sma_meta *meta;
	struct sma_slab *slab;
	size_t item;

	sma_lock(sma);
	slab = find_ptr_slab(sma, ptr, &meta, &item);
	sma_unlock(sma);
	return slab != NULL;
}

int sma_init(struct sma *sma, size_t data_size, sma_ctr_t ctr, sma_dtr_t dtr,
             const char *name)
{
	TAILQ_INIT(&sma->meta);
	sma->ctr = ctr;
	sma->dtr = dtr;
	if (data_size < PAGE_SIZE / BITMAP_MIN_SIZE)
	{
		sma->bitmap_count = PAGE_SIZE / data_size;
		sma->bitmap_count -= (sizeof(struct sma_slab) + sizeof(size_t) * (sma->bitmap_count + BITMAP_BPW - 1) / BITMAP_BPW + data_size - 1) / data_size;
		if (sma->bitmap_count < BITMAP_MIN_SIZE)
			sma->bitmap_count = BITMAP_MIN_SIZE;
	}
	else
	{
		sma->bitmap_count = BITMAP_MIN_SIZE;
	}
	sma->bitmap_words = (sma->bitmap_count + BITMAP_BPW - 1) / BITMAP_BPW;
	sma->bitmap_size = sizeof(size_t) * sma->bitmap_words;
	sma->data_size = data_size;
	sma->meta_size = sizeof(struct sma_slab) + sma->bitmap_size;
	sma->slab_size = sma->data_size * sma->bitmap_count + sma->meta_size;
	sma->slab_size += PAGE_SIZE - 1;
	sma->slab_size -= sma->slab_size % PAGE_SIZE;
	memset(&sma->stats, 0, sizeof(sma->stats));
	mutex_init(&sma->mutex, MUTEX_RECURSIVE);
	sma->name = name;
	spinlock_lock(&sma_list_lock);
	TAILQ_INSERT_TAIL(&sma_list, sma, chain);
	if (sysfs_enabled)
		create_sysfs(sma);
	spinlock_unlock(&sma_list_lock);
	return 0;
}

void sma_destroy(struct sma *sma)
{
	spinlock_lock(&sma_list_lock);
	TAILQ_REMOVE(&sma_list, sma, chain);
	spinlock_unlock(&sma_list_lock);
	struct sma_meta *meta, *nxt;
	TAILQ_FOREACH_SAFE(meta, &sma->meta, chain, nxt)
		sma_meta_delete(sma, meta);
	mutex_destroy(&sma->mutex);
}

static ssize_t print_block(struct uio *uio, size_t start, size_t end,
                           size_t blocks, size_t size)
{
	return uprintf(uio, "   0x%0*zx - 0x%0*zx : %zu blocks / %zu bytes\n",
	               (int)(sizeof(void*) / 4), start,
	               (int)(sizeof(void*) / 4), end, blocks, size);
}

static ssize_t c_e(struct sma *sma, struct uio *uio, void **start, void *end)
{
	size_t size = (uint8_t*)end - (uint8_t*)*start;
	ssize_t ret = print_block(uio, (size_t)*start, (size_t)end,
	                          size / sma->data_size, size);
	if (ret < 0)
		return ret;
	*start = NULL;
	return size;
}

static ssize_t print_slab(struct sma *sma, struct sma_slab *slab,
                          struct uio *uio)
{
	size_t total = 0;
	void *start = NULL;
	ssize_t ret;
	size_t i;

	for (i = 0; i < sma->bitmap_count; ++i)
	{
		size_t set = bitmap_get(slab, i);
		if (set && !start)
		{
			start = slab->addr + i * sma->data_size;
			continue;
		}
		if (!set && start)
		{
			ret = c_e(sma, uio, &start, slab->addr + i * sma->data_size);
			if (ret < 0)
				return ret;
			total += ret;
		}
	}
	if (start)
	{
		ret = c_e(sma, uio, &start, slab->addr + i * sma->data_size);
		if (ret < 0)
			return ret;
		total += ret;
	}
	return total;
}

int sma_print(struct sma *sma, struct uio *uio)
{
	struct sma_meta *meta;
	size_t total = 0;
	ssize_t ret;

	sma_lock(sma);
	ret = uprintf(uio, "sma %s %p\n", sma->name, sma);
	if (ret < 0)
		goto end;
	TAILQ_FOREACH(meta, &sma->meta, chain)
	{
		struct sma_slab *slab;
		ret = uprintf(uio, " meta %p\n", meta);
		if (ret < 0)
			goto end;
		SLAB_FOREACH(slab, meta, sma)
		{
			ret = uprintf(uio, "  slab %p @ %p (%s)\n",
			              slab, slab->addr, states_str[slab->state]);
			if (ret < 0)
				goto end;
			ret = print_slab(sma, slab, uio);
			if (ret < 0)
				goto end;
			total += ret;
		}
	}
	ret = uprintf(uio, "total    : %zu bytes\n", total);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "data size: %zu\n", sma->data_size);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "meta size: %zu\n", sma->meta_size);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "slab size: %zu\n", sma->slab_size);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "bitmap nb: %zu\n", sma->bitmap_count);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "alloc    : %" PRIu64 "\n", sma->stats.nalloc);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "free     : %" PRIu64 "\n", sma->stats.nfree);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "current  : %" PRIu64 "\n", sma->stats.ncurrent);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "metas    : %" PRIu64 "\n", sma->stats.nmetas);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "slabs    : %" PRIu64 "\n", sma->stats.nslabs);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "allocp   : %" PRIu64 "\n", sma->stats.nallocp);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "freep    : %" PRIu64 "\n", sma->stats.nfreep);
	if (ret < 0)
		goto end;
	ret = uprintf(uio, "currentp : %" PRIu64 "\n", sma->stats.ncurrentp);
	if (ret < 0)
		goto end;
	ret = 0;

end:
	sma_unlock(sma);
	return ret;
}

static int sma_sys_open(struct file *file, struct node *node)
{
	file->userdata = node->userdata;
	return 0;
}

static ssize_t sma_sys_read(struct file *file, struct uio *uio)
{
	struct sma *sma = file->userdata;
	size_t count = uio->count;
	off_t off = uio->off;
	sma_print(sma, uio);
	uio->off = off + count - uio->count;
	return count - uio->count;
}

static const struct file_op sma_fop =
{
	.open = sma_sys_open,
	.read = sma_sys_read,
};

static int create_sysfs(struct sma *sma)
{
	if (!sma->name)
		return 0;
	if (sma->sysfs_node)
		return -EINVAL;
	char path[1024];
	snprintf(path, sizeof(path), "sma/%s", sma->name);
	int ret = sysfs_mknode(path, 0, 0, 0400, &sma_fop, &sma->sysfs_node);
	if (ret)
		return ret;
	sma->sysfs_node->userdata = sma;
	return 0;
}

void sma_register_sysfs(void)
{
	struct sma *sma;
	spinlock_lock(&sma_list_lock);
	sysfs_enabled = 1;
	TAILQ_FOREACH(sma, &sma_list, chain)
		create_sysfs(sma);
	spinlock_unlock(&sma_list_lock);
}
