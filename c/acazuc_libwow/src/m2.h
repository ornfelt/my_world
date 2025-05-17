#ifndef WOW_M2_H
#define WOW_M2_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

struct wow_m2_spline_vec3f
{
	/* 0x00 */ struct wow_vec3f value;
	/* 0x0C */ struct wow_vec3f in_tan;
	/* 0x18 */ struct wow_vec3f out_tan;
};

struct wow_m2_spline_float
{
	/* 0x0 */ float value;
	/* 0x4 */ float in_tan;
	/* 0x8 */ float out_tan;
};

struct wow_m2_array
{
	/* 0x0 */ uint32_t count;
	/* 0x4 */ uint32_t offset;
};

struct wow_m2_vertex
{
	/* 0x00 */ struct wow_vec3f pos;
	/* 0x0C */ uint8_t bone_weights[4];
	union
	{
		/* 0x10 */ uint8_t bone_indices[4];
		/* 0x10 */ uint32_t bone32;
	};
	/* 0x14 */ struct wow_vec3f normal;
	/* 0x20 */ struct wow_vec2f tex_coords[2];
};

struct wow_m2_bounds
{
	/* 0x00 */ struct wow_vec3f aabb0;
	/* 0x0C */ struct wow_vec3f aabb1;
	/* 0x18 */ float radius;
};

struct wow_m2_range
{
	/* 0x0 */ uint32_t minimum;
	/* 0x4 */ uint32_t maximum;
};

#define WOW_M2_MATERIAL_FLAGS_UNLIT       0x01
#define WOW_M2_MATERIAL_FLAGS_UNFOGGED    0x02
#define WOW_M2_MATERIAL_FLAGS_UNCULLED    0x04
#define WOW_M2_MATERIAL_FLAGS_DEPTH_TEST  0x08
#define WOW_M2_MATERIAL_FLAGS_DEPTH_WRITE 0x10

struct wow_m2_material
{
	/* 0x0 */ uint16_t flags;
	/* 0x2 */ uint16_t blend_mode;
};

struct wow_m2_box
{
	/* 0x0 */ struct wow_vec3f model_rotation_speed_min;
	/* 0xC */ struct wow_vec3f model_rotation_speed_max;
};

#define WOW_M2_SKIN_SECTION_HAIR     0
#define WOW_M2_SKIN_SECTION_FACIAL1  1
#define WOW_M2_SKIN_SECTION_FACIAL2  2
#define WOW_M2_SKIN_SECTION_FACIAL3  3
#define WOW_M2_SKIN_SECTION_GLOVES   4
#define WOW_M2_SKIN_SECTION_BOOTS    5
#define WOW_M2_SKIN_SECTION_TAIL     6
#define WOW_M2_SKIN_SECTION_EARS     7
#define WOW_M2_SKIN_SECTION_WRISTS   8
#define WOW_M2_SKIN_SECTION_KNEEPADS 9
#define WOW_M2_SKIN_SECTION_CHEST    10
#define WOW_M2_SKIN_SECTION_PANTS    11
#define WOW_M2_SKIN_SECTION_TABARD   12
#define WOW_M2_SKIN_SECTION_TROUSERS 13
#define WOW_M2_SKIN_SECTION_UNK      14
#define WOW_M2_SKIN_SECTION_CLOAK    15
#define WOW_M2_SKIN_SECTION_MISC     16
#define WOW_M2_SKIN_SECTION_EYES     17
#define WOW_M2_SKIN_SECTION_BELT     18
#define WOW_M2_SKIN_SECTION_BONE     19

struct wow_m2_skin_section
{
	/* 0x00 */ uint16_t skin_section_id;
	/* 0x02 */ uint16_t level;
	/* 0x04 */ uint16_t vertex_start;
	/* 0x06 */ uint16_t vertex_count;
	/* 0x08 */ uint16_t index_start;
	/* 0x0A */ uint16_t index_count;
	/* 0x0C */ uint16_t bone_count;
	/* 0x0E */ uint16_t bone_combo_index;
	/* 0x10 */ uint16_t bone_influences;
	/* 0x12 */ uint16_t center_bone_index;
	/* 0x14 */ struct wow_vec3f center_position;
	/* 0x20 */ struct wow_vec3f sort_center_position;
	/* 0x2C */ float sort_radius;
};

