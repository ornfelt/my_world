#ifndef GX_M2_H
#define GX_M2_H

#ifdef WITH_DEBUG_RENDERING
# include "gx/m2_collisions.h"
# include "gx/m2_lights.h"
# include "gx/m2_bones.h"
# include "gx/aabb.h"
#endif

#include "refcount.h"

#include <jks/array.h>
#include <jks/aabb.h>
#include <jks/vec2.h>

#include <gfx/objects.h>

#include <stdbool.h>
#include <pthread.h>

struct gx_m2_particles;
struct gx_m2_instance;
struct gx_m2_ribbons;
struct gx_m2_texture;
struct gx_m2_profile;
struct wow_m2_track;
struct wow_m2_file;
struct gx_m2_batch;
struct gx_frame;
struct gx_blp;
struct gx_m2;

struct gx_m2_render_params
{
	struct mat4f shadow_vp;
	struct mat4f shadow_p;
	struct mat4f shadow_v;
	struct mat4f vp;
	struct mat4f v;
	struct mat4f p;
	struct vec3f fog_color;
	struct vec3f view_right;
	struct vec3f view_bottom;
};

struct gx_m2_texture
{
	struct gx_blp *texture;
	uint32_t transform;
	uint32_t wow_flags;
	uint8_t type;
	bool has_transform;
	bool initialized;
};

struct gx_m2_batch
{
	struct gx_m2_profile *parent;
	uint16_t combiners[2];
	uint32_t pipeline_state;
	gfx_buffer_t uniform_buffers[RENDER_FRAMES_COUNT];
	struct gx_m2_texture textures[2];
	struct vec3f fog_color;
	uint16_t skin_section_id;
	uint16_t color_transform;
	uint16_t material;
	uint16_t texture_weight;
	uint32_t indices_offset;
	uint32_t indices_nb;
	uint8_t priority_plane;
	uint8_t wow_flags;
	uint8_t id;
	float alpha_test;
	bool has_color_transform;
	bool has_texture_weight;
	bool fog_override;
	bool blending;
};

struct gx_m2_profile
{
	struct jks_array batches; /* struct gx_m2_batch */
	struct jks_array transparent_batches; /* uint8_t */
	struct jks_array opaque_batches; /* uint8_t */
	struct gx_m2 *parent;
	bool initialized;
};

struct gx_m2_frame
{
	struct jks_array shadow_to_render; /* struct gx_m2_instance* */
	struct jks_array to_render; /* struct gx_m2_instance* */
	bool updated;
	pthread_mutex_t mutex;
};

enum gx_m2_flag
{
	GX_M2_FLAG_LOAD_ASKED            = (1 << 0),
	GX_M2_FLAG_LOADED                = (1 << 1),
	GX_M2_FLAG_SKYBOX                = (1 << 2),
	GX_M2_FLAG_HAS_TRANS_BATCHES     = (1 << 3),
	GX_M2_FLAG_HAS_BILLBOARD_BONES   = (1 << 4),
	GX_M2_FLAG_HAS_OPAQUE_BATCHES    = (1 << 5),
	GX_M2_FLAG_IN_RENDER_LIST        = (1 << 6),
	GX_M2_FLAG_INITIALIZED           = (1 << 7),
	GX_M2_FLAG_IN_SHADOW_RENDER_LIST = (1 << 8),
};

