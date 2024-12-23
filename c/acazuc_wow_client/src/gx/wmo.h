#ifndef GX_WMO_H
#define GX_WMO_H

#ifdef WITH_DEBUG_RENDERING
# include "gx/wmo_portals.h"
# include "gx/wmo_lights.h"
# include "gx/aabb.h"
#endif

#include "refcount.h"

#include <jks/frustum.h>
#include <jks/array.h>
#include <jks/vec4.h>
#include <jks/aabb.h>

#include <wow/wmo.h>

#include <gfx/objects.h>

#include <stdbool.h>

struct gx_wmo_instance;
struct gx_wmo_group;
struct gx_frame;

struct gx_wmo_frame
{
	struct jks_array to_render; /* struct gx_wmo_instance* */
	pthread_mutex_t mutex;
};

enum gx_wmo_flag
{
	GX_WMO_FLAG_LOAD_ASKED     = (1 << 0),
	GX_WMO_FLAG_LOADED         = (1 << 1),
	GX_WMO_FLAG_INITIALIZED    = (1 << 2),
	GX_WMO_FLAG_IN_RENDER_LIST = (1 << 3),
};

struct gx_wmo
{
	struct gx_wmo_frame frames[RENDER_FRAMES_COUNT];
	struct jks_array instances; /* struct gx_wmo_instance* */
	struct jks_array groups; /* struct gx_wmo_group* */
	struct jks_array modd; /* struct wow_modd_data */
	struct jks_array mods; /* struct wow_mods_data */
	struct jks_array molt; /* struct wow_molt_data */
	struct jks_array mopt; /* struct wow_mopt_data */
	struct jks_array mopr; /* struct wow_mopr_data */
	struct jks_array momt; /* struct wow_momt_data */
	struct jks_array mopv; /* struct wow_vec3f */
	struct jks_array modn; /* char */
	struct jks_array motx; /* char */
#ifdef WITH_DEBUG_RENDERING
	struct gx_wmo_portals gx_portals;
	struct gx_wmo_lights gx_lights;
#endif
	enum gx_wmo_flag flags;
	char *filename;
	struct vec4f ambient;
	struct aabb aabb;
	uint32_t wow_flags;
	refcount_t refcount;
};

struct gx_wmo *gx_wmo_new(char *filename);
void gx_wmo_free(struct gx_wmo *wmo);
void gx_wmo_ref(struct gx_wmo *wmo);
void gx_wmo_ask_load(struct gx_wmo *wmo);
void gx_wmo_clear_update(struct gx_wmo *wmo, struct gx_frame *frame);
void gx_wmo_render(struct gx_wmo *wmo, struct gx_frame *frame);
#ifdef WITH_DEBUG_RENDERING
void gx_wmo_render_portals(struct gx_wmo *wmo, struct gx_frame *frame);
void gx_wmo_render_lights(struct gx_wmo *wmo, struct gx_frame *frame);
void gx_wmo_render_collisions(struct gx_wmo *wmo, struct gx_frame *frame, bool triangles);
#endif

static inline bool gx_wmo_flag_get(struct gx_wmo *wmo, enum gx_wmo_flag flag)
{
	return (__atomic_load_n(&wmo->flags, __ATOMIC_RELAXED) & flag) == flag;
}

static inline bool gx_wmo_flag_set(struct gx_wmo *wmo, enum gx_wmo_flag flag)
{
	return (__atomic_fetch_or(&wmo->flags, flag, __ATOMIC_SEQ_CST) & flag) == flag;
}

static inline bool gx_wmo_flag_clear(struct gx_wmo *wmo, enum gx_wmo_flag flag)
{
	return (__atomic_fetch_and(&wmo->flags, ~flag, __ATOMIC_RELAXED) & flag) == flag;
}

struct gx_wmo_batch_instance_frame
{
	bool culled;
};

struct gx_wmo_batch_instance
{
	struct gx_wmo_batch_instance_frame frames[RENDER_FRAMES_COUNT];
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb gx_aabb;
#endif
	struct aabb aabb;
};