#define WOW_M2_BATCH_FLAGS_INVERT      0x01
#define WOW_M2_BATCH_FLAGS_TRANSFORM   0x02
#define WOW_M2_BATCH_FLAGS_PROJECTED   0x04
#define WOW_M2_BATCH_FLAGS_UNKNOWN     0x08
#define WOW_M2_BATCH_FLAGS_BATCH       0x10
#define WOW_M2_BATCH_FLAGS_PROJECTED2  0x20
#define WOW_M2_BATCH_FLAGS_HAS_WEIGHTS 0x40

struct wow_m2_batch
{
	/* 0x00 */ uint8_t flags;
	/* 0x01 */ int8_t priority_plane;
	/* 0x02 */ int16_t shader_id;
	/* 0x04 */ uint16_t skin_section_index;
	/* 0x06 */ uint16_t geoset_index;
	/* 0x08 */ uint16_t color_index;
	/* 0x0A */ uint16_t material_index;
	/* 0x0C */ uint16_t material_layer;
	/* 0x0E */ uint16_t texture_count;
	/* 0x10 */ uint16_t texture_combo_index;
	/* 0x12 */ uint16_t texture_coord_combo_index;
	/* 0x13 */ uint16_t texture_weight_combo_index;
	/* 0x14 */ uint16_t texture_transform_combo_index;
};

struct wow_m2_sequence
{
	/* 0x00 */ uint16_t id;
	/* 0x02 */ uint16_t variation_index;
	/* 0x04 */ uint32_t start;
	/* 0x08 */ uint32_t end;
	/* 0x0C */ float movespeed;
	/* 0x10 */ uint32_t flags;
	/* 0x14 */ int16_t frequency;
	/* 0x16 */ uint16_t padding;
	/* 0x18 */ struct wow_m2_range replay;
	/* 0x20 */ uint32_t blend_time;
	/* 0x24 */ struct wow_m2_bounds bounds;
	/* 0x40 */ int16_t variation_next;
	/* 0x42 */ uint16_t alias_next;
};

struct wow_m2_playable_animation
{
	/* 0x0 */ uint16_t id;
	/* 0x2 */ uint16_t flags;
};

#define WOW_M2_BONE_BILLBOARD               (WOW_M2_BONE_CYLINDRICAL_BILLBOARD | WOW_M2_BONE_SPHERICAL_BILLBOARD)
#define WOW_M2_BONE_CYLINDRICAL_BILLBOARD   (WOW_M2_BONE_CYLINDRICAL_X_BILLBOARD | WOW_M2_BONE_CYLINDRICAL_Y_BILLBOARD | WOW_M2_BONE_CYLINDRICAL_Z_BILLBOARD)
#define WOW_M2_BONE_SPHERICAL_BILLBOARD     0x0008
#define WOW_M2_BONE_CYLINDRICAL_X_BILLBOARD 0x0010
#define WOW_M2_BONE_CYLINDRICAL_Y_BILLBOARD 0x0020
#define WOW_M2_BONE_CYLINDRICAL_Z_BILLBOARD 0x0040
#define WOW_M2_BONE_TRANSFORMED             0x0200
#define WOW_M2_BONE_KINEMATIC               0x0400
#define WOW_M2_BONE_HELMET_ANIM_SCALED      0x1000

#define WOW_M2_HEADER_FLAG_TILT_X                     0x01
#define WOW_M2_HEADER_FLAG_TILT_Y                     0x02
#define WOW_M2_HEADER_FLAG_UNKNOWN1                   0x04
#define WOW_M2_HEADER_FLAG_USE_TEXTURE_COMBINER_COMBO 0x08
#define WOW_M2_HEADER_FLAG_UNKNOWN2                   0x10

