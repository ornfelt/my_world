#ifndef WOW_WMO_H
#define WOW_WMO_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_compound;
struct wow_mpq_file;

struct wow_mcvp
{
	struct wow_chunk_header header;
	struct wow_vec4f *data;
	uint32_t size;
};

struct wow_mfog_data
{
	uint32_t flags;
	struct wow_vec3f pos;
	float small_radius;
	float large_radius;
	float fog_end1;
	float fog_start1;
	struct wow_vec4b color1;
	float fog_end2;
	float fog_start2;
	struct wow_vec4b color2;
};

struct wow_mfog
{
	struct wow_chunk_header header;
	struct wow_mfog_data *data;
	uint32_t size;
};

#define WOW_MODD_NAME_MASK (0xFFFFFF)
#define WOW_MODD_FLAG_1    (1 << 24)
#define WOW_MODD_FLAG_2    (1 << 25)
#define WOW_MODD_FLAG_3    (1 << 26)
#define WOW_MODD_FLAG_4    (1 << 27)
#define WOW_MODD_FLAG_5    (1 << 28)
#define WOW_MODD_FLAG_6    (1 << 29)
#define WOW_MODD_FLAG_7    (1 << 30)
#define WOW_MODD_FLAG_8    (1 << 31)

struct wow_modd_data
{
	uint32_t name_flags;
	struct wow_vec3f position;
	struct wow_quatf rotation;
	float scale;
	struct wow_vec4b color;
};

struct wow_modd
{
	struct wow_chunk_header header;
	struct wow_modd_data *data;
	uint32_t size;
};

struct wow_modn
{
	struct wow_chunk_header header;
	char *data;
	uint32_t size;
};

struct wow_mods_data
{
	char name[20];
	uint32_t start_index;
	uint32_t count;
	char pad[4];
};

struct wow_mods
{
	struct wow_chunk_header header;
	struct wow_mods_data *data;
	uint32_t size;
};

enum wow_light_type
{
	WOW_LIGHT_OMNI = 0,
	WOW_LIGHT_SPOT = 1,
	WOW_LIGHT_DIRECT = 2,
	WOW_LIGHT_AMBIENT = 3,
};

struct wow_molt_data
{
	uint8_t type;
	uint8_t use_atten;
	uint8_t pad[2];
	struct wow_vec4b color;
	struct wow_vec3f position;
	float intensity;
	float unknown[4];
	float atten_start;
	float atten_end;
};

struct wow_molt
{
	struct wow_chunk_header header;
	struct wow_molt_data *data;
	uint32_t size;
};

struct wow_movb_data
{
	uint16_t first_vertex;
	uint16_t count;
};

struct wow_movb
{
	struct wow_chunk_header header;
	struct wow_movb_data *data;
	uint32_t size;
};

struct wow_movv
{
	struct wow_chunk_header header;
	struct wow_vec3f *data;
	uint32_t size;
};

struct wow_mopr_data
{
	uint16_t portal_index;
	uint16_t group_index;
	int16_t side;
	uint16_t filler;
};

struct wow_mopr
{
	struct wow_chunk_header header;
	struct wow_mopr_data *data;
	uint32_t size;
};

struct wow_mopt_data
{
	uint16_t start_vertex;
	uint16_t count;
	struct wow_vec3f normal;
	float distance;
};

struct wow_mopt
{
	struct wow_chunk_header header;
	struct wow_mopt_data *data;
	uint32_t size;
};

struct wow_mopv
{
	struct wow_chunk_header header;
	struct wow_vec3f *data;
	uint32_t size;
};

struct wow_mogi_data
{
	uint32_t flags;
	struct wow_vec3f aabb0;
	struct wow_vec3f aabb1;
	int32_t name_offset;
};

struct wow_mogi
{
	struct wow_chunk_header header;
	struct wow_mogi_data *data;
	uint32_t size;
};

#define WOW_MOMT_FLAGS_UNLIT    0x001
#define WOW_MOMT_FLAGS_UNFOGGED 0x002
#define WOW_MOMT_FLAGS_UNCULLED 0x004
#define WOW_MOMT_FLAGS_EXTLIGHT 0x008
#define WOW_MOMT_FLAGS_SIDN     0x010
#define WOW_MOMT_FLAGS_WINDOW   0x020
#define WOW_MOMT_FLAGS_CLAMP_S  0x040
#define WOW_MOMT_FLAGS_CLAMP_T  0x080
#define WOW_MOMT_FLAGS_UNKNOWN  0x100

struct wow_momt_data
{
	/* 0x00 */ uint32_t flags;
	/* 0x04 */ uint32_t shader;
	/* 0x08 */ uint32_t blend_mode;
	/* 0x0C */ uint32_t texture1;
	/* 0x10 */ struct wow_vec4b emissive_color;
	/* 0x14 */ struct wow_vec4b sidn_emissive_color;
	/* 0x18 */ uint32_t texture2;
	/* 0x1C */ uint32_t diff_color;
	/* 0x20 */ uint32_t group_type;
	/* 0x24 */ uint32_t texture3;
	/* 0x28 */ uint32_t color2;
	/* 0x2C */ uint32_t flags2;
	/* 0x30 */ uint32_t run_time_data[4];
};

struct wow_momt
{
	struct wow_chunk_header header;
	struct wow_momt_data *data;
	uint32_t size;
};

struct wow_motx
{
	struct wow_chunk_header header;
	char *data;
	uint32_t size;
};

#define WOW_MOHD_FLAGS_ATTENUATE_PORTAL_DISTANCE 0x1
#define WOW_MOHD_FLAGS_SKIP_BASE_COLOR           0x2
#define WOW_MOHD_FLAGS_USE_DBC_LIQUID_TYPE       0x4
#define WOW_MOHD_FLAGS_LIGHTEN_INTERIORS         0x8

struct wow_mohd
{
	struct wow_chunk_header header;
	uint32_t textures_nb;
	uint32_t groups_nb;
	uint32_t portals_nb;
	uint32_t lights_nb;
	uint32_t models_nb;
	uint32_t doodad_defs_nb;
	uint32_t doodad_sets_nb;
	struct wow_vec4b ambient;
	uint32_t wmo_id;
	struct wow_vec3f aabb0;
	struct wow_vec3f aabb1;
	uint16_t flags;
	uint16_t num_lod;
};

struct wow_mogn
{
	struct wow_chunk_header header;
	char *data;
	uint32_t size;
};

struct wow_mosb
{
	struct wow_chunk_header header;
	char *data;
	uint32_t size;
};

struct wow_wmo_file
{
	struct wow_motx motx;
	struct wow_momt momt;
	struct wow_mver mver;
	struct wow_mohd mohd;
	struct wow_mogi mogi;
	struct wow_mopv mopv;
	struct wow_mopt mopt;
	struct wow_mopr mopr;
	struct wow_movv movv;
	struct wow_movb movb;
	struct wow_molt molt;
	struct wow_mods mods;
	struct wow_modn modn;
	struct wow_modd modd;
	struct wow_mfog mfog;
	struct wow_mcvp mcvp;
	struct wow_mogn mogn;
	struct wow_mosb mosb;
};

struct wow_wmo_file *wow_wmo_file_new(struct wow_mpq_file *mpq);
void wow_wmo_file_delete(struct wow_wmo_file *file);

#ifdef __cplusplus
}
#endif

#endif
