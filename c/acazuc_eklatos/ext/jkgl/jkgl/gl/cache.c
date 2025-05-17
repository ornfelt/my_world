#include "internal.h"

#include <assert.h>

void
cache_init(struct cache *cache, const struct cache_op *op)
{
	cache->op = op;
	for (GLuint i = 0; i < sizeof(cache->hash_head) / sizeof(*cache->hash_head); ++i)
		TAILQ_INIT(&cache->hash_head[i]);
	TAILQ_INIT(&cache->fifo_head);
	cache->hash_count = 0;
	cache->fifo_count = 0;
}

static struct cache_entry_head *
cache_entry_head(struct cache *cache, uint32_t hash)
{
	size_t size;

	size = sizeof(cache->hash_head) / sizeof(*cache->hash_head);
	return &cache->hash_head[hash % size];
}

struct cache_entry *
cache_get(struct gl_ctx *ctx,
          struct cache *cache,
          const void *state)
{
	struct cache_entry_head *head;
	struct cache_entry *entry;
	uint32_t hash;

	hash = cache->op->hash(ctx, state);
	head = cache_entry_head(cache, hash);
	TAILQ_FOREACH(entry, head, hash_chain)
	{
		if (entry->hash != hash)
			continue;
		if (cache->op->eq(ctx, entry, state))
			break;
	}
	if (entry)
	{
		if (!entry->ref)
		{
			cache->fifo_count--;
			TAILQ_REMOVE(&cache->fifo_head, entry, fifo_chain);
		}
	}
	else
	{
		entry = cache->op->alloc(ctx, hash, state);
		if (!entry)
			return NULL;
		cache->hash_count++;
		TAILQ_INSERT_TAIL(head, entry, hash_chain);
	}
	entry->ref++;
	return entry;
}

void
cache_unref(struct gl_ctx *ctx,
            struct cache *cache,
            struct cache_entry *entry)
{
	assert(entry->ref);
	if (--entry->ref)
		return;
	TAILQ_INSERT_HEAD(&cache->fifo_head, entry, fifo_chain);
	if (cache->fifo_count < 1024) /* XXX better threshold */
	{
		cache->fifo_count++;
		return;
	}
	entry = TAILQ_LAST(&cache->fifo_head, cache_entry_head);
	TAILQ_REMOVE(&cache->fifo_head, entry, fifo_chain);
	TAILQ_REMOVE(cache_entry_head(cache, entry->hash), entry, hash_chain);
	cache->op->free(ctx, entry);
}