struct wow_m2_header
{
	/* 0x000 */ uint32_t magic;
	/* 0x004 */ uint32_t version;
	/* 0x008 */ struct wow_m2_array name; /* char */
	/* 0x010 */ uint32_t flags;
	/* 0x014 */ struct wow_m2_array global_sequences;                /* uint32_t */
	/* 0x01C */ struct wow_m2_array sequences;                       /* struct wow_m2_sequence */
	/* 0x024 */ struct wow_m2_array sequence_lookup_table;           /* uint16_t */
	/* 0x02C */ struct wow_m2_array playable_animations;             /* struct wow_m2_playable_animation */
	/* 0x034 */ struct wow_m2_array bones;                           /* struct wow_m2_bone_int */
	/* 0x03C */ struct wow_m2_array key_bone_lookup_table;           /* uint16_t */
	/* 0x044 */ struct wow_m2_array vertexes;                        /* struct wow_m2_vertex */
	/* 0x04C */ struct wow_m2_array skin_profiles;                   /* struct wow_m2_skin_profile */
	/* 0x054 */ struct wow_m2_array colors;                          /* struct wow_m2_color_int */
	/* 0x05C */ struct wow_m2_array textures;                        /* struct wow_m2_texture */
	/* 0x064 */ struct wow_m2_array texture_weights;                 /* struct wow_m2_texture_weight_int */
	/* 0x06C */ struct wow_m2_array unknown;                         /* struct wow_m2_texture_transform_int */
	/* 0x074 */ struct wow_m2_array texture_transforms;              /* struct wow_m2_texture_transform_int */
	/* 0x07C */ struct wow_m2_array replacable_texture_lookup;       /* uint16_t */
	/* 0x084 */ struct wow_m2_array materials;                       /* struct wow_m2_material */
	/* 0x08C */ struct wow_m2_array bone_lookup_table;               /* uint16_t */
	/* 0x094 */ struct wow_m2_array texture_lookup_table;            /* uint16_t */
	/* 0x09C */ struct wow_m2_array texture_unit_lookup_table;       /* uint16_t */
	/* 0x0A4 */ struct wow_m2_array texture_weights_lookup_table;    /* uint16_t */
	/* 0x0AC */ struct wow_m2_array texture_transforms_lookup_table; /* uint16_t */
	/* 0x0B4 */ struct wow_vec3f aabb0;
	/* 0x0C0 */ struct wow_vec3f aabb1;
	/* 0x0CC */ float bounding_sphere_radius;
	/* 0x0D0 */ struct wow_vec3f caabb0;
	/* 0x0DC */ struct wow_vec3f caabb1;
	/* 0x0E8 */ float collision_sphere_radius;
	/* 0x0EC */ struct wow_m2_array collision_triangles;     /* uint16_t */
	/* 0x0F4 */ struct wow_m2_array collision_vertexes;      /* vec3 */
	/* 0x0FC */ struct wow_m2_array collision_normals;       /* vec3 */
	/* 0x104 */ struct wow_m2_array attachments;             /* struct wow_m2_attachment */
	/* 0x10C */ struct wow_m2_array attachment_lookup_table; /* uint16_t */
	/* 0x114 */ struct wow_m2_array events;                  /* struct wow_m2_event */
	/* 0x11C */ struct wow_m2_array lights;                  /* struct wow_m2_light_int */
	/* 0x124 */ struct wow_m2_array cameras;                 /* struct wow_m2_camera_int */
	/* 0x12C */ struct wow_m2_array camera_lookup_table;     /* uint16_t */
	/* 0x134 */ struct wow_m2_array ribbons;                 /* struct wow_m2_ribbon */
	/* 0x13C */ struct wow_m2_array particles;               /* srtuct wow_m2_particle */
	/* 0x144 */ struct wow_m2_array texture_combiner_combos; /* uint16_t */
};

struct wow_m2_skin_profile_bone
{
	union
	{
		uint8_t values[4];
		uint32_t u32;
	};
};

struct wow_m2_skin_profile
{
	struct wow_m2_skin_section *sections;
	uint32_t sections_nb;
	struct wow_m2_batch *batches;
	uint32_t batches_nb;
	uint16_t *vertexes;
	uint32_t vertexes_nb;
	uint16_t *indices;
	uint32_t indices_nb;
	struct wow_m2_skin_profile_bone *bones;
	uint32_t bones_nb;
};

struct wow_m2_track
{
	uint16_t interpolation_type;
	int16_t global_sequence;
	struct wow_m2_range *interpolation_ranges;
	uint32_t interpolation_ranges_nb;
	uint32_t *timestamps;
	uint32_t timestamps_nb;
	void *values;
	uint32_t values_nb;
};

