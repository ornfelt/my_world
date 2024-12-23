#ifndef GX_MCLQ_H
#define GX_MCLQ_H

#ifdef WITH_DEBUG_RENDERING
# include "gx/aabb.h"
#endif

#include <jks/aabb.h>
#include <jks/vec3.h>
#include <jks/vec2.h>

#include <gfx/objects.h>

#include <stdbool.h>

#define GX_MCLQ_LIQUIDS_NB 4

struct gx_mclq_init_data;
struct wow_adt_file;
struct map_tile;

enum gx_mclq_flag
{
	GX_MCLQ_FLAG_IN_RENDER_LIST = (1 << 0),
	GX_MCLQ_FLAG_INITIALIZED    = (1 << 1),
};

struct gx_mclq_batch_frame
{
	bool culled;
};

struct gx_mclq_batch
{
	struct gx_mclq_batch_frame frames[RENDER_FRAMES_COUNT];
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb gx_aabb;
#endif
	uint32_t indices_offset;
	uint16_t indices_nb;
	struct vec3f center;
	struct aabb aabb;
};

struct gx_mclq_liquid
{
	struct gx_mclq_init_data *init_data;
	struct gx_mclq_batch *batches;
	uint32_t batches_nb;
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t uniform_buffers[RENDER_FRAMES_COUNT];
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
	gfx_buffer_t depths_buffer;
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb gx_aabb;
#endif
	struct aabb aabb;
	enum gx_mclq_flag flags;
};

struct gx_mclq_frame
{
	struct mat4f mvp;
	struct mat4f mv;
};

struct gx_mclq
{
	struct map_tile *parent;
	struct gx_mclq_frame frames[RENDER_FRAMES_COUNT];
	struct gx_mclq_liquid liquids[GX_MCLQ_LIQUIDS_NB];
	struct vec3f pos;
	struct mat4f m;
	uint8_t type;
	enum gx_mclq_flag flags;
};

struct gx_mclq *gx_mclq_new(struct map_tile *parent, struct wow_adt_file *file);
void gx_mclq_delete(struct gx_mclq *mclq);
int gx_mclq_initialize(struct gx_mclq *mclq);
void gx_mclq_cull(struct gx_mclq *mclq, struct gx_frame *frame);
void gx_mclq_render(struct gx_mclq *mclq, struct gx_frame *frame, uint8_t type);

static inline bool gx_mclq_flag_get(struct gx_mclq *mclq, enum gx_mclq_flag flag)
{
	return (__atomic_load_n(&mclq->flags, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool gx_mclq_flag_set(struct gx_mclq *mclq, enum gx_mclq_flag flag)
{
	return (__atomic_fetch_or(&mclq->flags, flag, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool gx_mclq_flag_clear(struct gx_mclq *mclq, enum gx_mclq_flag flag)
{
	return (__atomic_fetch_and(&mclq->flags, ~flag, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool gx_mclq_liquid_flag_set(struct gx_mclq_liquid *liquid, enum gx_mclq_flag flag)
{
	return (__atomic_fetch_or(&liquid->flags, flag, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool gx_mclq_liquid_flag_clear(struct gx_mclq_liquid *liquid, enum gx_mclq_flag flag)
{
	return (__atomic_fetch_and(&liquid->flags, ~flag, __ATOMIC_RELAXED) & flag) != 0;
}

#endif
