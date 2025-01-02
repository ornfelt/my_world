#include <errno.h>
#include <sma.h>
#include <mem.h>
#include <cpu.h>

static struct sma vm_range_sma;
static struct vm_range *range_cache;

void vm_region_init_sma(void)
{
	sma_init(&vm_range_sma, sizeof(struct vm_range), NULL, NULL, "vm_range");
}

void vm_region_print(struct uio *uio, struct vm_region *region)
{
	uprintf(uio, "region %p:\n", region);
	struct vm_range *range;
	TAILQ_FOREACH(range, &region->ranges, chain)
	{
		uprintf(uio, "\t0x%0*zx - 0x%0*zx\n",
		        (int)(sizeof(uintptr_t) * 2), range->addr,
		        (int)(sizeof(uintptr_t) * 2), range->addr + range->size);
	}
}

int vm_region_alloc(struct vm_region *region, uintptr_t addr, size_t size,
                    size_t alignment, uintptr_t *ret)
{
	if (!alignment)
		alignment = PAGE_SIZE;
	else if (alignment & (alignment - 1)) /* test power of two */
		return -EINVAL;
	size_t align_mask = alignment - 1;
	if (!size)
		return -EINVAL;
	if (addr & align_mask)
		return -EINVAL;
	uintptr_t end;
	if (__builtin_add_overflow(addr, size, &end))
		return -EOVERFLOW;
	if (addr && (addr < region->addr || end > region->addr + region->size))
		return -ENOMEM;
	if (TAILQ_EMPTY(&region->ranges))
	{
		if (!addr || addr == region->addr)
		{
			size_t pad = region->addr & align_mask;
			if (!pad)
			{
				region->range_0.addr = region->addr + size;
				region->range_0.size = region->size - size;
				TAILQ_INSERT_HEAD(&region->ranges, &region->range_0, chain);
				*ret = region->addr;
				goto end;
			}
			/* XXX this will cause problem if the first allocation
			 * goes into this code path
			 */
			pad = alignment - pad;
			struct vm_range *newr = sma_alloc(&vm_range_sma, 0);
			if (!newr)
				return -ENOMEM;
			newr->addr = region->addr;
			newr->size = pad;
			TAILQ_INSERT_TAIL(&region->ranges, newr, chain);
			region->range_0.addr = region->addr + size + pad;
			region->range_0.size = region->size - size - pad;
			TAILQ_INSERT_TAIL(&region->ranges, &region->range_0, chain);
			*ret = region->addr + pad;
			goto end;
		}
		if (end == region->addr + region->size)
		{
			region->range_0.addr = region->addr;
			region->range_0.size = region->size - size;
			TAILQ_INSERT_HEAD(&region->ranges, &region->range_0, chain);
			*ret = region->addr + region->size - size;
			goto end;
		}
		struct vm_range *newr = sma_alloc(&vm_range_sma, 0);
		if (!newr)
			return -ENOMEM;
		newr->addr = region->addr;
		newr->size = addr - region->addr;
		TAILQ_INSERT_TAIL(&region->ranges, newr, chain);
		region->range_0.addr = end;
		region->range_0.size = region->addr + region->size
		                     - region->range_0.addr;
		TAILQ_INSERT_TAIL(&region->ranges, &region->range_0, chain);
		*ret = addr;
		goto end;
	}
	struct vm_range *item;
	if (addr)
	{
		TAILQ_FOREACH(item, &region->ranges, chain)
		{
			if (addr < item->addr)
				return -ENOMEM;
			if (addr >= item->addr + item->size)
				continue;
			if (size > item->size)
				continue;
			if (item->size == size)
			{
				if (item->addr != addr)
					continue;
				TAILQ_REMOVE(&region->ranges, item, chain);
				if (item != &region->range_0)
					sma_free(&vm_range_sma, item);
				*ret = addr;
				goto end;
			}
			if (addr == item->addr)
			{
				item->addr += size;
				item->size -= size;
				*ret = addr;
				goto end;
			}
			if (end == item->addr + item->size)
			{
				item->size -= size;
				*ret = addr;
				goto end;
			}
			struct vm_range *newr = sma_alloc(&vm_range_sma, 0);
			if (!newr)
				return -ENOMEM;
			newr->addr = end;
			newr->size = item->size - (end - item->addr);
			TAILQ_INSERT_AFTER(&region->ranges, item, newr, chain);
			item->size = addr - item->addr;
			*ret = addr;
			goto end;
		}
	}
	else
	{
		TAILQ_FOREACH(item, &region->ranges, chain)
		{
			if (item->size < size)
				continue;
			uintptr_t tmp = item->addr;
			uintptr_t pad = tmp & align_mask;
			if (pad)
			{
				pad = alignment - pad;
				if (item->size < size + pad)
					continue;
				if (item->size == size + pad)
				{
					item->size -= size;
					*ret = item->addr + pad;
					goto end;
				}
				struct vm_range *newr = sma_alloc(&vm_range_sma, 0);
				if (!newr)
					return -ENOMEM;
				newr->addr = item->addr;
				newr->size = pad;
				TAILQ_INSERT_BEFORE(item, newr, chain);
				item->addr += size + pad;
				item->size -= size + pad;
				*ret = newr->addr + pad;
				goto end;
			}
			*ret = item->addr;
			if (item->size == size)
			{
				TAILQ_REMOVE(&region->ranges, item, chain);
				if (item != &region->range_0)
					sma_free(&vm_range_sma, item);
			}
			else
			{
				item->addr += size;
				item->size -= size;
			}
			goto end;
		}
	}
	return -ENOMEM;

end:
	return 0;
}