struct wow_m2_texture_transform
{
	struct wow_m2_track translation; /* srtuct wow_vec3f */
	struct wow_m2_track rotation;    /* struct wow_quatf */
	struct wow_m2_track scaling;     /* struct wow_vec3f */
};

struct wow_m2_texture_weight
{
	struct wow_m2_track weight; /* int16_t */
};

struct wow_m2_color
{
	struct wow_m2_track color; /* struct wow_vec3f */
	struct wow_m2_track alpha; /* int16_t */
};

struct wow_m2_bone
{
	int32_t key_bone_id;
	uint32_t flags;
	int16_t parent_bone;
	uint16_t submesh_id;
	uint32_t bone_name_crc;
	struct wow_m2_track translation; /* struct wow_vec3f */
	struct wow_m2_track rotation;    /* struct wow_quats */
	struct wow_m2_track scale;       /* struct wow_vec3f */
	struct wow_vec3f pivot;
};

struct wow_m2_light
{
	uint16_t type;
	int16_t bone;
	struct wow_vec3f position;
	struct wow_m2_track ambient_color;     /* struct wow_vec3f */
	struct wow_m2_track ambient_intensity; /* float */
	struct wow_m2_track diffuse_color;     /* struct wow_vec3f */
	struct wow_m2_track diffuse_intensity; /* float */
	struct wow_m2_track attenuation_start; /* float */
	struct wow_m2_track attenuation_end;   /* float */
	struct wow_m2_track visibility;        /* uint8_t */
};

struct wow_m2_camera
{
	uint32_t type;
	float fov;
	float far_clip;
	float near_clip;
	struct wow_m2_track position;        /* struct m2_spline_vec3f */
	struct wow_vec3f position_base;
	struct wow_m2_track target_position; /* struct m2_spline_vec3f */
	struct wow_vec3f target_position_base;
	struct wow_m2_track roll;            /* struct m2_spline_float */
};

#define WOW_M2_PARTICLE_FLAG_LIGHTING   0x0000001
#define WOW_M2_PARTICLE_FLAG_UNK1       0x0000002
#define WOW_M2_PARTICLE_FLAG_UNK2       0x0000004
#define WOW_M2_PARTICLE_FLAG_WORLDUP    0x0000008
#define WOW_M2_PARTICLE_FLAG_DONTTRAIL  0x0000010
#define WOW_M2_PARTICLE_FLAG_UNLIT      0x0000020
#define WOW_M2_PARTICLE_FLAG_BURST      0x0000040
#define WOW_M2_PARTICLE_FLAG_MODELSPACE 0x0000080
#define WOW_M2_PARTICLE_FLAG_UNK3       0x0000100
#define WOW_M2_PARTICLE_FLAG_RANDOMIZED 0x0000200
#define WOW_M2_PARTICLE_FLAG_PINNED     0x0000400
#define WOW_M2_PARTICLE_FLAG_UNK4       0x0000800
#define WOW_M2_PARTICLE_FLAG_XYQUAD     0x0001000
#define WOW_M2_PARTICLE_FLAG_GROUND     0x0002000
#define WOW_M2_PARTICLE_FLAG_UNK5       0x0004000
#define WOW_M2_PARTICLE_FLAG_UNK6       0x0008000
#define WOW_M2_PARTICLE_FLAG_RAND_TEX   0x0010000
#define WOW_M2_PARTICLE_FLAG_OUTWARD    0x0020000
#define WOW_M2_PARTICLE_FLAG_INWARD     0x0040000
#define WOW_M2_PARTICLE_FLAG_SCALEVARY  0x0080000
#define WOW_M2_PARTICLE_FLAG_UNK7       0x0100000
#define WOW_M2_PARTICLE_FLAG_RAND_FLIP  0x0200000
#define WOW_M2_PARTICLE_FLAG_NO_DIST    0x0400000
#define WOW_M2_PARTICLE_FLAG_COMP_GRAV  0x0800000
#define WOW_M2_PARTICLE_FLAG_BONE_JOINT 0x1000000
#define WOW_M2_PARTICLE_FLAG_UNK8       0x2000000
#define WOW_M2_PARTICLE_FLAG_RATE_DIST  0x4000000
#define WOW_M2_PARTICLE_FLAG_UNK9       0x8000000