void gx_wmo_batch_instance_init(struct gx_wmo_batch_instance *instance);
void gx_wmo_batch_instance_destroy(struct gx_wmo_batch_instance *instance);
#ifdef WITH_DEBUG_RENDERING
void gx_wmo_batch_instance_render_aabb(struct gx_wmo_batch_instance *instance, const struct mat4f *mvp);
#endif

struct gx_wmo_group_instance_frame
{
	bool cull_source;
	bool culled;
};

struct gx_wmo_group_instance
{
	struct gx_wmo_group_instance_frame frames[RENDER_FRAMES_COUNT];
	struct jks_array batches; /* struct gx_wmo_batch_instance */
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb gx_aabb;
#endif
	struct aabb aabb;
};

void gx_wmo_group_instance_init(struct gx_wmo_group_instance *instance);
void gx_wmo_group_instance_destroy(struct gx_wmo_group_instance *instance);
bool gx_wmo_group_instance_on_load(struct gx_wmo_instance *instance, struct gx_wmo_group *group, struct gx_wmo_group_instance *group_instance);

struct gx_wmo_instance_frame
{
	gfx_buffer_t uniform_buffer;
	struct mat4f mvp;
	struct mat4f mv;
	float distance_to_camera;
	bool culled;
};

enum gx_wmo_instance_flag
{
	GX_WMO_INSTANCE_FLAG_IN_RENDER_LIST = (1 << 0),
};

struct gx_wmo_instance
{
	struct gx_wmo_instance_frame frames[RENDER_FRAMES_COUNT];
	struct jks_array groups; /* struct gx_wmo_group_instance */
	struct jks_array m2; /* struct gx_m2_instance* */
	uint8_t *traversed_portals; /* bitmask */
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb gx_aabb;
#endif
	struct gx_wmo *parent;
	struct frustum frustum;
	struct aabb aabb;
	struct mat4f m_inv;
	struct mat4f m;
	struct vec3f pos;
	uint16_t doodad_start;
	uint16_t doodad_end;
	uint16_t doodad_set;
	enum gx_wmo_instance_flag flags;
	refcount_t refcount;
};

static inline bool gx_wmo_instance_flag_get(struct gx_wmo_instance *instance, enum gx_wmo_instance_flag flag)
{
	return (__atomic_load_n(&instance->flags, __ATOMIC_RELAXED) & flag) == flag;
}

static inline bool gx_wmo_instance_flag_set(struct gx_wmo_instance *instance, enum gx_wmo_instance_flag flag)
{
	return (__atomic_fetch_or(&instance->flags, flag, __ATOMIC_SEQ_CST) & flag) == flag;
}

static inline bool gx_wmo_instance_flag_clear(struct gx_wmo_instance *instance, enum gx_wmo_instance_flag flag)
{
	return (__atomic_fetch_and(&instance->flags, ~flag, __ATOMIC_RELAXED) & flag) == flag;
}

struct gx_wmo_instance *gx_wmo_instance_new(const char *filename);
void gx_wmo_instance_free(struct gx_wmo_instance *instance);
void gx_wmo_instance_ref(struct gx_wmo_instance *instance);
void gx_wmo_instance_on_load(struct gx_wmo_instance *instance);
void gx_wmo_instance_load_doodad_set(struct gx_wmo_instance *instance);
void gx_wmo_instance_update(struct gx_wmo_instance *instance, struct gx_frame *frame, bool bypass_frustum);
void gx_wmo_instance_update_aabb(struct gx_wmo_instance *instance);
void gx_wmo_instance_add_to_render(struct gx_wmo_instance *instance, struct gx_frame *frame, bool bypass_frustum);
void gx_wmo_instance_calculate_distance_to_camera(struct gx_wmo_instance *instance, struct gx_frame *frame);
void gx_wmo_instance_set_mat(struct gx_wmo_instance *instance, const struct mat4f *mat);

#endif