struct gx_m2
{
	struct gx_m2_frame frames[RENDER_FRAMES_COUNT];
	struct jks_array instances; /* struct gx_m2_instance */
	struct jks_array profiles; /* struct gx_m2_profile */
	struct wow_m2_playable_animation *playable_animations;
	struct wow_m2_texture_transform *texture_transforms;
	struct wow_m2_texture_weight *texture_weights;
	struct wow_m2_attachment *attachments;
	struct wow_m2_sequence *sequences;
	struct wow_m2_particle *particles;
	struct wow_m2_material *materials;
	struct wow_m2_texture *textures;
	struct wow_m2_vertex *vertexes;
	struct wow_m2_ribbon *ribbons;
	struct wow_m2_camera *cameras;
	struct wow_m2_color *colors;
	struct wow_m2_light *lights;
	struct wow_m2_bone *bones;
	struct wow_vec3f *collision_vertexes;
	struct wow_vec3f *collision_normals;
	uint16_t *collision_triangles;
	uint16_t *attachment_lookups;
	uint16_t *key_bone_lookups;
	uint16_t *sequence_lookups;
	uint32_t *global_sequences;
	uint16_t *bone_lookups;
	uint32_t playable_animations_nb;
	uint32_t texture_transforms_nb;
	uint32_t texture_weights_nb;
	uint32_t attachments_nb;
	uint32_t sequences_nb;
	uint32_t particles_nb;
	uint32_t materials_nb;
	uint32_t textures_nb;
	uint32_t vertexes_nb;
	uint32_t ribbons_nb;
	uint32_t cameras_nb;
	uint32_t colors_nb;
	uint32_t lights_nb;
	uint32_t bones_nb;
	uint32_t collision_vertexes_nb;
	uint32_t collision_normals_nb;
	uint32_t collision_triangles_nb;
	uint32_t attachment_lookups_nb;
	uint32_t key_bone_lookups_nb;
	uint32_t sequence_lookups_nb;
	uint32_t global_sequences_nb;
	uint32_t bone_lookups_nb;
	struct jks_array indices; /* uint16_t */
	enum gx_m2_flag flags;
	gfx_buffer_t vertexes_buffer;
	gfx_buffer_t indices_buffer;
#ifdef WITH_DEBUG_RENDERING
	struct gx_m2_collisions gx_collisions;
	struct gx_m2_lights gx_lights;
	struct gx_m2_bones gx_bones;
#endif
	gfx_attributes_state_t attributes_state;
	char *filename;
	struct aabb caabb;
	struct aabb aabb;
	float collision_sphere_radius;
	uint32_t wow_flags;
	uint32_t version;
	float render_distance;
	refcount_t refcount;
};

struct gx_m2 *gx_m2_new(char *filename);
void gx_m2_free(struct gx_m2 *m2);
void gx_m2_ref(struct gx_m2 *m2);
void gx_m2_ask_load(struct gx_m2 *m2);
void gx_m2_render(struct gx_m2 *m2, struct gx_frame *frame, bool transparent);
#ifdef WITH_DEBUG_RENDERING
void gx_m2_render_bones_points(struct gx_m2 *m2, struct gx_frame *frame);
void gx_m2_render_bones_lines(struct gx_m2 *m2, struct gx_frame *frame);
void gx_m2_render_lights(struct gx_m2 *m2, struct gx_frame *frame);
void gx_m2_render_collisions(struct gx_m2 *m2, struct gx_frame *frame, bool triangles);
#endif
void gx_m2_clear_update(struct gx_m2 *m2, struct gx_frame *frame);

static inline bool gx_m2_flag_get(struct gx_m2 *m2, enum gx_m2_flag flag)
{
	return (__atomic_load_n(&m2->flags, __ATOMIC_RELAXED) & flag) == flag;
}

static inline bool gx_m2_flag_set(struct gx_m2 *m2, enum gx_m2_flag flag)
{
	return (__atomic_fetch_or(&m2->flags, flag, __ATOMIC_SEQ_CST) & flag) == flag;
}

static inline bool gx_m2_flag_clear(struct gx_m2 *m2, enum gx_m2_flag flag)
{
	return (__atomic_fetch_and(&m2->flags, ~flag, __ATOMIC_RELAXED) & flag) == flag;
}

struct gx_m2_light
{
	struct vec3f position;
	struct vec3f color;
	struct vec2f attenuation;
};

struct gx_m2_lighting
{
	struct vec4f light_direction;
	struct vec4f ambient_color;
	struct vec4f diffuse_color;
	struct gx_m2_light lights[4];
	uint32_t lights_count;
	gfx_buffer_t uniform_buffer;
};

enum gx_m2_lighting_type
{
	GX_M2_LIGHTING_WORLD,
	GX_M2_LIGHTING_WMO_INDOOR,
	GX_M2_LIGHTING_WMO_OUTDOOR,
	GX_M2_LIGHTING_GROUND_SHADOW,
	GX_M2_LIGHTING_GROUND_LIGHT,
};

struct gx_m2_instance_frame
{
	gfx_buffer_t uniform_buffer;
	struct jks_array bone_mats; /* struct mat4f */
	struct mat4f shadow_mvp;
	struct mat4f shadow_mv;
	struct mat4f mvp;
	struct mat4f mv;
	float distance_to_camera;
	bool shadow_culled;
	bool culled;
};