struct wow_m2_particle
{
	uint32_t id;
	uint32_t flags;
	struct wow_vec3f position;
	uint16_t bone;
	uint16_t texture;
	char *geometry_model_filename;
	char *recursion_model_filename;
	union
	{
		struct
		{
			uint8_t blending_type2;
			uint8_t emitter_type2;
			uint16_t particle_color_index;
		};
		struct
		{
			uint16_t blending_type;
			uint16_t emitter_type;
		};
	};
	uint8_t particle_type;
	uint8_t head_or_tail;
	int16_t texture_tile_rotation;
	uint16_t texture_dimensions_rows;
	uint16_t texture_dimensions_columns;
	struct wow_m2_track emission_speed;       /* float */
	struct wow_m2_track speed_variation;      /* float */
	struct wow_m2_track vertical_range;       /* float */
	struct wow_m2_track horizontal_range;     /* float */
	struct wow_m2_track gravity;              /* float */
	struct wow_m2_track lifespan;             /* float */
	struct wow_m2_track emission_rate;        /* float */
	struct wow_m2_track emission_area_length; /* float */
	struct wow_m2_track emission_area_width;  /* float */
	struct wow_m2_track z_source;             /* float */
	float mid_point;
	struct wow_vec4b color_values[3];
	float scale_values[3];
	uint16_t lifespan_uv_anim[3];
	uint16_t decay_uv_anim[3];
	int16_t tail_uv_anim[2];
	int16_t tail_decay_uv_anim[2];
	float tail_length;
	float twinkle_speed;
	float twinkle_percent;
	float twinkle_scale_min;
	float twinkle_scale_max;
	float burst_multiplier;
	float drag;
	float spin;
	struct wow_m2_box tumble;
	struct wow_vec3f wind_vector;
	float wind_time;
	float follow_speed1;
	float follow_scale1;
	float follow_speed2;
	float follow_scale2;
	struct wow_vec3f *spline_points; /* struct wow_vec3f */
	uint32_t spline_points_nb;
	struct wow_m2_track enabled_in;  /* uint8_t */
};

#define WOW_M2_TEXTURE_FLAG_CLAMP_S 0x1
#define WOW_M2_TEXTURE_FLAG_CLAMP_T 0x2

struct wow_m2_texture
{
	uint32_t type;
	uint32_t flags;
	char *filename;
};

struct wow_m2_attachment
{
	uint32_t id;
	uint16_t bone;
	uint16_t unknown;
	struct wow_vec3f position;
	struct wow_m2_track animate_attached; /* uint8_t */
};

struct wow_m2_ribbon
{
	uint32_t ribbon_id;
	uint32_t bone_index;
	struct wow_vec3f position;
	uint16_t *texture_indices;
	uint32_t texture_indices_nb;
	uint16_t *material_indices;
	uint32_t material_indices_nb;
	struct wow_m2_track color;            /* struct wow_vec3f */
	struct wow_m2_track alpha;            /* int16_t */
	struct wow_m2_track height_above;     /* float */
	struct wow_m2_track height_below;     /* float */
	float edges_per_second;
	float edge_lifetime;
	float gravity;
	uint16_t texture_rows;
	uint16_t texture_cols;
	struct wow_m2_track tex_slot;         /* uint16_t */
	struct wow_m2_track visibility;       /* uint8_t */
};