int vm_region_free(struct vm_region *region, uintptr_t addr, size_t size)
{
	uintptr_t end;
	if (__builtin_add_overflow(addr, size, &end))
		return -EOVERFLOW;
	if (TAILQ_EMPTY(&region->ranges))
	{
		region->range_0.addr = addr;
		region->range_0.size = size;
		TAILQ_INSERT_HEAD(&region->ranges, &region->range_0, chain);
		goto end;
	}
	if (!range_cache)
	{
		range_cache = sma_alloc(&vm_range_sma, 0);
		if (!range_cache)
			return -ENOMEM;
	}
	struct vm_range *item;
	TAILQ_FOREACH(item, &region->ranges, chain)
	{
		if (item->addr == end)
		{
			item->addr -= size;
			item->size += size;
			struct vm_range *prev = TAILQ_PREV(item, vm_range_head, chain);
			if (!prev)
				goto end;
			if (prev->addr + prev->size != addr)
				goto end;
			prev->size += item->size;
			TAILQ_REMOVE(&region->ranges, item, chain);
			if (item != &region->range_0)
				sma_free(&vm_range_sma, item);
			goto end;
		}
		if (item->addr + item->size == addr)
		{
			item->size += size;
			struct vm_range *next = TAILQ_NEXT(item, chain);
			if (!next)
				goto end;
			if (next->addr != item->addr + item->size)
				goto end;
			item->size += next->size;
			TAILQ_REMOVE(&region->ranges, next, chain);
			if (next != &region->range_0)
				sma_free(&vm_range_sma, next);
			goto end;
		}
		if (addr < item->addr)
		{
			struct vm_range *newr = range_cache;
			range_cache = NULL;
			newr->addr = addr;
			newr->size = size;
			TAILQ_INSERT_BEFORE(item, newr, chain);
			goto end;
		}
	}
	struct vm_range *newr = range_cache;
	range_cache = NULL;
	newr->addr = addr;
	newr->size = size;
	TAILQ_INSERT_TAIL(&region->ranges, newr, chain);

end:
	return 0;
}

int vm_region_test(struct vm_region *region, uintptr_t addr, size_t size)
{
	struct vm_range *item;
	TAILQ_FOREACH(item, &region->ranges, chain)
	{
		/* early quit because of ranges ordering */
		if (addr + size <= item->addr)
			break;
		if (addr < item->addr + item->size)
			return 0;
	}
	return 1;
}

int vm_region_dup(struct vm_region *dst, const struct vm_region *src)
{
	dst->addr = src->addr;
	dst->size = src->size;
	TAILQ_INIT(&dst->ranges);
	struct vm_range *item;
	TAILQ_FOREACH(item, &src->ranges, chain)
	{
		struct vm_range *newr = sma_alloc(&vm_range_sma, 0);
		if (!newr)
			return -ENOMEM;
		newr->addr = item->addr;
		newr->size = item->size;
		TAILQ_INSERT_TAIL(&dst->ranges, newr, chain);
	}
	return 0;
}

void vm_region_init(struct vm_region *region, uintptr_t addr, uintptr_t size)
{
	region->addr = addr;
	region->size = size;
	TAILQ_INIT(&region->ranges);
}

void vm_region_destroy(struct vm_region *region)
{
	struct vm_range *range;
	while ((range = TAILQ_FIRST(&region->ranges)))
	{
		TAILQ_REMOVE(&region->ranges, range, chain);
		sma_free(&vm_range_sma, range);
	}
}