enum gx_m2_instance_flag
{
	GX_M2_INSTANCE_FLAG_IN_RENDER_LIST = (1 << 0),
	GX_M2_INSTANCE_FLAG_ENABLE_LIGHTS  = (1 << 1),
	GX_M2_INSTANCE_FLAG_BONES_UPDATED  = (1 << 2),
	GX_M2_INSTANCE_FLAG_DYN_BATCHES    = (1 << 3),
	GX_M2_INSTANCE_FLAG_DYN_SHADOW     = (1 << 4),
};

struct gx_m2_instance
{
	struct gx_m2 *parent;
	struct gx_m2_instance_frame frames[RENDER_FRAMES_COUNT];
	struct gx_m2_particles *gx_particles;
	struct gx_m2_lighting *local_lighting;
	struct gx_m2_ribbons *gx_ribbons;
	struct jks_array lights; /* struct gx_m2_light */
	struct jks_array enabled_batches; /* uint16_t */
	struct jks_array bone_calc; /* bitmask as uint8_t */
#ifdef WITH_DEBUG_RENDERING
	struct gx_aabb gx_caabb;
	struct gx_aabb gx_aabb;
#endif
	struct gx_blp *monster_textures[3];
	struct gx_blp *skin_extra_texture;
	struct gx_blp *object_texture;
	struct gx_blp *skin_texture;
	struct gx_blp *hair_texture;
	struct aabb caabb;
	struct aabb aabb;
	struct mat4f m_inv;
	struct mat4f m;
	struct vec3f pos;
	struct wow_m2_sequence *prev_sequence;
	struct wow_m2_sequence *sequence;
	uint64_t prev_sequence_started;
	uint32_t prev_sequence_time;
	uint32_t prev_sequence_id;
	uint64_t sequence_started;
	uint32_t sequence_time;
	uint32_t sequence_id;
	uint32_t camera;
	float render_distance_max;
	float sequence_speed;
	float scale;
	enum gx_m2_lighting_type lighting_type;
	enum gx_m2_instance_flag flags;
	refcount_t refcount;
};

