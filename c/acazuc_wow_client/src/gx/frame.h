#ifndef RENDER_FRAME_H
#define RENDER_FRAME_H

#ifdef WITH_DEBUG_RENDERING
# include "gx/collisions.h"
#endif

#include "gx/m2.h"

#include <jks/frustum.h>
#include <jks/array.h>
#include <jks/mat4.h>
#include <jks/vec3.h>

#include <gfx/objects.h>

#include <stdbool.h>
#include <pthread.h>

struct gx_m2_ground_batch;
struct gx_wmo_instance;
struct gx_m2_instance;
struct gx_wmo_mliq;
struct map_tile;
struct gx_mclq;
struct gx_mcnk;
struct gx_text;
struct gx_wmo;
struct camera;
struct gx_m2;

struct gx_frame_list
{
	struct jks_array entries;
	pthread_mutex_t mutex;
};

struct gx_frame_backrefs
{
	struct gx_frame_list wmo_mliq; /* struct gx_wmo_mliq* */
	struct gx_frame_list wmo; /* struct gx_wmo_instance* */
	struct gx_frame_list m2; /* struct gx_m2_instance* */
	struct gx_frame_list tiles; /* struct map_tile* */
};

struct gx_frame_render_lists
{
	struct gx_frame_list wmo_mliq[9]; /* struct gx_wmo_mliq* */
	struct gx_frame_list mclq[4]; /* struct gx_mclq* */
	struct gx_frame_list wmo; /* struct gx_wmo* */
	struct gx_frame_list m2_particles; /* struct gx_m2_instance* */
	struct gx_frame_list m2_ribbons; /* struct gx_m2_instance* */
	struct gx_frame_list m2_transparent; /* struct gx_m2_instance* */
	struct gx_frame_list m2_opaque; /* struct gx_m2* */
	struct gx_frame_list m2_shadow; /* struct gx_m2* */
	struct gx_frame_list m2_ground; /* struct gx_m2_ground_batch* */
	struct gx_frame_list m2; /* struct gx_m2* */
	struct gx_frame_list mcnk; /* struct gx_mcnk* */
	struct gx_frame_list text; /* struct gx_text* */
	struct gx_frame_list worldobj; /* struct worldobj* */
#ifdef WITH_DEBUG_RENDERING
	struct
	{
		struct gx_frame_list aabbs; /* struct gx_aabb* */
		struct gx_aabb_batch batch;
	} aabb[4];
#endif
};

struct gx_frame
{
	struct gx_frame_render_lists render_lists;
	struct gx_frame_backrefs backrefs;
#ifdef WITH_DEBUG_RENDERING
	struct gx_collisions gx_collisions;
#endif
	gfx_buffer_t m2_ground_shadow_uniform_buffer;
	gfx_buffer_t m2_ground_light_uniform_buffer;
	gfx_buffer_t m2_world_uniform_buffer;
	gfx_buffer_t particle_uniform_buffer;
	gfx_buffer_t ribbon_uniform_buffer;
	gfx_buffer_t river_uniform_buffer;
	gfx_buffer_t ocean_uniform_buffer;
	gfx_buffer_t magma_uniform_buffer;
	gfx_buffer_t mcnk_uniform_buffer;
	gfx_buffer_t mliq_uniform_buffer;
	gfx_buffer_t wmo_uniform_buffer;
	struct gx_m2_render_params m2_params;
	enum gx_m2_lighting_type m2_lighting_type;
	struct frustum shadow_frustum;
	struct frustum wdl_frustum;
	struct frustum frustum;
	struct mat4f view_shadow_vp;
	struct mat4f view_shadow_p;
	struct mat4f view_shadow_v;
	struct mat4f view_wdl_vp;
	struct mat4f view_wdl_p;
	struct vec3f view_pos;
	struct vec3f view_rot;
	struct mat4f view_vp;
	struct mat4f view_v;
	struct mat4f view_p;
	struct vec4f view_bottom;
	struct vec4f view_right;
	struct mat4f cull_wdl_vp;
	struct mat4f cull_wdl_p;
	struct vec3f cull_pos;
	struct vec3f cull_rot;
	struct mat4f cull_vp;
	struct mat4f cull_v;
	struct mat4f cull_p;
	float view_distance;
	float fov;
	int64_t time;
	int64_t dt;
	int id;
};

void gx_frame_init(struct gx_frame *gx_frame, int id);
void gx_frame_destroy(struct gx_frame *gx_frame);
void gx_frame_build_uniform_buffers(struct gx_frame *gx_frame);
void gx_frame_copy_cameras(struct gx_frame *gx_frame, struct camera *cull_camera, struct camera *view_camera);
void gx_frame_clear_scene(struct gx_frame *gx_frame);
void gx_frame_release_obj(struct gx_frame *gx_frame);
void gx_frame_add_mcnk(struct gx_frame *frame, struct gx_mcnk *mcnk);
void gx_frame_add_mclq(struct gx_frame *frame, uint8_t type, struct gx_mclq *mclq);
void gx_frame_add_wmo(struct gx_frame *frame, struct gx_wmo *wmo);
void gx_frame_add_wmo_backref(struct gx_frame *frame, struct gx_wmo_instance *instance);
void gx_frame_add_wmo_mliq(struct gx_frame *frame, struct gx_wmo_mliq *mliq, uint8_t liquid);
void gx_frame_add_wmo_mliq_backref(struct gx_frame *frame, struct gx_wmo_mliq *mliq);
void gx_frame_add_m2(struct gx_frame *frame, struct gx_m2 *m2);
void gx_frame_add_m2_particles(struct gx_frame *frame, struct gx_m2_instance *instance);
void gx_frame_add_m2_ribbons(struct gx_frame *frame, struct gx_m2_instance *instance);
void gx_frame_add_m2_transparent(struct gx_frame *frame, struct gx_m2_instance *instance);
void gx_frame_add_m2_opaque(struct gx_frame *frame, struct gx_m2 *m2);
void gx_frame_add_m2_shadow(struct gx_frame *frame, struct gx_m2 *m2);
void gx_frame_add_m2_backref(struct gx_frame *frame, struct gx_m2_instance *instance);
void gx_frame_add_m2_ground(struct gx_frame *frame, struct gx_m2_ground_batch *batch);
void gx_frame_add_text(struct gx_frame *frame, struct gx_text *text);
void gx_frame_add_tile(struct gx_frame *frame, struct map_tile *tile);
#ifdef WITH_DEBUG_RENDERING
void gx_frame_add_aabb(struct gx_frame *frame, struct gx_aabb *aabb);
#endif

#endif
