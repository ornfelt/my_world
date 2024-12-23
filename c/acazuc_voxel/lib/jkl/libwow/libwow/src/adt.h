#ifndef WOW_ADT_H
#define WOW_ADT_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

#define WOW_MHDR_FLAGS_MFBO      0x1
#define WOW_MHDR_FLAGS_NORTHREND 0x2

struct wow_mhdr
{
	struct wow_chunk_header header;
	uint32_t flags;
	uint32_t mcin;
	uint32_t mtex;
	uint32_t mmdx;
	uint32_t mmid;
	uint32_t mwmo;
	uint32_t mwid;
	uint32_t mddf;
	uint32_t modf;
	uint32_t mfbo;
	uint32_t mh2o;
	uint32_t mtxf;
	uint32_t unused[4];
};

struct wow_mfbo
{
	struct wow_chunk_header header;
	uint16_t maximum[3][3];
	uint16_t minimum[3][3];
};

struct wow_mmdx
{
	struct wow_chunk_header header;
	char *data;
	uint32_t size;
};

struct wow_mmid
{
	struct wow_chunk_header header;
	uint32_t *data;
	uint32_t size;
};

#define WOW_MDDF_FLAGS_BIODOME   0x01
#define WOW_MDDF_FLAGS_SHRUBBERY 0x02
#define WOW_MDDF_FLAGS_UNKNOWN1  0x04
#define WOW_MDDF_FLAGS_UNKNOWN2  0x08
#define WOW_MDDF_FLAGS_LIQUID    0x10
#define WOW_MDDF_FLAGS_FILEDATA  0x20
#define WOW_MDDF_FLAGS_UNKNOWN3  0x40

struct wow_mddf_data
{
	uint32_t name_id;
	uint32_t unique_id;
	struct wow_vec3f position;
	struct wow_vec3f rotation;
	uint16_t scale;
	uint16_t flags;
};

struct wow_mddf
{
	struct wow_chunk_header header;
	struct wow_mddf_data *data;
	uint32_t size;
};

struct wow_mwid
{
	struct wow_chunk_header header;
	uint32_t *data;
	uint32_t size;
};

struct wow_mcin_data
{
	uint32_t offset;
	uint32_t size;
	uint32_t flags;
	uint32_t async_id;
};

struct wow_mcin
{
	struct wow_chunk_header header;
	struct wow_mcin_data data[16 * 16];
};

struct wow_mcvt
{
	struct wow_chunk_header header;
	float data[9 * 9 + 8 * 8];
};

struct wow_mcnr
{
	struct wow_chunk_header header;
	int8_t data[(9 * 9 + 8 * 8) * 3];
};

struct wow_mcly_data_flags
{
	uint32_t animation_rotation : 3;
	uint32_t animation_speed : 3;
	uint32_t animation_enabled : 1;
	uint32_t overbright : 1;
	uint32_t use_alpha_map : 1;
	uint32_t alpha_map_compressed : 1;
	uint32_t use_cube_map_reflection : 1;
	uint32_t unknown_0x800 : 1;
	uint32_t unknown_0x1000 : 1;
	uint32_t unused : 19;
};

struct wow_mcly_data
{
	uint32_t texture_id;
	struct wow_mcly_data_flags flags;
	uint32_t offset_in_mcal;
	uint32_t effect_id;
};

struct wow_mcly
{
	struct wow_chunk_header header;
	struct wow_mcly_data *data;
	uint32_t size;
};

struct wow_mcrf
{
	struct wow_chunk_header header;
	uint32_t *doodads;
	uint32_t doodads_nb;
	uint32_t *wmos;
	uint32_t wmos_nb;
};

struct wow_mcal
{
	struct wow_chunk_header header;
	uint8_t *data;
	uint32_t size;
};

struct wow_mcsh
{
	struct wow_chunk_header header;
	uint8_t data[64][8];
};

struct wow_mcse_data
{
	uint32_t sound_point_id;
	uint32_t sound_name_id;
	struct wow_vec3f pos;
	float min_distance;
	float max_distance;
	float cutoff_distance;
	uint16_t start_time;
	uint16_t end_time;
	uint16_t mode;
	uint16_t group_silence_min;
	uint16_t group_silence_max;
	uint16_t play_instances_min;
	uint16_t play_instances_max;
	uint8_t loop_count_min;
	uint8_t loop_count_max;
	uint16_t inter_sound_gap_min;
	uint16_t inter_sound_gap_max;
};

struct wow_mcse
{
	struct wow_chunk_header header;
	struct wow_mcse_data *data;
	uint32_t size;
};

#define WOW_MCLQ_TILE_LIQUID_OCEAN 0x01
#define WOW_MCLQ_TILE_LIQUID_RIVER 0x04
#define WOW_MCLQ_TILE_LIQUID_MAGMA 0x06
#define WOW_MCLQ_TILE_LIQUID_SLIME 0x07
#define WOW_MCLQ_TILE_HIDDEN       0x08
#define WOW_MCLQ_TILE_LIQUID_TYPE  0x0f
#define WOW_MCLQ_TILE_MUST_SWIM    0x40
#define WOW_MCLQ_TILE_FATIGUE      0x80