static inline bool gx_m2_instance_flag_get(struct gx_m2_instance *instance, enum gx_m2_instance_flag flag)
{
	return (__atomic_load_n(&instance->flags, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool gx_m2_instance_flag_set(struct gx_m2_instance *instance, enum gx_m2_instance_flag flag)
{
	return (__atomic_fetch_or(&instance->flags, flag, __ATOMIC_RELAXED) & flag) != 0;
}

static inline bool gx_m2_instance_flag_clear(struct gx_m2_instance *instance, enum gx_m2_instance_flag flag)
{
	return (__atomic_fetch_and(&instance->flags, ~flag, __ATOMIC_RELAXED) & flag) != 0;
}

struct gx_m2_instance *gx_m2_instance_new(struct gx_m2 *parent);
struct gx_m2_instance *gx_m2_instance_new_filename(const char *filename);
void gx_m2_instance_free(struct gx_m2_instance *instance);
void gx_m2_instance_ref(struct gx_m2_instance *instance);
void gx_m2_instance_clear_bones(struct gx_m2_instance *instance);
void gx_m2_instance_update_bones(struct gx_m2_instance *instance, struct gx_frame *frame);
void gx_m2_instance_update_bone(struct gx_m2_instance *instance, struct gx_frame *frame, uint16_t bone, const struct mat4f *mat);
void gx_m2_instance_clear_update(struct gx_m2_instance *instance, struct gx_frame *frame);
void gx_m2_instance_force_update(struct gx_m2_instance *instance, struct gx_frame *frame, struct gx_m2_render_params *params);
void gx_m2_instance_render(struct gx_m2_instance *instance, struct gx_frame *frame, bool transparent, struct gx_m2_render_params *params);
void gx_m2_instance_render_particles(struct gx_m2_instance *instance, struct gx_frame *frame, struct gx_m2_render_params *params);
void gx_m2_instance_render_ribbons(struct gx_m2_instance *instance, struct gx_frame *frame, struct gx_m2_render_params *params);
void gx_m2_instance_add_to_render(struct gx_m2_instance *instance, struct gx_frame *frame, bool bypass_frustum, struct gx_m2_render_params *params);
void gx_m2_instance_update_aabb(struct gx_m2_instance *instance);
void gx_m2_instance_on_parent_loaded(struct gx_m2_instance *instance);
void gx_m2_instance_set_skin_extra_texture(struct gx_m2_instance *instance, struct gx_blp *texture);
void gx_m2_instance_set_skin_texture(struct gx_m2_instance *instance, struct gx_blp *texture);
void gx_m2_instance_set_hair_texture(struct gx_m2_instance *instance, struct gx_blp *texture);
void gx_m2_instance_set_monster_texture(struct gx_m2_instance *instance, int idx, struct gx_blp *texture);
void gx_m2_instance_set_object_texture(struct gx_m2_instance *instance, struct gx_blp *texture);
void gx_m2_instance_set_sequence(struct gx_m2_instance *instance, uint32_t sequence);
void gx_m2_instance_enable_batch(struct gx_m2_instance *instance, uint16_t batch);
void gx_m2_instance_enable_batches(struct gx_m2_instance *instance, uint16_t start, uint16_t end);
void gx_m2_instance_disable_batch(struct gx_m2_instance *instance, uint16_t batch);
void gx_m2_instance_disable_batches(struct gx_m2_instance *instance, uint16_t start, uint16_t end);
void gx_m2_instance_clear_batches(struct gx_m2_instance *instance);
void gx_m2_instance_set_mat(struct gx_m2_instance *instance, struct mat4f *mat);

bool m2_get_track_value_vec4f(struct gx_m2 *m2, struct wow_m2_track *track, struct vec4f *val, struct wow_m2_sequence *sequence, uint32_t t);
bool m2_get_track_value_vec3f(struct gx_m2 *m2, struct wow_m2_track *track, struct vec3f *val, struct wow_m2_sequence *sequence, uint32_t t);
bool m2_get_track_value_float(struct gx_m2 *m2, struct wow_m2_track *track, float *val, struct wow_m2_sequence *sequence, uint32_t t);
bool m2_get_track_value_uint8(struct gx_m2 *m2, struct wow_m2_track *track, uint8_t *val, struct wow_m2_sequence *sequence, uint32_t t);
bool m2_get_track_value_int16(struct gx_m2 *m2, struct wow_m2_track *track, int16_t *val, struct wow_m2_sequence *sequence, uint32_t t);
bool m2_get_track_value_quat16(struct gx_m2 *m2, struct wow_m2_track *track, struct vec4f *val, struct wow_m2_sequence *sequence, uint32_t t);

bool m2_instance_get_track_value_vec4f(struct gx_m2_instance *instance, struct wow_m2_track *track, struct vec4f *val);
bool m2_instance_get_track_value_vec3f(struct gx_m2_instance *instance, struct wow_m2_track *track, struct vec3f *val);
bool m2_instance_get_track_value_float(struct gx_m2_instance *instance, struct wow_m2_track *track, float *val);
bool m2_instance_get_track_value_uint8(struct gx_m2_instance *instance, struct wow_m2_track *track, uint8_t *val);
bool m2_instance_get_track_value_int16(struct gx_m2_instance *instance, struct wow_m2_track *track, int16_t *val);
bool m2_instance_get_track_value_quat16(struct gx_m2_instance *instance, struct wow_m2_track *track, struct vec4f *val);

struct gx_m2_ground
{
	struct mat4f mat;
	struct vec3f position;
	float render_distance;
};

struct gx_m2_ground_batch_frame
{
	gfx_attributes_state_t attributes_state;
	gfx_buffer_t instanced_buffer;
	gfx_buffer_t uniform_buffer;
	struct jks_array instances; /* struct mat4f */
	size_t buffer_size;
};

struct gx_m2_ground_batch
{
	struct gx_m2_ground_batch_frame frames[RENDER_FRAMES_COUNT];
	struct gx_m2 *m2;
	bool initialized;
	bool light;
	refcount_t refcount;
};

struct gx_m2_ground_batch *gx_m2_ground_batch_new(struct gx_m2 *m2, bool light);
void gx_m2_ground_batch_free(struct gx_m2_ground_batch *batch);
void gx_m2_ground_batch_ref(struct gx_m2_ground_batch *batch);
void gx_m2_ground_batch_cull_start(struct gx_m2_ground_batch *batch, struct gx_frame *frame);
void gx_m2_ground_batch_cull(struct gx_m2_ground_batch *batch, struct gx_frame *frame, const struct gx_m2_ground *entries, size_t entries_count);
void gx_m2_ground_batch_cull_end(struct gx_m2_ground_batch *batch, struct gx_frame *frame);
void gx_m2_ground_batch_render(struct gx_m2_ground_batch *batch, struct gx_frame *frame);

#endif
