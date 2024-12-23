#include "cache.h"

#include "font/model.h"

#include "gx/wmo.h"
#include "gx/blp.h"
#include "gx/m2.h"

#include "map/tile.h"

#include "memory.h"
#include "log.h"
#include "wow.h"
#include "dbc.h"

#include <wow/mpq.h>
#include <wow/dbc.h>

#include <inttypes.h>
#include <string.h>
#include <assert.h>

MEMORY_DECL(GENERIC);

typedef bool (*cache_constructor_t)(void *key, void **ref);
typedef bool (*cache_key_dup_t)(const void *key, void **d);
typedef void (*cache_ref_t)(void *ref);

struct cache_def
{
	const char *name;
	cache_constructor_t constructor;
	cache_ref_t ref;
	int string_key;
};

struct cache_type
{
	const struct cache_def *def;
	struct jks_hmap hmap; /* key, val */
	pthread_mutex_t mutex;
};

struct cache
{
	struct cache_type blp; /* char*, struct blp* */
	struct cache_type wmo; /* char*, struct gx_wmo* */
	struct cache_type map_wmo; /* uint32_t, struct map_wmo* */
	struct cache_type m2; /* char*, struct gx_m2* */
	struct cache_type map_m2; /* uint32_t. struct map_m2* */
	struct cache_type font; /* char*, struct font_model* */
	struct cache_type dbc; /* char*, struct dbc* */
};

static bool blp_constructor(void *key, void **ref)
{
	*ref = gx_blp_from_filename(key);
	return *ref != NULL;
}

static bool wmo_constructor(void *key, void **ref)
{
	*ref = gx_wmo_new(key);
	return *ref != NULL;
}

static bool map_wmo_constructor(void *key, void **ref)
{
	*ref = map_wmo_new((uint32_t)(uintptr_t)key);
	return *ref != NULL;
}

static bool m2_constructor(void *key, void **ref)
{
	*ref = gx_m2_new(key);
	return *ref != NULL;
}

static bool map_m2_constructor(void *key, void **ref)
{
	*ref = map_m2_new((uint32_t)(uintptr_t)key);
	return *ref != NULL;
}

static bool font_constructor(void *key, void **ref)
{
	struct wow_mpq_file *file = wow_mpq_get_file(g_wow->mpq_compound, key);
	if (!file)
		return false;
	*ref = font_model_new(key, (const char*)file->data, file->size);
	wow_mpq_file_delete(file);
	return *ref != NULL;
}

static bool dbc_constructor(void *key, void **ref)
{
	struct wow_mpq_file *mpq_file = wow_mpq_get_file(g_wow->mpq_compound, key);
	if (!mpq_file)
		return false;
	struct wow_dbc_file *dbc_file = wow_dbc_file_new(mpq_file);
	wow_mpq_file_delete(mpq_file);
	if (!dbc_file)
		return false;
	*ref = dbc_new(key, dbc_file);
	if (!*ref)
		wow_dbc_file_delete(dbc_file);
	return *ref != NULL;
}

static const struct cache_def blp_def =
{
	.name = "blp",
	.constructor = blp_constructor,
	.ref         = (void*)gx_blp_ref,
	.string_key  = 1,
};

static const struct cache_def wmo_def =
{
	.name = "wmo",
	.constructor = wmo_constructor,
	.ref         = (void*)gx_wmo_ref,
	.string_key  = 1,
};

static const struct cache_def map_wmo_def =
{
	.name = "map_wmo",
	.constructor = map_wmo_constructor,
	.ref         = (void*)map_wmo_ref,
	.string_key  = 0,
};

static const struct cache_def m2_def =
{
	.name = "m2",
	.constructor = m2_constructor,
	.ref         = (void*)gx_m2_ref,
	.string_key  = 1,
};

static const struct cache_def map_m2_def =
{
	.name = "map_m2",
	.constructor = map_m2_constructor,
	.ref         = (void*)map_m2_ref,
	.string_key  = 0,
};

static const struct cache_def font_def =
{
	.name = "font",
	.constructor = font_constructor,
	.ref         = (void*)font_model_ref,
	.string_key  = 1,
};

static const struct cache_def dbc_def =
{
	.name = "dbc",
	.constructor = dbc_constructor,
	.ref         = (void*)dbc_ref,
	.string_key  = 1,
};

static void cache_type_init(struct cache_type *cache, jks_hmap_destructor_t destructor, jks_hmap_hash_fn_t hash_fn, jks_hmap_cmp_fn_t cmp_fn, const struct cache_def *def)
{
	jks_hmap_init(&cache->hmap, sizeof(void*), destructor, hash_fn, cmp_fn, &jks_hmap_memory_fn_GENERIC);
	cache->def = def;
	pthread_mutex_init(&cache->mutex, NULL);
}

