#ifndef WOW_WMO_GROUP_H
#define WOW_WMO_GROUP_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

struct wow_molr
{
	struct wow_chunk_header header;
	uint16_t *data;
	uint32_t size;
};

struct wow_modr
{
	struct wow_chunk_header header;
	uint16_t *data;
	uint32_t size;
};

#define WOW_MOBN_NODE_FLAGS_XAXIS     0x0
#define WOW_MOBN_NODE_FLAGS_YAXIS     0x1
#define WOW_MOBN_NODE_FLAGS_ZAXIS     0x2
#define WOW_MOBN_NODE_FLAGS_AXIS_MASK 0x3
#define WOW_MOBN_NODE_FLAGS_LEAF      0x4
#define WOW_MOBN_NODE_FLAGS_NO_CHILD  0xffff

struct wow_mobn_node
{
	uint16_t flags;
	int16_t neg_child;
	int16_t pos_child;
	uint16_t faces_nb;
	uint32_t face_start;
	float plane_dist;
};

struct wow_mobn
{
	struct wow_chunk_header header;
	struct wow_mobn_node *data;
	uint32_t size;
};

struct wow_mobr
{
	struct wow_chunk_header header;
	uint16_t *data;
	uint32_t size;
};

struct wow_mocv
{
	struct wow_chunk_header header;
	struct wow_vec4b *data;
	uint32_t size;
};

struct wow_mliq_vert
{
	union
	{
		struct
		{
			uint8_t flow1;
			uint8_t flow2;
			uint8_t flow1_pct;
			uint8_t filler;
			float height;
		}  water;
		struct
		{
			int16_t s;
			int16_t t;
			float height;
		} magma;
	};
};

struct wow_mliq_header
{
	struct wow_chunk_header header;
	uint32_t xverts;
	uint32_t yverts;
	uint32_t xtiles;
	uint32_t ytiles;
	struct wow_vec3f coords;
	uint16_t material;
};

#define WOW_MLIQ_TILE_LIQUID_TYPE 0x0F
#define WOW_MLIQ_TILE_FISHABLE    0x40
#define WOW_MLIQ_TILE_SHARED      0x80

struct wow_mliq
{
	struct wow_mliq_header header;
	struct wow_mliq_vert *vertexes;
	uint32_t vertexes_nb;
	uint8_t *tiles;
	uint32_t tiles_nb;
};

struct wow_moba_data
{
	/* 0x00 */ struct wow_vec3s aabb0;
	/* 0x06 */ struct wow_vec3s aabb1;
	/* 0x0C */ uint32_t start_index;
	/* 0x10 */ uint16_t count;
	/* 0x12 */ uint16_t min_index;
	/* 0x14 */ uint16_t max_index;
	/* 0x16 */ uint8_t flag_unknown : 1;
	/* 0x17 */ uint8_t material_id;
};

struct wow_moba
{
	struct wow_chunk_header header;
	struct wow_moba_data *data;
	uint32_t size;
};

struct wow_motv
{
	struct wow_chunk_header header;
	struct wow_vec2f *data;
	uint32_t size;
};

struct wow_monr
{
	struct wow_chunk_header header;
	struct wow_vec3f *data;
	uint32_t size;
};

struct wow_movt
{
	struct wow_chunk_header header;
	struct wow_vec3f *data;
	uint32_t size;
};

struct wow_movi
{
	struct wow_chunk_header header;
	uint16_t *data;
	uint32_t size;
};

#define WOW_MOPY_FLAGS_TRANSPARENT  0x01
#define WOW_MOPY_FLAGS_NOCAMCOLLIDE 0x02
#define WOW_MOPY_FLAGS_DETAIL       0x04
#define WOW_MOPY_FLAGS_COLLISION    0x08
#define WOW_MOPY_FLAGS_HINT         0x10
#define WOW_MOPY_FLAGS_RENDER       0x20
#define WOW_MOPY_FLAGS_UNKNOWN2     0x40
#define WOW_MOPY_FLAGS_COLLIDE_HIT  0x80

struct wow_mopy_data
{
	uint8_t flags;
	uint8_t material_id;
};

