#ifndef WOW_DBC_H
#define WOW_DBC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct wow_mpq_file;

struct wow_dbc_header
{
	uint32_t magic;
	uint32_t record_count;
	uint32_t field_count;
	uint32_t record_size;
	uint32_t string_block_size;
};

struct wow_dbc_file;

struct wow_dbc_row
{
	const struct wow_dbc_file *file;
	const uint8_t *ptr;
};

struct wow_dbc_file
{
	struct wow_dbc_header header;
	uint8_t *data;
	uint32_t data_size;
	char *strings;
	uint32_t strings_size;
};

struct wow_dbc_file *wow_dbc_file_new(struct wow_mpq_file *mpq);
void wow_dbc_file_delete(struct wow_dbc_file *file);
struct wow_dbc_row wow_dbc_get_row(const struct wow_dbc_file *file, uint32_t row);
const void *wow_dbc_get_ptr(const struct wow_dbc_row *row, uint32_t column_offset);
int8_t wow_dbc_get_i8(const struct wow_dbc_row *row, uint32_t column_offest);
uint8_t wow_dbc_get_u8(const struct wow_dbc_row *row, uint32_t column_offset);
int16_t wow_dbc_get_i16(const struct wow_dbc_row *row, uint32_t column_offset);
uint16_t wow_dbc_get_u16(const struct wow_dbc_row *row, uint32_t column_offset);
int32_t wow_dbc_get_i32(const struct wow_dbc_row *row, uint32_t column_offset);
uint32_t wow_dbc_get_u32(const struct wow_dbc_row *row, uint32_t column_offset);
int64_t wow_dbc_get_i64(const struct wow_dbc_row *row, uint32_t column_offset);
uint64_t wow_dbc_get_u64(const struct wow_dbc_row *row, uint32_t column_offset);
float wow_dbc_get_flt(const struct wow_dbc_row *row, uint32_t column_offset);
const char *wow_dbc_get_str(const struct wow_dbc_row *row, uint32_t column_offset);

enum wow_dbc_def_type
{
	WOW_DBC_TYPE_I8,
	WOW_DBC_TYPE_U8,
	WOW_DBC_TYPE_I16,
	WOW_DBC_TYPE_U16,
	WOW_DBC_TYPE_I32,
	WOW_DBC_TYPE_U32,
	WOW_DBC_TYPE_I64,
	WOW_DBC_TYPE_U64,
	WOW_DBC_TYPE_STR,
	WOW_DBC_TYPE_LSTR,
	WOW_DBC_TYPE_FLT,
	WOW_DBC_TYPE_END
};

struct wow_dbc_def
{
	enum wow_dbc_def_type type;
	const char *name;
};

extern const struct wow_dbc_def wow_dbc_animation_data_def[];
extern const struct wow_dbc_def wow_dbc_area_poi_def[];
extern const struct wow_dbc_def wow_dbc_area_table_def[];
extern const struct wow_dbc_def wow_dbc_auction_house_def[];
extern const struct wow_dbc_def wow_dbc_char_base_info_def[];
extern const struct wow_dbc_def wow_dbc_char_hair_geosets_def[];
extern const struct wow_dbc_def wow_dbc_char_hair_textures_def[];
extern const struct wow_dbc_def wow_dbc_char_sections_def[];
extern const struct wow_dbc_def wow_dbc_char_start_outfit_def[];
extern const struct wow_dbc_def wow_dbc_char_titles_def[];
extern const struct wow_dbc_def wow_dbc_character_facial_hair_styles_def[];
extern const struct wow_dbc_def wow_dbc_chat_profanity_def[];
extern const struct wow_dbc_def wow_dbc_chr_classes_def[];
extern const struct wow_dbc_def wow_dbc_chr_races_def[];
extern const struct wow_dbc_def wow_dbc_creature_display_info_def[];
extern const struct wow_dbc_def wow_dbc_creature_display_info_extra_def[];
extern const struct wow_dbc_def wow_dbc_creature_model_data_def[];
extern const struct wow_dbc_def wow_dbc_game_object_display_info_def[];
extern const struct wow_dbc_def wow_dbc_ground_effect_texture_def[];
extern const struct wow_dbc_def wow_dbc_ground_effect_doodad_def[];
extern const struct wow_dbc_def wow_dbc_helmet_geoset_vis_data_def[];
extern const struct wow_dbc_def wow_dbc_item_def[];
extern const struct wow_dbc_def wow_dbc_item_class_def[];
extern const struct wow_dbc_def wow_dbc_item_display_info_def[];
extern const struct wow_dbc_def wow_dbc_item_set_def[];
extern const struct wow_dbc_def wow_dbc_item_sub_class_def[];
extern const struct wow_dbc_def wow_dbc_light_def[];
extern const struct wow_dbc_def wow_dbc_light_float_band_def[];
extern const struct wow_dbc_def wow_dbc_light_int_band_def[];
extern const struct wow_dbc_def wow_dbc_light_params_def[];
extern const struct wow_dbc_def wow_dbc_light_skybox_def[];
extern const struct wow_dbc_def wow_dbc_liquid_type_def[];
extern const struct wow_dbc_def wow_dbc_loading_screens_def[];
extern const struct wow_dbc_def wow_dbc_loading_screen_taxi_splines_def[];
extern const struct wow_dbc_def wow_dbc_map_def[];
extern const struct wow_dbc_def wow_dbc_name_gen_def[];
extern const struct wow_dbc_def wow_dbc_sound_entries_def[];
extern const struct wow_dbc_def wow_dbc_spell_def[];
extern const struct wow_dbc_def wow_dbc_spell_icon_def[];
extern const struct wow_dbc_def wow_dbc_talent_def[];
extern const struct wow_dbc_def wow_dbc_talent_tab_def[];
extern const struct wow_dbc_def wow_dbc_taxi_nodes_def[];
extern const struct wow_dbc_def wow_dbc_taxi_path_def[];
extern const struct wow_dbc_def wow_dbc_taxi_path_node_def[];
extern const struct wow_dbc_def wow_dbc_world_map_area_def[];
extern const struct wow_dbc_def wow_dbc_world_map_continent_def[];
extern const struct wow_dbc_def wow_dbc_world_map_overlay_def[];
extern const struct wow_dbc_def wow_dbc_world_map_transforms_def[];
extern const struct wow_dbc_def wow_dbc_wow_error_strings_def[];

#ifdef __cplusplus
}
#endif

#endif
