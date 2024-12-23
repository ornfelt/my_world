#ifndef CACHE_H
#define CACHE_H

#include <jks/hmap.h>

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

struct font_model;
struct map_wmo;
struct map_m2;
struct gx_wmo;
struct gx_blp;
struct gx_m2;
struct cache;
struct dbc;

struct cache *cache_new(void);
void cache_delete(struct cache *cache);
void cache_print(struct cache *cache);

#define CACHE_FUNCTIONS(key_type, ref_type, name) \
bool cache_ref_##name(struct cache *cache, const key_type key, ref_type *ref); \
bool cache_ref_unmutexed_##name(struct cache *cache, const key_type key, ref_type *ref); \
void cache_unref_##name(struct cache *cache, const key_type key); \
void cache_unref_unmutexed_##name(struct cache *cache, const key_type key); \
void cache_lock_##name(struct cache *cache); \
void cache_unlock_##name(struct cache *cache); \

CACHE_FUNCTIONS(char*, struct gx_blp*, blp);
CACHE_FUNCTIONS(char*, struct gx_wmo*, wmo);
CACHE_FUNCTIONS(uint32_t, struct map_wmo*, map_wmo);
CACHE_FUNCTIONS(char*, struct gx_m2*, m2);
CACHE_FUNCTIONS(uint32_t, struct map_m2*, map_m2);
CACHE_FUNCTIONS(char*, struct font_model*, font);
CACHE_FUNCTIONS(char*, struct dbc*, dbc);

#undef CACHE_FUNCTIONS

#endif