struct wow_mclq_data
{
	float height_min;
	float height_max;
	struct
	{
		union
		{
			struct
			{
				uint8_t depth;
				char flow0_pct;
				char flow1_pct;
				char padding;
				float height;
			} water;
			struct
			{
				uint8_t depth;
				char foam;
				char wet;
				char padding;
				float height;
			} ocean;
			struct
			{
				uint16_t s;
				uint16_t t;
				float height;
			} magma;
		};
	} verts[9 * 9];
	uint8_t tiles[8][8];
	uint32_t n_flowvs;
	struct
	{
		struct wow_vec3f sphere_pos;
		float sphere_radius;
		struct wow_vec3f dir;
		float velocity;
		float amplitude;
		float frequency;
	} flows[2];
};

struct wow_mclq
{
	struct wow_chunk_header header;
	struct wow_mclq_data *data;
	uint32_t size;
};

struct wow_mccv
{
	struct wow_chunk_header header;
	struct wow_vec4b data[9 * 9 + 8 * 8];
};

struct wow_mclv
{
	struct wow_chunk_header header;
	struct wow_vec4b data[9 * 9 + 8 * 8];
};

#define WOW_MCNK_FLAGS_MCSH       0x00001
#define WOW_MCNK_FLAGS_IMPASS     0x00002
#define WOW_MCNK_FLAGS_LQ_RIVER   0x00004
#define WOW_MCNK_FLAGS_LQ_OCEAN   0x00008
#define WOW_MCNK_FLAGS_LQ_MAGMA   0x00010
#define WOW_MCNK_FLAGS_LQ_SLIME   0x00020
#define WOW_MCNK_FLAGS_MCCV       0x00040
#define WOW_MCNK_FLAGS_UNKNOWN1   0x00080
#define WOW_MCNK_FLAGS_UNKNOWN2   0x00100
#define WOW_MCNK_FLAGS_UNKNOWN3   0x00200
#define WOW_MCNK_FLAGS_UNKNOWN4   0x00400
#define WOW_MCNK_FLAGS_UNKNOWN5   0x00800
#define WOW_MCNK_FLAGS_UNKNOWN6   0x01000
#define WOW_MCNK_FLAGS_UNKNOWN7   0x02000
#define WOW_MCNK_FLAGS_UNKNOWN7   0x02000
#define WOW_MCNK_FLAGS_FIX_MCAL   0x08000
#define WOW_MCNK_FLAGS_HIGH_HOLES 0x10000

struct wow_mcnk_header
{
	struct wow_chunk_header header;
	uint32_t flags;
	uint32_t index_x;
	uint32_t index_y;
	uint32_t layers;
	uint32_t doodad_refs_nb;
	uint32_t ofs_mcvt;
	uint32_t ofs_mcnr;
	uint32_t ofs_mcly;
	uint32_t ofs_mcrf;
	uint32_t ofs_mcal;
	uint32_t size_mcal;
	uint32_t ofs_mcsh;
	uint32_t size_mcsh;
	uint32_t area_id;
	uint32_t map_obj_refs_nb;
	uint16_t holes;
	uint16_t unknown;
	uint16_t low_quality_texture[8];
	union
	{
		uint8_t no_effect_doodad[8];
		uint64_t no_effect_doodads;
	};
	uint32_t ofs_mcse;
	uint32_t snd_emitters_nb;
	uint32_t ofs_mclq;
	uint32_t size_mclq;
	struct wow_vec3f position;
	uint32_t ofs_mccv;
	uint32_t ofs_mclv;
	uint32_t unused;
};

struct wow_mcnk
{
	struct wow_mcnk_header header;
	struct wow_mcvt mcvt;
	struct wow_mcnr mcnr;
	struct wow_mcly mcly;
	struct wow_mcrf mcrf;
	struct wow_mcal mcal;
	struct wow_mcsh mcsh;
	struct wow_mcse mcse;
	struct wow_mclq mclq;
#if 0
	struct wow_mccv mccv;
	struct wow_mclv mclv;
#endif
};

struct wow_mtex
{
	struct wow_chunk_header header;
	char *data;
	uint32_t size;
};

struct wow_adt_file
{
	char **textures;
	uint32_t textures_nb;
	struct wow_mver mver;
	struct wow_mhdr mhdr;
	struct wow_mcin mcin;
	struct wow_mtex mtex;
	struct wow_mwmo mwmo;
	struct wow_mwid mwid;
	struct wow_modf modf;
	struct wow_mmdx mmdx;
	struct wow_mmid mmid;
	struct wow_mddf mddf;
	struct wow_mfbo mfbo;
	struct wow_mcnk mcnk[16 * 16];
};

struct wow_adt_file *wow_adt_file_new(struct wow_mpq_file *file);
void wow_adt_file_delete(struct wow_adt_file *file);

#ifdef __cplusplus
}
#endif

#endif