struct cache *cache_new(void)
{
	struct cache *cache = mem_malloc(MEM_GENERIC, sizeof(*cache));
	if (!cache)
		return NULL;
	cache_type_init(&cache->blp, NULL, jks_hmap_hash_str, jks_hmap_cmp_str, &blp_def);
	cache_type_init(&cache->wmo, NULL, jks_hmap_hash_str, jks_hmap_cmp_str, &wmo_def);
	cache_type_init(&cache->map_wmo, NULL, jks_hmap_hash_u32, jks_hmap_cmp_u32, &map_wmo_def);
	cache_type_init(&cache->m2, NULL, jks_hmap_hash_str, jks_hmap_cmp_str, &m2_def);
	cache_type_init(&cache->map_m2, NULL, jks_hmap_hash_u32, jks_hmap_cmp_u32, &map_m2_def);
	cache_type_init(&cache->font, NULL, jks_hmap_hash_str, jks_hmap_cmp_str, &font_def);
	cache_type_init(&cache->dbc, NULL, jks_hmap_hash_str, jks_hmap_cmp_str, &dbc_def);
	return cache;
}

static void cache_type_destroy(struct cache_type *cache)
{
	jks_hmap_destroy(&cache->hmap);
	pthread_mutex_destroy(&cache->mutex);
}

void cache_delete(struct cache *cache)
{
	if (!cache)
		return;
	cache_type_destroy(&cache->blp);
	cache_type_destroy(&cache->wmo);
	cache_type_destroy(&cache->map_wmo);
	cache_type_destroy(&cache->m2);
	cache_type_destroy(&cache->map_m2);
	cache_type_destroy(&cache->font);
	cache_type_destroy(&cache->dbc);
	mem_free(MEM_GENERIC, cache);
}

void cache_print(struct cache *cache)
{
#define PRINT(type) \
	LOG_INFO(#type "s: %"  PRIu32, (uint32_t)cache->type.hmap.size);

	PRINT(blp);
	PRINT(wmo);
	PRINT(map_wmo);
	PRINT(m2);
	PRINT(map_m2);
	PRINT(font);
	PRINT(dbc);

#undef PRINT
}

static bool cache_ref(struct cache_type *cache, const void *key, void **ref)
{
	assert(ref);
	void **value = jks_hmap_get(&cache->hmap, JKS_HMAP_KEY_PTR((void*)key));
	if (value)
	{
		cache->def->ref(*value);
		*ref = *value;
		return true;
	}
	void *key_dup;
	if (cache->def->string_key)
	{
		key_dup = mem_strdup(MEM_GENERIC, key);
		if (!key_dup)
			return false;
	}
	else
	{
		key_dup = (void*)key;
	}
	if (!cache->def->constructor(key_dup, ref))
	{
		if (cache->def->string_key)
			mem_free(MEM_GENERIC, key_dup);
		return false;
	}
	if (!jks_hmap_set(&cache->hmap, JKS_HMAP_KEY_PTR(key_dup), ref))
	{
		if (cache->def->string_key)
			mem_free(MEM_GENERIC, key_dup);
		return false;
	}
	return true;
}

static void cache_unref(struct cache_type *cache, const void *key)
{
	if (!jks_hmap_erase(&cache->hmap, JKS_HMAP_KEY_PTR((void*)key)))
	{
		assert(!"unref unexisting key");
		return;
	}
}

#define CACHE_FUNCTIONS(key_type, ref_type, name) \
bool cache_ref_##name(struct cache *cache, const key_type key, ref_type *ref) \
{ \
	cache_lock_##name(cache); \
	ref_type ret = (ref_type)(intptr_t)cache_ref(&cache->name, (const void*)(intptr_t)key, (void**)ref); \
	cache_unlock_##name(cache); \
	return ret; \
} \
void cache_unref_##name(struct cache *cache, const key_type key) \
{ \
	cache_lock_##name(cache); \
	cache_unref(&cache->name, (const void*)(intptr_t)key); \
	cache_unlock_##name(cache); \
} \
bool cache_ref_unmutexed_##name(struct cache *cache, const key_type key, ref_type *ref) \
{ \
	return (ref_type)(intptr_t)cache_ref(&cache->name, (const void*)(intptr_t)key, (void**)ref); \
} \
void cache_unref_unmutexed_##name(struct cache *cache, const key_type key) \
{ \
	cache_unref(&cache->name, (void*)(intptr_t)key); \
} \
void cache_lock_##name(struct cache *cache) \
{ \
	pthread_mutex_lock(&cache->name.mutex); \
} \
void cache_unlock_##name(struct cache *cache) \
{ \
	pthread_mutex_unlock(&cache->name.mutex); \
}

CACHE_FUNCTIONS(char*, struct gx_blp*, blp);
CACHE_FUNCTIONS(char*, struct gx_wmo*, wmo);
CACHE_FUNCTIONS(uint32_t, struct map_wmo*, map_wmo);
CACHE_FUNCTIONS(char*, struct gx_m2*, m2);
CACHE_FUNCTIONS(uint32_t, struct map_m2*, map_m2);
CACHE_FUNCTIONS(char*, struct font_model*, font);
CACHE_FUNCTIONS(char*, struct dbc*, dbc);
