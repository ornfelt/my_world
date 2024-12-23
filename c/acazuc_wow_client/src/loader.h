#ifndef LOADER_H
#define LOADER_H

#include <stdbool.h>

struct wow_mpq_compound;
struct loader;

enum async_task_type /* priority ordered */
{
	ASYNC_TASK_BLP_LOAD,
	ASYNC_TASK_BLP_UNLOAD,
	ASYNC_TASK_MAP_TILE_LOAD,
	ASYNC_TASK_MAP_TILE_UNLOAD,
	ASYNC_TASK_WMO_GROUP_LOAD,
	ASYNC_TASK_WMO_GROUP_UNLOAD,
	ASYNC_TASK_WMO_LOAD,
	ASYNC_TASK_WMO_UNLOAD,
	ASYNC_TASK_M2_LOAD,
	ASYNC_TASK_M2_UNLOAD,
	ASYNC_TASK_SKIN_TEXTURE,
	ASYNC_TASK_MINIMAP_TEXTURE,
	ASYNC_TASK_CLOUDS_TEXTURE,
	ASYNC_TASK_M2_INSTANCE_DELETE,
	ASYNC_TASK_WMO_INSTANCE_DELETE,
	ASYNC_TASK_TEXT_DELETE,
	ASYNC_TASK_WDB_SAVE,
	ASYNC_TASK_LAST
};

enum loader_object_type /* priority ordered */
{
	LOADER_MAP_TILE,
	LOADER_BLP,
	LOADER_M2,
	LOADER_WMO,
	LOADER_WMO_GROUP,
	LOADER_LAST,
};

typedef void (*loader_load_fn_t)(struct wow_mpq_compound *compound, void *userdata);
typedef bool (*loader_init_fn_t)(void *userdata);

struct loader *loader_new(void);
void loader_delete(struct loader *loader);
bool loader_has_async(struct loader *loader);
bool loader_has_loading(struct loader *loader);
void loader_tick(struct loader *loader);
void loader_push(struct loader *loader, enum async_task_type type, loader_load_fn_t fn, void *userdata);
void loader_init_object(struct loader *loader, enum loader_object_type type, loader_init_fn_t fn, void *userdata);
void loader_start_cull(struct loader *loader);
void loader_wait_cull(struct loader *loader);

#endif