struct wow_m2_file
{
	struct wow_m2_playable_animation *playable_animations;
	struct wow_m2_texture_transform *texture_transforms;
	struct wow_m2_texture_weight *texture_weights;
	struct wow_m2_skin_profile *skin_profiles;
	struct wow_m2_attachment *attachments;
	struct wow_m2_material *materials;
	struct wow_m2_sequence *sequences;
	struct wow_m2_particle *particles;
	struct wow_m2_texture *textures;
	struct wow_m2_vertex *vertexes;
	struct wow_m2_camera *cameras;
	struct wow_m2_ribbon *ribbons;
	struct wow_m2_color *colors;
	struct wow_m2_light *lights;
	struct wow_m2_bone *bones;
	struct wow_vec3f *collision_vertexes;
	struct wow_vec3f *collision_normals;
	uint16_t *texture_transforms_lookups;
	uint16_t *texture_weights_lookups;
	uint16_t *texture_combiner_combos;
	uint16_t *texture_unit_lookups;
	uint16_t *collision_triangles;
	uint16_t *attachment_lookups;
	uint16_t *key_bone_lookups;
	uint16_t *sequence_lookups;
	uint32_t *global_sequences;
	uint16_t *texture_lookups;
	uint16_t *camera_lookups;
	uint16_t *bone_lookups;
	uint32_t playable_animations_nb;
	uint32_t texture_transforms_nb;
	uint32_t texture_weights_nb;
	uint32_t skin_profiles_nb;
	uint32_t attachments_nb;
	uint32_t materials_nb;
	uint32_t sequences_nb;
	uint32_t particles_nb;
	uint32_t textures_nb;
	uint32_t vertexes_nb;
	uint32_t cameras_nb;
	uint32_t ribbons_nb;
	uint32_t colors_nb;
	uint32_t lights_nb;
	uint32_t bones_nb;
	uint32_t collision_vertexes_nb;
	uint32_t collision_normals_nb;
	uint32_t texture_transforms_lookups_nb;
	uint32_t texture_weights_lookups_nb;
	uint32_t texture_combiner_combos_nb;
	uint32_t texture_unit_lookups_nb;
	uint32_t collision_triangles_nb;
	uint32_t attachment_lookups_nb;
	uint32_t key_bone_lookups_nb;
	uint32_t sequence_lookups_nb;
	uint32_t global_sequences_nb;
	uint32_t texture_lookups_nb;
	uint32_t camera_lookups_nb;
	uint32_t bone_lookups_nb;
	char *name;
	struct wow_m2_header header;
};

struct wow_m2_file *wow_m2_file_new(struct wow_mpq_file *mpq);
void wow_m2_file_delete(struct wow_m2_file *file);

struct wow_m2_texture_transform *wow_m2_texture_transforms_dup(const struct wow_m2_texture_transform *dup, uint32_t nb);
void wow_m2_texture_transforms_delete(struct wow_m2_texture_transform *val, uint32_t nb);

struct wow_m2_texture_weight *wow_m2_texture_weights_dup(const struct wow_m2_texture_weight *dup, uint32_t nb);
void wow_m2_texture_weights_delete(struct wow_m2_texture_weight *val, uint32_t nb);

struct wow_m2_skin_profile *wow_m2_skin_profiles_dup(const struct wow_m2_skin_profile *dup, uint32_t nb);
void wow_m2_skin_profiles_delete(struct wow_m2_skin_profile *val, uint32_t nb);

struct wow_m2_attachment *wow_m2_attachments_dup(const struct wow_m2_attachment *dup, uint32_t nb);
void wow_m2_attachments_delete(struct wow_m2_attachment *val, uint32_t nb);

struct wow_m2_particle *wow_m2_particles_dup(const struct wow_m2_particle *dup, uint32_t nb);
void wow_m2_particles_delete(struct wow_m2_particle *val, uint32_t nb);

struct wow_m2_texture *wow_m2_textures_dup(const struct wow_m2_texture *dup, uint32_t nb);
void wow_m2_textures_delete(struct wow_m2_texture *val, uint32_t nb);

struct wow_m2_camera *wow_m2_cameras_dup(const struct wow_m2_camera *dup, uint32_t nb);
void wow_m2_cameras_delete(struct wow_m2_camera *val, uint32_t nb);

struct wow_m2_ribbon *wow_m2_ribbons_dup(const struct wow_m2_ribbon *dup, uint32_t nb);
void wow_m2_ribbons_delete(struct wow_m2_ribbon *vla, uint32_t nb);

struct wow_m2_color *wow_m2_colors_dup(const struct wow_m2_color *dup, uint32_t nb);
void wow_m2_colors_delete(struct wow_m2_color *val, uint32_t nb);

struct wow_m2_light *wow_m2_lights_dup(const struct wow_m2_light *dup, uint32_t nb);
void wow_m2_lights_delete(struct wow_m2_light *val, uint32_t nb);

struct wow_m2_bone *wow_m2_bones_dup(const struct wow_m2_bone *dup, uint32_t nb);
void wow_m2_bones_delete(struct wow_m2_bone *val, uint32_t nb);

#ifdef __cplusplus
}
#endif

#endif