struct wow_mopy
{
	struct wow_chunk_header header;
	struct wow_mopy_data *data;
	uint32_t size;
};

#define WOW_MOGP_FLAGS_BSP        0x00000001
#define WOW_MOGP_FLAGS_LIGHT_MAP  0x00000002
#define WOW_MOGP_FLAGS_COLOR      0x00000004
#define WOW_MOGP_FLAGS_OUTDOOR    0x00000008
#define WOW_MOGP_FLAGS_5          0x00000010
#define WOW_MOGP_FLAGS_6          0x00000020
#define WOW_MOGP_FLAGS_EXT_LIGHT  0x00000040
#define WOW_MOGP_FLAGS_8          0x00000080
#define WOW_MOGP_FLAGS_EXT_SKYBOX 0x00000100
#define WOW_MOGP_FLAGS_LIGHT      0x00000200
#define WOW_MOGP_FLAGS_11         0x00000400
#define WOW_MOGP_FLAGS_DOODAD     0x00000800
#define WOW_MOGP_FLAGS_LIQUID     0x00001000
#define WOW_MOGP_FLAGS_INDOOR     0x00002000
#define WOW_MOGP_FLAGS_15         0x00004000
#define WOW_MOGP_FLAGS_16         0x00008000
#define WOW_MOGP_FLAGS_ALWAYSDRAW 0x00010000
#define WOW_MOGP_FLAGS_MORI_MORB  0x00020000
#define WOW_MOGP_FLAGS_SKYBOX     0x00040000
#define WOW_MOGP_FLAGS_OCEAN      0x00080000
#define WOW_MOGP_FLAGS_21         0x00100000
#define WOW_MOGP_FLAGS_MOUNT      0x00200000
#define WOW_MOGP_FLAGS_23         0x00400000
#define WOW_MOGP_FLAGS_24         0x00800000
#define WOW_MOGP_FLAGS_COLOR2     0x01000000
#define WOW_MOGP_FLAGS_MOTV2      0x02000000
#define WOW_MOGP_FLAGS_ANTIPORTAL 0x04000000
#define WOW_MOGP_FLAGS_28         0x08000000
#define WOW_MOGP_FLAGS_29         0x10000000
#define WOW_MOGP_FLAGS_30         0x20000000
#define WOW_MOGP_FLAGS_MOTV3      0x40000000
#define WOW_MOGP_FLAGS_32         0x80000000

struct wow_mogp
{
	/* 0x00 */ struct wow_chunk_header header;
	/* 0x08 */ uint32_t group_name;
	/* 0x0C */ uint32_t descriptive_group_name;
	/* 0x10 */ uint32_t flags;
	/* 0x14 */ struct wow_vec3f aabb0;
	/* 0x20 */ struct wow_vec3f aabb1;
	/* 0x2C */ uint16_t portal_start;
	/* 0x2E */ uint16_t portal_count;
	/* 0x30 */ uint16_t trans_batch_count;
	/* 0x32 */ uint16_t ext_batch_count;
	/* 0x34 */ uint16_t int_batch_count;
	/* 0x36 */ uint16_t padding_or_batch_type_d;
	/* 0x38 */ uint8_t fog_ids[4];
	/* 0x3C */ uint32_t group_liquid;
	/* 0x40 */ uint32_t group_id;
	/* 0x44 */ uint32_t flags2;
	/* 0x48 */ uint32_t unk1;
};

struct wow_wmo_file;

struct wow_wmo_group_file
{
	struct wow_mver mver;
	struct wow_mogp mogp;
	struct wow_mopy mopy;
	struct wow_movi movi;
	struct wow_movt movt;
	struct wow_monr monr;
	struct wow_motv motv;
	struct wow_moba moba;
	struct wow_molr molr;
	struct wow_modr modr;
	struct wow_mobn mobn;
	struct wow_mobr mobr;
	struct wow_mocv mocv;
	struct wow_mliq mliq;
};

struct wow_wmo_group_file *wow_wmo_group_file_new(struct wow_mpq_file *mpq);
void wow_wmo_group_file_delete(struct wow_wmo_group_file *file);

#ifdef __cplusplus
}
#endif

#endif
