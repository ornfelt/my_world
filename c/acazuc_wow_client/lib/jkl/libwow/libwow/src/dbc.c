#include "dbc.h"

#include "common.h"
#include "mpq.h"

#include <stdlib.h>
#include <string.h>

struct wow_dbc_file *
wow_dbc_file_new(struct wow_mpq_file *mpq)
{
	struct wow_dbc_file *file;

	file = WOW_MALLOC(sizeof(*file));
	if (!file)
		return NULL;
	memset(file, 0, sizeof(*file));
	if (wow_mpq_read(mpq, &file->header, sizeof(file->header)) != sizeof(file->header))
		goto err;
	file->data_size = file->header.record_count * file->header.record_size;
	file->data = WOW_MALLOC(file->data_size);
	if (!file->data)
		goto err;
	if (wow_mpq_read(mpq, file->data, file->data_size) != file->data_size)
		goto err;
	file->strings_size = mpq->size - sizeof(file->header) - file->header.record_count * file->header.record_size;
	file->strings = WOW_MALLOC(file->strings_size);
	if (!file->strings)
		goto err;
	if (wow_mpq_read(mpq, file->strings, file->strings_size) != file->strings_size)
		goto err;
	return file;

err:
	wow_dbc_file_delete(file);
	return NULL;
}

void
wow_dbc_file_delete(struct wow_dbc_file *file)
{
	if (!file)
		return;
	WOW_FREE(file->data);
	WOW_FREE(file->strings);
	WOW_FREE(file);
}

struct wow_dbc_row
wow_dbc_get_row(const struct wow_dbc_file *file, uint32_t row)
{
	struct wow_dbc_row ret;

	ret.file = file;
	ret.ptr = file->data + file->header.record_size * row;
	return ret;
}

const void *
wow_dbc_get_ptr(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return row->ptr + column_offset;
}

int8_t
wow_dbc_get_i8(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(int8_t*)wow_dbc_get_ptr(row, column_offset);
}

uint8_t
wow_dbc_get_u8(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(uint8_t*)wow_dbc_get_ptr(row, column_offset);
}

int16_t
wow_dbc_get_i16(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(int16_t*)wow_dbc_get_ptr(row, column_offset);
}

uint16_t
wow_dbc_get_u16(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(uint16_t*)wow_dbc_get_ptr(row, column_offset);
}

int32_t
wow_dbc_get_i32(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(int32_t*)wow_dbc_get_ptr(row, column_offset);
}

uint32_t
wow_dbc_get_u32(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(uint32_t*)wow_dbc_get_ptr(row, column_offset);
}

int64_t
wow_dbc_get_i64(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(int64_t*)wow_dbc_get_ptr(row, column_offset);
}

uint64_t
wow_dbc_get_u64(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(uint64_t*)wow_dbc_get_ptr(row, column_offset);
}

float
wow_dbc_get_flt(const struct wow_dbc_row *row, uint32_t column_offset)
{
	return *(float*)wow_dbc_get_ptr(row, column_offset);
}

const char *
wow_dbc_get_str(const struct wow_dbc_row *row, uint32_t column_offset)
{
	uint32_t offset = wow_dbc_get_u32(row, column_offset);
	if (offset >= row->file->strings_size)
		return NULL;
	return &row->file->strings[offset];
}

const struct wow_dbc_def
wow_dbc_animation_data_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_STR, "name"},
	/*  8 */ {WOW_DBC_TYPE_U32, "weapon_flags"},
	/* 12 */ {WOW_DBC_TYPE_U32, "body_flags"},
	/* 16 */ {WOW_DBC_TYPE_U32, "flags"},
	/* 20 */ {WOW_DBC_TYPE_U32, "fallback"},
	/* 24 */ {WOW_DBC_TYPE_U32, "previous"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_area_poi_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "importance"},
	/*   8 */ {WOW_DBC_TYPE_U32, "icon"},
	/*  12 */ {WOW_DBC_TYPE_U32, "faction"},
	/*  16 */ {WOW_DBC_TYPE_FLT, "x"},
	/*  20 */ {WOW_DBC_TYPE_FLT, "y"},
	/*  24 */ {WOW_DBC_TYPE_FLT, "z"},
	/*  28 */ {WOW_DBC_TYPE_U32, "map"},
	/*  32 */ {WOW_DBC_TYPE_U32, "flags"},
	/*  36 */ {WOW_DBC_TYPE_U32, "area"},
	/*  40 */ {WOW_DBC_TYPE_LSTR, "name"},
	/* 108 */ {WOW_DBC_TYPE_LSTR, "description"},
	/* 176 */ {WOW_DBC_TYPE_U32, "world_state"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_area_table_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "map"},
	/*   8 */ {WOW_DBC_TYPE_U32, "area"},
	/*  12 */ {WOW_DBC_TYPE_U32, "area_bit"},
	/*  16 */ {WOW_DBC_TYPE_U32, "flags"},
	/*  20 */ {WOW_DBC_TYPE_U32, "sound_provider"},
	/*  24 */ {WOW_DBC_TYPE_U32, "sound_provider_water"},
	/*  28 */ {WOW_DBC_TYPE_U32, "sound_ambience"},
	/*  32 */ {WOW_DBC_TYPE_U32, "zone_music"},
	/*  36 */ {WOW_DBC_TYPE_U32, "zone_intro"},
	/*  40 */ {WOW_DBC_TYPE_U32, "exploration_level"},
	/*  44 */ {WOW_DBC_TYPE_LSTR, "name"},
	/* 112 */ {WOW_DBC_TYPE_U32, "faction_group"},
	/* 116 */ {WOW_DBC_TYPE_U32, "liquid0"},
	/* 120 */ {WOW_DBC_TYPE_U32, "liquid1"},
	/* 124 */ {WOW_DBC_TYPE_U32, "liquid2"},
	/* 128 */ {WOW_DBC_TYPE_U32, "liquid3"},
	/* 132 */ {WOW_DBC_TYPE_FLT, "min_elevation"},
	/* 136 */ {WOW_DBC_TYPE_FLT, "max_elevation"},
	/* 140 */ {WOW_DBC_TYPE_U32, "light"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_auction_house_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "faction"},
	/*  8 */ {WOW_DBC_TYPE_U32, "deposit_rate"},
	/* 12 */ {WOW_DBC_TYPE_U32, "consigment_rate"},
	/* 16 */ {WOW_DBC_TYPE_LSTR, "name"},
	         {WOW_DBC_TYPE_END, ""},
};

const struct wow_dbc_def
wow_dbc_char_base_info_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U8, "race"},
	/* 1 */ {WOW_DBC_TYPE_U8, "class"},
	        {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_char_hair_geosets_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "race"},
	/*  8 */ {WOW_DBC_TYPE_U32, "gender"},
	/* 12 */ {WOW_DBC_TYPE_U32, "variation"},
	/* 16 */ {WOW_DBC_TYPE_U32, "geoset"},
	/* 20 */ {WOW_DBC_TYPE_U32, "bald"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_char_hair_textures_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "race"},
	/*  8 */ {WOW_DBC_TYPE_U32, "gender"},
	/* 12 */ {WOW_DBC_TYPE_U32, "unk1"},
	/* 16 */ {WOW_DBC_TYPE_U32, "unk2"},
	/* 20 */ {WOW_DBC_TYPE_I32, "unk3"},
	/* 24 */ {WOW_DBC_TYPE_I32, "unk4"},
	/* 28 */ {WOW_DBC_TYPE_I32, "unk5"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_char_sections_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "race"},
	/*  8 */ {WOW_DBC_TYPE_U32, "gender"},
	/* 12 */ {WOW_DBC_TYPE_U32, "base_section"},
	/* 16 */ {WOW_DBC_TYPE_U32, "variation"},
	/* 20 */ {WOW_DBC_TYPE_U32, "color_index"},
	/* 24 */ {WOW_DBC_TYPE_STR, "texture1"},
	/* 28 */ {WOW_DBC_TYPE_STR, "texture2"},
	/* 32 */ {WOW_DBC_TYPE_STR, "texture3"},
	/* 36 */ {WOW_DBC_TYPE_U32, "flags"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_char_start_outfit_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U8, "race"},
	/*   5 */ {WOW_DBC_TYPE_U8, "class"},
	/*   6 */ {WOW_DBC_TYPE_U8, "gender"},
	/*   7 */ {WOW_DBC_TYPE_U8, "outfit_id"},
	/*   8 */ {WOW_DBC_TYPE_U32, "item1"},
	/*  12 */ {WOW_DBC_TYPE_U32, "item2"},
	/*  16 */ {WOW_DBC_TYPE_U32, "item3"},
	/*  20 */ {WOW_DBC_TYPE_U32, "item4"},
	/*  24 */ {WOW_DBC_TYPE_U32, "item5"},
	/*  28 */ {WOW_DBC_TYPE_U32, "item6"},
	/*  32 */ {WOW_DBC_TYPE_U32, "item7"},
	/*  36 */ {WOW_DBC_TYPE_U32, "item8"},
	/*  40 */ {WOW_DBC_TYPE_U32, "item9"},
	/*  44 */ {WOW_DBC_TYPE_U32, "item1"},
	/*  48 */ {WOW_DBC_TYPE_U32, "item10"},
	/*  52 */ {WOW_DBC_TYPE_U32, "item11"},
	/*  56 */ {WOW_DBC_TYPE_U32, "item12"},
	/*  60 */ {WOW_DBC_TYPE_U32, "display_info1"},
	/*  64 */ {WOW_DBC_TYPE_U32, "display_info2"},
	/*  68 */ {WOW_DBC_TYPE_U32, "display_info3"},
	/*  72 */ {WOW_DBC_TYPE_U32, "display_info4"},
	/*  76 */ {WOW_DBC_TYPE_U32, "display_info5"},
	/*  80 */ {WOW_DBC_TYPE_U32, "display_info6"},
	/*  84 */ {WOW_DBC_TYPE_U32, "display_info7"},
	/*  88 */ {WOW_DBC_TYPE_U32, "display_info8"},
	/*  92 */ {WOW_DBC_TYPE_U32, "display_info9"},
	/*  96 */ {WOW_DBC_TYPE_U32, "display_info1"},
	/* 100 */ {WOW_DBC_TYPE_U32, "display_info10"},
	/* 104 */ {WOW_DBC_TYPE_U32, "display_info11"},
	/* 108 */ {WOW_DBC_TYPE_U32, "display_info12"},
	/* 112 */ {WOW_DBC_TYPE_U32, "inventory_type1"},
	/* 116 */ {WOW_DBC_TYPE_U32, "inventory_type2"},
	/* 120 */ {WOW_DBC_TYPE_U32, "inventory_type3"},
	/* 124 */ {WOW_DBC_TYPE_U32, "inventory_type4"},
	/* 128 */ {WOW_DBC_TYPE_U32, "inventory_type5"},
	/* 132 */ {WOW_DBC_TYPE_U32, "inventory_type6"},
	/* 136 */ {WOW_DBC_TYPE_U32, "inventory_type7"},
	/* 140 */ {WOW_DBC_TYPE_U32, "inventory_type8"},
	/* 144 */ {WOW_DBC_TYPE_U32, "inventory_type9"},
	/* 148 */ {WOW_DBC_TYPE_U32, "inventory_type1"},
	/* 152 */ {WOW_DBC_TYPE_U32, "inventory_type10"},
	/* 156 */ {WOW_DBC_TYPE_U32, "inventory_type11"},
	/* 160 */ {WOW_DBC_TYPE_U32, "inventory_type12"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_char_titles_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "unk"},
	/*   8 */ {WOW_DBC_TYPE_LSTR, "male_name"},
	/*  76 */ {WOW_DBC_TYPE_LSTR, "female_name"},
	/* 144 */ {WOW_DBC_TYPE_U32, "mask_id"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_character_facial_hair_styles_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "race"},
	/*  4 */ {WOW_DBC_TYPE_U32, "gender"},
	/*  8 */ {WOW_DBC_TYPE_U32, "variation"},
	/* 12 */ {WOW_DBC_TYPE_U32, "unk1"},
	/* 16 */ {WOW_DBC_TYPE_U32, "unk2"},
	/* 20 */ {WOW_DBC_TYPE_U32, "unk3"},
	/* 24 */ {WOW_DBC_TYPE_U32, "geoset1"},
	/* 28 */ {WOW_DBC_TYPE_U32, "geoset2"},
	/* 32 */ {WOW_DBC_TYPE_U32, "geoset3"},
	/* 36 */ {WOW_DBC_TYPE_U32, "geoset4"},
	/* 40 */ {WOW_DBC_TYPE_U32, "geoset5"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_chat_profanity_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "word"},
	/* 8 */ {WOW_DBC_TYPE_U32, "language"},
	        {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_chr_classes_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "unknown"},
	/*   8 */ {WOW_DBC_TYPE_U32, "power_type"},
	/*  12 */ {WOW_DBC_TYPE_STR, "pet_name_token"},
	/*  16 */ {WOW_DBC_TYPE_LSTR, "neutral_name"},
	/*  84 */ {WOW_DBC_TYPE_LSTR, "female_name"},
	/* 152 */ {WOW_DBC_TYPE_LSTR, "male_name"},
	/* 220 */ {WOW_DBC_TYPE_U32, "spell_class_set"},
	/* 224 */ {WOW_DBC_TYPE_U32, "flags"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_chr_races_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "flags"},
	/*   8 */ {WOW_DBC_TYPE_U32, "faction"},
	/*  12 */ {WOW_DBC_TYPE_U32, "exploration_sound"},
	/*  16 */ {WOW_DBC_TYPE_U32, "male_display_id"},
	/*  20 */ {WOW_DBC_TYPE_U32, "female_display_id"},
	/*  24 */ {WOW_DBC_TYPE_STR, "client_prefix"},
	/*  28 */ {WOW_DBC_TYPE_FLT, "mount_scale"},
	/*  32 */ {WOW_DBC_TYPE_U32, "base_langage"},
	/*  36 */ {WOW_DBC_TYPE_U32, "creature_type"},
	/*  40 */ {WOW_DBC_TYPE_U32, "login_spell"},
	/*  44 */ {WOW_DBC_TYPE_U32, "combat_spell"},
	/*  48 */ {WOW_DBC_TYPE_U32, "res_sick_spell"},
	/*  52 */ {WOW_DBC_TYPE_U32, "splash_sound"},
	/*  56 */ {WOW_DBC_TYPE_LSTR, "neutral_name"},
	/* 124 */ {WOW_DBC_TYPE_LSTR, "female_name"},
	/* 192 */ {WOW_DBC_TYPE_LSTR, "male_name"},
	/* 260 */ {WOW_DBC_TYPE_STR, "facial_hair_customization"},
	/* 264 */ {WOW_DBC_TYPE_STR, "facial_hair_customization_internal"},
	/* 268 */ {WOW_DBC_TYPE_STR, "hair_customization"},
	/* 272 */ {WOW_DBC_TYPE_U32, "required_expansion"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_creature_display_info_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "model"},
	/*  8 */ {WOW_DBC_TYPE_U32, "sound"},
	/* 12 */ {WOW_DBC_TYPE_U32, "extra_display_information"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "scale"},
	/* 20 */ {WOW_DBC_TYPE_U32, "opacity"},
	/* 24 */ {WOW_DBC_TYPE_STR, "texture1"},
	/* 28 */ {WOW_DBC_TYPE_STR, "texture2"},
	/* 32 */ {WOW_DBC_TYPE_STR, "texture3"},
	/* 36 */ {WOW_DBC_TYPE_STR, "texture4"},
	/* 40 */ {WOW_DBC_TYPE_STR, "portait_texture"},
	/* 44 */ {WOW_DBC_TYPE_U32, "blood_level"},
	/* 48 */ {WOW_DBC_TYPE_U32, "blood"},
	/* 52 */ {WOW_DBC_TYPE_U32, "npc_sounds"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_creature_display_info_extra_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "race"},
	/*  8 */ {WOW_DBC_TYPE_U32, "gender"},
	/* 12 */ {WOW_DBC_TYPE_U32, "skin"},
	/* 16 */ {WOW_DBC_TYPE_U32, "face"},
	/* 20 */ {WOW_DBC_TYPE_U32, "hair_style"},
	/* 24 */ {WOW_DBC_TYPE_U32, "hair_color"},
	/* 28 */ {WOW_DBC_TYPE_U32, "facial_hair"},
	/* 32 */ {WOW_DBC_TYPE_U32, "item_helm"},
	/* 36 */ {WOW_DBC_TYPE_U32, "item_shoulder"},
	/* 40 */ {WOW_DBC_TYPE_U32, "item_shirt"},
	/* 44 */ {WOW_DBC_TYPE_U32, "item_cuirass"},
	/* 48 */ {WOW_DBC_TYPE_U32, "item_belt"},
	/* 52 */ {WOW_DBC_TYPE_U32, "item_legs"},
	/* 56 */ {WOW_DBC_TYPE_U32, "item_boots"},
	/* 60 */ {WOW_DBC_TYPE_U32, "item_wrists"},
	/* 64 */ {WOW_DBC_TYPE_U32, "item_gloves"},
	/* 68 */ {WOW_DBC_TYPE_U32, "item_tabard"},
	/* 72 */ {WOW_DBC_TYPE_U32, "item_cape"},
	/* 76 */ {WOW_DBC_TYPE_U32, "flags"},
	/* 80 */ {WOW_DBC_TYPE_STR, "baked_texture"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_creature_model_data_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "flags"},
	/*  8 */ {WOW_DBC_TYPE_STR, "model"},
	/* 12 */ {WOW_DBC_TYPE_U32, "size_class"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "scale"},
	/* 20 */ {WOW_DBC_TYPE_U32, "blood"},
	/* 24 */ {WOW_DBC_TYPE_U32, "footprint"},
	/* 28 */ {WOW_DBC_TYPE_FLT, "footprint_texture_length"},
	/* 32 */ {WOW_DBC_TYPE_FLT, "footprint_texture_width"},
	/* 36 */ {WOW_DBC_TYPE_FLT, "footprint_texture_scale"},
	/* 40 */ {WOW_DBC_TYPE_U32, "foley_material"},
	/* 44 */ {WOW_DBC_TYPE_U32, "footstep_shake_size"},
	/* 48 */ {WOW_DBC_TYPE_U32, "death_thud_shake_size"},
	/* 52 */ {WOW_DBC_TYPE_U32, "sound"},
	/* 56 */ {WOW_DBC_TYPE_FLT, "collision_width"},
	/* 60 */ {WOW_DBC_TYPE_FLT, "collision_height"},
	/* 64 */ {WOW_DBC_TYPE_FLT, "mount_height"},
	/* 68 */ {WOW_DBC_TYPE_FLT, "geobox_min_x"},
	/* 72 */ {WOW_DBC_TYPE_FLT, "geobox_min_y"},
	/* 76 */ {WOW_DBC_TYPE_FLT, "geobox_min_z"},
	/* 80 */ {WOW_DBC_TYPE_FLT, "geobox_max_x"},
	/* 84 */ {WOW_DBC_TYPE_FLT, "geobox_max_y"},
	/* 88 */ {WOW_DBC_TYPE_FLT, "geobox_max_z"},
	/* 92 */ {WOW_DBC_TYPE_FLT, "world_effect_scale"},
	/* 96 */ {WOW_DBC_TYPE_FLT, "attached_effect_scale"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_declined_word_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "word"},
	        {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_declined_word_cases_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "declined_word"},
	/*  8 */ {WOW_DBC_TYPE_U32, "case_index"},
	/* 12 */ {WOW_DBC_TYPE_STR, "word"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_game_object_display_info_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_STR, "model"},
	/*  8 */ {WOW_DBC_TYPE_U32, "sound_stand"},
	/* 12 */ {WOW_DBC_TYPE_U32, "sound_open"},
	/* 16 */ {WOW_DBC_TYPE_U32, "sound_loop"},
	/* 20 */ {WOW_DBC_TYPE_U32, "sound_close"},
	/* 24 */ {WOW_DBC_TYPE_U32, "sound_destroy"},
	/* 30 */ {WOW_DBC_TYPE_U32, "sound_opened"},
	/* 34 */ {WOW_DBC_TYPE_U32, "sound_custom1"},
	/* 38 */ {WOW_DBC_TYPE_U32, "sound_custom2"},
	/* 42 */ {WOW_DBC_TYPE_U32, "sound_custom3"},
	/* 46 */ {WOW_DBC_TYPE_U32, "sound_custom4"},
	/* 50 */ {WOW_DBC_TYPE_FLT, "geobox_min_x"},
	/* 54 */ {WOW_DBC_TYPE_FLT, "geobox_min_y"},
	/* 58 */ {WOW_DBC_TYPE_FLT, "geobox_min_z"},
	/* 62 */ {WOW_DBC_TYPE_FLT, "geobox_max_x"},
	/* 66 */ {WOW_DBC_TYPE_FLT, "geobox_max_y"},
	/* 70 */ {WOW_DBC_TYPE_FLT, "geobox_max_z"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_ground_effect_texture_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "doodad1"},
	/*  8 */ {WOW_DBC_TYPE_U32, "doodad2"},
	/* 12 */ {WOW_DBC_TYPE_U32, "doodad3"},
	/* 16 */ {WOW_DBC_TYPE_U32, "doodad4"},
	/* 20 */ {WOW_DBC_TYPE_U32, "density"},
	/* 24 */ {WOW_DBC_TYPE_U32, "sound"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_ground_effect_doodad_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_U32, "internal_id"},
	/* 8 */ {WOW_DBC_TYPE_STR, "model"},
	        {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_helmet_geoset_vis_data_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "hair_flags"},
	/*  8 */ {WOW_DBC_TYPE_U32, "facial1_flags"},
	/* 12 */ {WOW_DBC_TYPE_U32, "facial2_flags"},
	/* 16 */ {WOW_DBC_TYPE_U32, "facial3_flags"},
	/* 20 */ {WOW_DBC_TYPE_U32, "ears_flags"},
	/* 24 */ {WOW_DBC_TYPE_U32, "misc_flags"},
	/* 28 */ {WOW_DBC_TYPE_U32, "eyes_flags"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_item_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "display_info"},
	/*  8 */ {WOW_DBC_TYPE_U32, "slot_id"},
	/* 12 */ {WOW_DBC_TYPE_U32, "class"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_item_class_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "subclass"},
	/*  8 */ {WOW_DBC_TYPE_U32, "flags"},
	/* 12 */ {WOW_DBC_TYPE_LSTR, "name"},
                 {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_item_display_info_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_STR, "model_name_left"},
	/*  8 */ {WOW_DBC_TYPE_STR, "model_name_right"},
	/* 12 */ {WOW_DBC_TYPE_STR, "model_texture_left"},
	/* 16 */ {WOW_DBC_TYPE_STR, "model_texture_right"},
	/* 20 */ {WOW_DBC_TYPE_STR, "icon1"},
	/* 24 */ {WOW_DBC_TYPE_STR, "icon2"},
	/* 28 */ {WOW_DBC_TYPE_U32, "geoset_group1"},
	/* 32 */ {WOW_DBC_TYPE_U32, "geoset_group2"},
	/* 36 */ {WOW_DBC_TYPE_U32, "geoset_group3"},
	/* 40 */ {WOW_DBC_TYPE_U32, "flags"},
	/* 44 */ {WOW_DBC_TYPE_U32, "spell_visual_id"},
	/* 48 */ {WOW_DBC_TYPE_U32, "group_sound_index"},
	/* 52 */ {WOW_DBC_TYPE_U32, "helmet_geoset_visual_id_male"},
	/* 56 */ {WOW_DBC_TYPE_U32, "helmet_geoset_visual_id_female"},
	/* 60 */ {WOW_DBC_TYPE_STR, "texture_upper_arm"},
	/* 64 */ {WOW_DBC_TYPE_STR, "texture_lower_arm"},
	/* 68 */ {WOW_DBC_TYPE_STR, "texture_hands"},
	/* 72 */ {WOW_DBC_TYPE_STR, "texture_upper_torso"},
	/* 76 */ {WOW_DBC_TYPE_STR, "texture_lower_torso"},
	/* 80 */ {WOW_DBC_TYPE_STR, "texture_upper_leg"},
	/* 84 */ {WOW_DBC_TYPE_STR, "texture_lower_leg"},
	/* 88 */ {WOW_DBC_TYPE_STR, "texture_foot"},
	/* 92 */ {WOW_DBC_TYPE_U32, "item_visual"},
	/* 96 */ {WOW_DBC_TYPE_U32, "particle_color_id"},
                 {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_item_set_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_LSTR, "name"},
	/*  72 */ {WOW_DBC_TYPE_U32, "item1"},
	/*  76 */ {WOW_DBC_TYPE_U32, "item2"},
	/*  80 */ {WOW_DBC_TYPE_U32, "item3"},
	/*  84 */ {WOW_DBC_TYPE_U32, "item4"},
	/*  88 */ {WOW_DBC_TYPE_U32, "item5"},
	/*  92 */ {WOW_DBC_TYPE_U32, "item6"},
	/*  96 */ {WOW_DBC_TYPE_U32, "item7"},
	/* 100 */ {WOW_DBC_TYPE_U32, "item8"},
	/* 104 */ {WOW_DBC_TYPE_U32, "item9"},
	/* 108 */ {WOW_DBC_TYPE_U32, "item10"},
	/* 112 */ {WOW_DBC_TYPE_U32, "item11"},
	/* 116 */ {WOW_DBC_TYPE_U32, "item12"},
	/* 120 */ {WOW_DBC_TYPE_U32, "item13"},
	/* 124 */ {WOW_DBC_TYPE_U32, "item14"},
	/* 128 */ {WOW_DBC_TYPE_U32, "item15"},
	/* 132 */ {WOW_DBC_TYPE_U32, "item16"},
	/* 136 */ {WOW_DBC_TYPE_U32, "item17"},
	/* 140 */ {WOW_DBC_TYPE_U32, "spell1"},
	/* 144 */ {WOW_DBC_TYPE_U32, "spell2"},
	/* 148 */ {WOW_DBC_TYPE_U32, "spell3"},
	/* 152 */ {WOW_DBC_TYPE_U32, "spell4"},
	/* 156 */ {WOW_DBC_TYPE_U32, "spell5"},
	/* 160 */ {WOW_DBC_TYPE_U32, "spell6"},
	/* 164 */ {WOW_DBC_TYPE_U32, "spell7"},
	/* 168 */ {WOW_DBC_TYPE_U32, "spell8"},
	/* 172 */ {WOW_DBC_TYPE_U32, "threshold1"},
	/* 176 */ {WOW_DBC_TYPE_U32, "threshold2"},
	/* 180 */ {WOW_DBC_TYPE_U32, "threshold3"},
	/* 184 */ {WOW_DBC_TYPE_U32, "threshold4"},
	/* 188 */ {WOW_DBC_TYPE_U32, "threshold5"},
	/* 192 */ {WOW_DBC_TYPE_U32, "threshold6"},
	/* 196 */ {WOW_DBC_TYPE_U32, "threshold7"},
	/* 200 */ {WOW_DBC_TYPE_U32, "threshold8"},
	/* 204 */ {WOW_DBC_TYPE_U32, "required_skill"},
	/* 208 */ {WOW_DBC_TYPE_U32, "required_skill_rank"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_item_sub_class_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "class_id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "sub_class_id"},
	/*   8 */ {WOW_DBC_TYPE_U32, "prerequisite_proficiency"},
	/*  12 */ {WOW_DBC_TYPE_U32, "postrequisite_proficiency"},
	/*  16 */ {WOW_DBC_TYPE_U32, "flags"},
	/*  20 */ {WOW_DBC_TYPE_U32, "display_flags"},
	/*  24 */ {WOW_DBC_TYPE_U32, "weapon_parry_seq"},
	/*  28 */ {WOW_DBC_TYPE_U32, "weapon_ready_seq"},
	/*  32 */ {WOW_DBC_TYPE_U32, "weapon_attack_seq"},
	/*  36 */ {WOW_DBC_TYPE_U32, "weapon_swing_size"},
	/*  40 */ {WOW_DBC_TYPE_LSTR, "name"},
	/* 229 */ {WOW_DBC_TYPE_LSTR, "verbose_name"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_language_words_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_U32, "language"},
	/* 8 */ {WOW_DBC_TYPE_STR, "word"},
	        {WOW_DBC_TYPE_END, ""},
};

const struct wow_dbc_def
wow_dbc_languages_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "name"},
	        {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_light_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "continent"},
	/*  8 */ {WOW_DBC_TYPE_FLT, "x"},
	/* 12 */ {WOW_DBC_TYPE_FLT, "y"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "z"},
	/* 20 */ {WOW_DBC_TYPE_FLT, "falloff_start"},
	/* 24 */ {WOW_DBC_TYPE_FLT, "falloff_end"},
	/* 28 */ {WOW_DBC_TYPE_U32, "params_clear"},
	/* 32 */ {WOW_DBC_TYPE_U32, "params_clear_water"},
	/* 36 */ {WOW_DBC_TYPE_U32, "params_storm"},
	/* 40 */ {WOW_DBC_TYPE_U32, "params_storm_water"},
	/* 44 */ {WOW_DBC_TYPE_U32, "params_death"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_light_float_band_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "count"},
	/*   8 */ {WOW_DBC_TYPE_U32, "time1"},
	/*  12 */ {WOW_DBC_TYPE_U32, "time2"},
	/*  16 */ {WOW_DBC_TYPE_U32, "time3"},
	/*  20 */ {WOW_DBC_TYPE_U32, "time4"},
	/*  24 */ {WOW_DBC_TYPE_U32, "time5"},
	/*  28 */ {WOW_DBC_TYPE_U32, "time6"},
	/*  32 */ {WOW_DBC_TYPE_U32, "time7"},
	/*  36 */ {WOW_DBC_TYPE_U32, "time8"},
	/*  40 */ {WOW_DBC_TYPE_U32, "time9"},
	/*  44 */ {WOW_DBC_TYPE_U32, "time10"},
	/*  48 */ {WOW_DBC_TYPE_U32, "time11"},
	/*  52 */ {WOW_DBC_TYPE_U32, "time12"},
	/*  56 */ {WOW_DBC_TYPE_U32, "time13"},
	/*  60 */ {WOW_DBC_TYPE_U32, "time14"},
	/*  64 */ {WOW_DBC_TYPE_U32, "time15"},
	/*  68 */ {WOW_DBC_TYPE_U32, "time16"},
	/*  72 */ {WOW_DBC_TYPE_FLT, "value1"},
	/*  76 */ {WOW_DBC_TYPE_FLT, "value2"},
	/*  80 */ {WOW_DBC_TYPE_FLT, "value3"},
	/*  84 */ {WOW_DBC_TYPE_FLT, "value4"},
	/*  88 */ {WOW_DBC_TYPE_FLT, "value5"},
	/*  92 */ {WOW_DBC_TYPE_FLT, "value6"},
	/*  96 */ {WOW_DBC_TYPE_FLT, "value7"},
	/* 100 */ {WOW_DBC_TYPE_FLT, "value8"},
	/* 104 */ {WOW_DBC_TYPE_FLT, "value9"},
	/* 108 */ {WOW_DBC_TYPE_FLT, "value10"},
	/* 112 */ {WOW_DBC_TYPE_FLT, "value11"},
	/* 116 */ {WOW_DBC_TYPE_FLT, "value12"},
	/* 120 */ {WOW_DBC_TYPE_FLT, "value13"},
	/* 124 */ {WOW_DBC_TYPE_FLT, "value14"},
	/* 128 */ {WOW_DBC_TYPE_FLT, "value15"},
	/* 132 */ {WOW_DBC_TYPE_FLT, "value16"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_light_int_band_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "count"},
	/*   8 */ {WOW_DBC_TYPE_U32, "time1"},
	/*  12 */ {WOW_DBC_TYPE_U32, "time2"},
	/*  16 */ {WOW_DBC_TYPE_U32, "time3"},
	/*  20 */ {WOW_DBC_TYPE_U32, "time4"},
	/*  24 */ {WOW_DBC_TYPE_U32, "time5"},
	/*  28 */ {WOW_DBC_TYPE_U32, "time6"},
	/*  32 */ {WOW_DBC_TYPE_U32, "time7"},
	/*  36 */ {WOW_DBC_TYPE_U32, "time8"},
	/*  40 */ {WOW_DBC_TYPE_U32, "time9"},
	/*  44 */ {WOW_DBC_TYPE_U32, "time10"},
	/*  48 */ {WOW_DBC_TYPE_U32, "time11"},
	/*  52 */ {WOW_DBC_TYPE_U32, "time12"},
	/*  56 */ {WOW_DBC_TYPE_U32, "time13"},
	/*  60 */ {WOW_DBC_TYPE_U32, "time14"},
	/*  64 */ {WOW_DBC_TYPE_U32, "time15"},
	/*  68 */ {WOW_DBC_TYPE_U32, "time16"},
	/*  72 */ {WOW_DBC_TYPE_U32, "value1"},
	/*  76 */ {WOW_DBC_TYPE_U32, "value2"},
	/*  80 */ {WOW_DBC_TYPE_U32, "value3"},
	/*  84 */ {WOW_DBC_TYPE_U32, "value4"},
	/*  88 */ {WOW_DBC_TYPE_U32, "value5"},
	/*  92 */ {WOW_DBC_TYPE_U32, "value6"},
	/*  96 */ {WOW_DBC_TYPE_U32, "value7"},
	/* 100 */ {WOW_DBC_TYPE_U32, "value8"},
	/* 104 */ {WOW_DBC_TYPE_U32, "value9"},
	/* 108 */ {WOW_DBC_TYPE_U32, "value10"},
	/* 112 */ {WOW_DBC_TYPE_U32, "value11"},
	/* 116 */ {WOW_DBC_TYPE_U32, "value12"},
	/* 120 */ {WOW_DBC_TYPE_U32, "value13"},
	/* 124 */ {WOW_DBC_TYPE_U32, "value14"},
	/* 128 */ {WOW_DBC_TYPE_U32, "value15"},
	/* 132 */ {WOW_DBC_TYPE_U32, "value16"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_light_params_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "highlight_sky"},
	/*  8 */ {WOW_DBC_TYPE_U32, "light_skybox_id"},
	/* 12 */ {WOW_DBC_TYPE_U32, "cloud_type_id"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "glow"},
	/* 20 */ {WOW_DBC_TYPE_FLT, "water_shallow_alpha"},
	/* 24 */ {WOW_DBC_TYPE_FLT, "water_deep_alpha"},
	/* 28 */ {WOW_DBC_TYPE_FLT, "ocean_shallow_alpha"},
	/* 32 */ {WOW_DBC_TYPE_FLT, "ocean_deep_alpha"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_light_skybox_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "model"},
	/* 8 */ {WOW_DBC_TYPE_U32, "flags"},
	        {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_liquid_type_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_STR, "name"},
	/*  8 */ {WOW_DBC_TYPE_U32, "liquid_id"},
	/* 12 */ {WOW_DBC_TYPE_U32, "spell"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_loading_screens_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "name"},
	/* 8 */ {WOW_DBC_TYPE_STR, "filename"},
	        {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_loading_screen_taxi_splines_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "path_id"},
	/*  8 */ {WOW_DBC_TYPE_FLT, "x1"},
	/* 12 */ {WOW_DBC_TYPE_FLT, "x2"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "x3"},
	/* 20 */ {WOW_DBC_TYPE_FLT, "x4"},
	/* 24 */ {WOW_DBC_TYPE_FLT, "x5"},
	/* 28 */ {WOW_DBC_TYPE_FLT, "x6"},
	/* 32 */ {WOW_DBC_TYPE_FLT, "x7"},
	/* 36 */ {WOW_DBC_TYPE_FLT, "x8"},
	/* 40 */ {WOW_DBC_TYPE_FLT, "y1"},
	/* 44 */ {WOW_DBC_TYPE_FLT, "y2"},
	/* 48 */ {WOW_DBC_TYPE_FLT, "y3"},
	/* 52 */ {WOW_DBC_TYPE_FLT, "y4"},
	/* 56 */ {WOW_DBC_TYPE_FLT, "y5"},
	/* 60 */ {WOW_DBC_TYPE_FLT, "y6"},
	/* 64 */ {WOW_DBC_TYPE_FLT, "y7"},
	/* 68 */ {WOW_DBC_TYPE_FLT, "y8"},
	/* 72 */ {WOW_DBC_TYPE_U32, "leg_index"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_map_def[] =
{
	/*   0 */  {WOW_DBC_TYPE_U32, "id"},
	/*   4 */  {WOW_DBC_TYPE_STR, "directory"},
	/*   8 */  {WOW_DBC_TYPE_U32, "instance_type"},
	/*  12 */ {WOW_DBC_TYPE_U32, "map_type"},
	/*  16 */ {WOW_DBC_TYPE_LSTR, "name"},
	/*  84 */ {WOW_DBC_TYPE_U32, "min_level"},
	/*  88 */ {WOW_DBC_TYPE_U32, "max_level"},
	/*  92 */ {WOW_DBC_TYPE_U32, "max_players"},
	/*  96 */ {WOW_DBC_TYPE_I32, "unk"},
	/* 100 */ {WOW_DBC_TYPE_FLT, "unk"},
	/* 104 */ {WOW_DBC_TYPE_FLT, "unk"},
	/* 108 */ {WOW_DBC_TYPE_U32, "parent_map"},
	/* 112 */ {WOW_DBC_TYPE_LSTR, "description_alliance"},
	/* 180 */ {WOW_DBC_TYPE_LSTR, "description_horde"},
	/* 248 */ {WOW_DBC_TYPE_U32, "loading_screen"}, /* ? */
	/* 252 */ {WOW_DBC_TYPE_U32, "unk"},
	/* 256 */ {WOW_DBC_TYPE_U32, "unk"},
	/* 260 */ {WOW_DBC_TYPE_FLT, "battlefield_map_icon_scale"},
	/* 264 */ {WOW_DBC_TYPE_LSTR, "requirement"},
	/* 332 */ {WOW_DBC_TYPE_LSTR, "heroic_requirement"},
	/* 400 */ {WOW_DBC_TYPE_LSTR, "unk"},
	/* 468 */ {WOW_DBC_TYPE_U32, "parent_area"},
	/* 472 */ {WOW_DBC_TYPE_FLT, "x_coord"},
	/* 476 */ {WOW_DBC_TYPE_FLT, "y_coord"},
	/* 480 */ {WOW_DBC_TYPE_U32, "reset_time_raid"},
	/* 484 */ {WOW_DBC_TYPE_U32, "reset_time_heroic"},
	/* 488 */ {WOW_DBC_TYPE_U32, "unk"},
	/* 492 */ {WOW_DBC_TYPE_I32, "time_override"},
	/* 496 */ {WOW_DBC_TYPE_U32, "expansion"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_name_gen_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_STR, "name"},
	/*  8 */ {WOW_DBC_TYPE_U32, "race"},
	/* 12 */ {WOW_DBC_TYPE_U32, "gender"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_sound_entries_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "sound_type"},
	/*   8 */ {WOW_DBC_TYPE_STR, "name"},
	/*  12 */ {WOW_DBC_TYPE_STR, "file1"},
	/*  16 */ {WOW_DBC_TYPE_STR, "file2"},
	/*  20 */ {WOW_DBC_TYPE_STR, "file3"},
	/*  24 */ {WOW_DBC_TYPE_STR, "file4"},
	/*  28 */ {WOW_DBC_TYPE_STR, "file5"},
	/*  32 */ {WOW_DBC_TYPE_STR, "file6"},
	/*  36 */ {WOW_DBC_TYPE_STR, "file7"},
	/*  40 */ {WOW_DBC_TYPE_STR, "file8"},
	/*  44 */ {WOW_DBC_TYPE_STR, "file9"},
	/*  48 */ {WOW_DBC_TYPE_STR, "file10"},
	/*  52 */ {WOW_DBC_TYPE_U32, "freq1"},
	/*  56 */ {WOW_DBC_TYPE_U32, "freq2"},
	/*  60 */ {WOW_DBC_TYPE_U32, "freq3"},
	/*  64 */ {WOW_DBC_TYPE_U32, "freq4"},
	/*  68 */ {WOW_DBC_TYPE_U32, "freq5"},
	/*  72 */ {WOW_DBC_TYPE_U32, "freq6"},
	/*  76 */ {WOW_DBC_TYPE_U32, "freq7"},
	/*  80 */ {WOW_DBC_TYPE_U32, "freq8"},
	/*  84 */ {WOW_DBC_TYPE_U32, "freq9"},
	/*  88 */ {WOW_DBC_TYPE_U32, "freq10"},
	/*  92 */ {WOW_DBC_TYPE_STR, "directory"},
	/*  96 */ {WOW_DBC_TYPE_FLT, "volume"},
	/* 100 */ {WOW_DBC_TYPE_U32, "flags"},
	/* 104 */ {WOW_DBC_TYPE_FLT, "min_distance"},
	/* 108 */ {WOW_DBC_TYPE_FLT, "max_distance"},
	/* 112 */ {WOW_DBC_TYPE_FLT, "distance_cutoff"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_spell_def[] =
{
	/*   0 */ {WOW_DBC_TYPE_U32, "id"},
	/*   4 */ {WOW_DBC_TYPE_U32, "category"},
	/*   8 */ {WOW_DBC_TYPE_U32, "cast"},
	/*  12 */ {WOW_DBC_TYPE_U32, "dispel"},
	/*  16 */ {WOW_DBC_TYPE_U32, "mechanic"},
	/*  20 */ {WOW_DBC_TYPE_U32, "attributes"},
	/*  24 */ {WOW_DBC_TYPE_U32, "attributes_ex1"},
	/*  28 */ {WOW_DBC_TYPE_U32, "attributes_ex2"},
	/*  32 */ {WOW_DBC_TYPE_U32, "attributes_ex3"},
	/*  36 */ {WOW_DBC_TYPE_U32, "attributes_ex4"},
	/*  40 */ {WOW_DBC_TYPE_U32, "attributes_ex5"},
	/*  44 */ {WOW_DBC_TYPE_U32, "attributes_ex6"},
	/*  48 */ {WOW_DBC_TYPE_U32, "stances"},
	/*  52 */ {WOW_DBC_TYPE_U32, "stances_exclude"},
	/*  56 */ {WOW_DBC_TYPE_U32, "targets"},
	/*  60 */ {WOW_DBC_TYPE_U32, "targets_creature_type"},
	/*  64 */ {WOW_DBC_TYPE_U32, "required_spell_focus"},
	/*  68 */ {WOW_DBC_TYPE_U32, "facing_caster_flags"},
	/*  72 */ {WOW_DBC_TYPE_U32, "caster_aura_state"},
	/*  76 */ {WOW_DBC_TYPE_U32, "target_aura_state"},
	/*  80 */ {WOW_DBC_TYPE_U32, "caster_aura_state_exclude"},
	/*  84 */ {WOW_DBC_TYPE_U32, "target_aura_state_exclude"},
	/*  88 */ {WOW_DBC_TYPE_U32, "casting_time_index"},
	/*  92 */ {WOW_DBC_TYPE_U32, "recovery_time"},
	/*  96 */ {WOW_DBC_TYPE_U32, "category_recovery_time"},
	/* 100 */ {WOW_DBC_TYPE_U32, "interrupt_flags"},
	/* 104 */ {WOW_DBC_TYPE_U32, "aura_interrupt_flags"},
	/* 108 */ {WOW_DBC_TYPE_U32, "channel_interrupt_flags"},
	/* 112 */ {WOW_DBC_TYPE_U32, "proc_flags"},
	/* 116 */ {WOW_DBC_TYPE_U32, "proc_chance"},
	/* 120 */ {WOW_DBC_TYPE_U32, "proc_charges"},
	/* 124 */ {WOW_DBC_TYPE_U32, "max_level"},
	/* 128 */ {WOW_DBC_TYPE_U32, "base_level"},
	/* 132 */ {WOW_DBC_TYPE_U32, "spell_level"},
	/* 136 */ {WOW_DBC_TYPE_U32, "duration_index"},
	/* 140 */ {WOW_DBC_TYPE_U32, "power_type"},
	/* 144 */ {WOW_DBC_TYPE_U32, "mana_cost"},
	/* 148 */ {WOW_DBC_TYPE_U32, "mana_cost_per_level"},
	/* 152 */ {WOW_DBC_TYPE_U32, "mana_per_second"},
	/* 156 */ {WOW_DBC_TYPE_U32, "mana_per_second_per_level"},
	/* 160 */ {WOW_DBC_TYPE_U32, "range_index"},
	/* 164 */ {WOW_DBC_TYPE_FLT, "speed"},
	/* 168 */ {WOW_DBC_TYPE_U32, "modal_next_spell"},
	/* 172 */ {WOW_DBC_TYPE_U32, "stack_amount"},
	/* 176 */ {WOW_DBC_TYPE_U32, "totem1"},
	/* 180 */ {WOW_DBC_TYPE_U32, "totem2"},
	/* 184 */ {WOW_DBC_TYPE_U32, "reagent1"},
	/* 188 */ {WOW_DBC_TYPE_U32, "reagent2"},
	/* 192 */ {WOW_DBC_TYPE_U32, "reagent3"},
	/* 196 */ {WOW_DBC_TYPE_U32, "reagent4"},
	/* 200 */ {WOW_DBC_TYPE_U32, "reagent5"},
	/* 204 */ {WOW_DBC_TYPE_U32, "reagent6"},
	/* 208 */ {WOW_DBC_TYPE_U32, "reagent7"},
	/* 212 */ {WOW_DBC_TYPE_U32, "reagent8"},
	/* 216 */ {WOW_DBC_TYPE_U32, "reagent_count1"},
	/* 220 */ {WOW_DBC_TYPE_U32, "reagent_count2"},
	/* 224 */ {WOW_DBC_TYPE_U32, "reagent_count3"},
	/* 228 */ {WOW_DBC_TYPE_U32, "reagent_count4"},
	/* 232 */ {WOW_DBC_TYPE_U32, "reagent_count5"},
	/* 236 */ {WOW_DBC_TYPE_U32, "reagent_count6"},
	/* 240 */ {WOW_DBC_TYPE_U32, "reagent_count7"},
	/* 244 */ {WOW_DBC_TYPE_U32, "reagent_count8"},
	/* 248 */ {WOW_DBC_TYPE_U32, "equipped_item_class"},
	/* 252 */ {WOW_DBC_TYPE_U32, "equipped_item_subclass"},
	/* 256 */ {WOW_DBC_TYPE_U32, "equipped_item_inventory"},
	/* 260 */ {WOW_DBC_TYPE_U32, "effect1"},
	/* 264 */ {WOW_DBC_TYPE_U32, "effect2"},
	/* 268 */ {WOW_DBC_TYPE_U32, "effect3"},
	/* 272 */ {WOW_DBC_TYPE_U32, "effect_die_sides1"},
	/* 276 */ {WOW_DBC_TYPE_U32, "effect_die_sides2"},
	/* 280 */ {WOW_DBC_TYPE_U32, "effect_die_sides3"},
	/* 284 */ {WOW_DBC_TYPE_U32, "effect_base_dice1"},
	/* 288 */ {WOW_DBC_TYPE_U32, "effect_base_dice2"},
	/* 292 */ {WOW_DBC_TYPE_U32, "effect_base_dice3"},
	/* 296 */ {WOW_DBC_TYPE_FLT, "effect_dice_per_level1"},
	/* 300 */ {WOW_DBC_TYPE_FLT, "effect_dice_per_level2"},
	/* 304 */ {WOW_DBC_TYPE_FLT, "effect_dice_per_level3"},
	/* 308 */ {WOW_DBC_TYPE_FLT, "effect_real_points_per_level1"},
	/* 312 */ {WOW_DBC_TYPE_FLT, "effect_real_points_per_level2"},
	/* 316 */ {WOW_DBC_TYPE_FLT, "effect_real_points_per_level3"},
	/* 320 */ {WOW_DBC_TYPE_U32, "effect_base_points1"},
	/* 324 */ {WOW_DBC_TYPE_U32, "effect_base_points2"},
	/* 328 */ {WOW_DBC_TYPE_U32, "effect_base_points3"},
	/* 332 */ {WOW_DBC_TYPE_U32, "effect_mechanic1"},
	/* 336 */ {WOW_DBC_TYPE_U32, "effect_mechanic2"},
	/* 340 */ {WOW_DBC_TYPE_U32, "effect_mechanic3"},
	/* 344 */ {WOW_DBC_TYPE_U32, "effect_implicit_targeta1"},
	/* 348 */ {WOW_DBC_TYPE_U32, "effect_implicit_targeta2"},
	/* 352 */ {WOW_DBC_TYPE_U32, "effect_implicit_targeta3"},
	/* 356 */ {WOW_DBC_TYPE_U32, "effect_implicit_targetb1"},
	/* 360 */ {WOW_DBC_TYPE_U32, "effect_implicit_targetb2"},
	/* 364 */ {WOW_DBC_TYPE_U32, "effect_implicit_targetb3"},
	/* 368 */ {WOW_DBC_TYPE_U32, "effect_radius_index1"},
	/* 372 */ {WOW_DBC_TYPE_U32, "effect_radius_index2"},
	/* 376 */ {WOW_DBC_TYPE_U32, "effect_radius_index3"},
	/* 380 */ {WOW_DBC_TYPE_U32, "effect_apply_aura_name1"},
	/* 384 */ {WOW_DBC_TYPE_U32, "effect_apply_aura_name2"},
	/* 388 */ {WOW_DBC_TYPE_U32, "effect_apply_aura_name3"},
	/* 392 */ {WOW_DBC_TYPE_U32, "effect_amplitude1"},
	/* 396 */ {WOW_DBC_TYPE_U32, "effect_amplitude2"},
	/* 400 */ {WOW_DBC_TYPE_U32, "effect_amplitude3"},
	/* 404 */ {WOW_DBC_TYPE_FLT, "effect_multiple_value1"},
	/* 408 */ {WOW_DBC_TYPE_FLT, "effect_multiple_value2"},
	/* 412 */ {WOW_DBC_TYPE_FLT, "effect_multiple_value3"},
	/* 416 */ {WOW_DBC_TYPE_U32, "effect_chain_target1"},
	/* 420 */ {WOW_DBC_TYPE_U32, "effect_chain_target2"},
	/* 424 */ {WOW_DBC_TYPE_U32, "effect_chain_target3"},
	/* 428 */ {WOW_DBC_TYPE_U32, "effect_item_type1"},
	/* 432 */ {WOW_DBC_TYPE_U32, "effect_item_type2"},
	/* 436 */ {WOW_DBC_TYPE_U32, "effect_item_type3"},
	/* 440 */ {WOW_DBC_TYPE_U32, "effect_misc_valuea1"},
	/* 444 */ {WOW_DBC_TYPE_U32, "effect_misc_valuea2"},
	/* 448 */ {WOW_DBC_TYPE_U32, "effect_misc_valuea3"},
	/* 452 */ {WOW_DBC_TYPE_U32, "effect_misc_valueb1"},
	/* 456 */ {WOW_DBC_TYPE_U32, "effect_misc_valueb2"},
	/* 460 */ {WOW_DBC_TYPE_U32, "effect_misc_valueb3"},
	/* 464 */ {WOW_DBC_TYPE_U32, "effect_trigger_spell1"},
	/* 468 */ {WOW_DBC_TYPE_U32, "effect_trigger_spell2"},
	/* 472 */ {WOW_DBC_TYPE_U32, "effect_trigger_spell3"},
	/* 476 */ {WOW_DBC_TYPE_FLT, "effect_points_per_combo_point1"},
	/* 480 */ {WOW_DBC_TYPE_FLT, "effect_points_per_combo_point2"},
	/* 484 */ {WOW_DBC_TYPE_FLT, "effect_points_per_combo_point3"},
	/* 488 */ {WOW_DBC_TYPE_U32, "visual1"},
	/* 492 */ {WOW_DBC_TYPE_U32, "visual2"},
	/* 496 */ {WOW_DBC_TYPE_U32, "icon"},
	/* 500 */ {WOW_DBC_TYPE_U32, "active_icon"},
	/* 504 */ {WOW_DBC_TYPE_U32, "priority"},
	/* 508 */ {WOW_DBC_TYPE_LSTR, "name"},
	/* 576 */ {WOW_DBC_TYPE_LSTR, "subtext"},
	/* 644 */ {WOW_DBC_TYPE_LSTR, "description"},
	/* 712 */ {WOW_DBC_TYPE_LSTR, "tooltip"},
	/* 780 */ {WOW_DBC_TYPE_U32, "mana_cost"},
	/* 784 */ {WOW_DBC_TYPE_U32, "start_cecovery_category"},
	/* 788 */ {WOW_DBC_TYPE_U32, "start_recovery_time"},
	/* 792 */ {WOW_DBC_TYPE_U32, "max_target_level"},
	/* 796 */ {WOW_DBC_TYPE_U32, "spell_family"},
	/* 800 */ {WOW_DBC_TYPE_U32, "spell_family_flags"},
	/* 804 */ {WOW_DBC_TYPE_U32, "max_affected_targets"},
	/* 808 */ {WOW_DBC_TYPE_U32, "damage_class"},
	/* 812 */ {WOW_DBC_TYPE_U32, "prevention_type"},
	/* 816 */ {WOW_DBC_TYPE_U32, "stance_bar_order"},
	/* 820 */ {WOW_DBC_TYPE_U32, "unk"},
	/* 824 */ {WOW_DBC_TYPE_FLT, "damage_multiplier1"},
	/* 828 */ {WOW_DBC_TYPE_FLT, "damage_multiplier2"},
	/* 832 */ {WOW_DBC_TYPE_FLT, "damage_multiplier3"},
	/* 836 */ {WOW_DBC_TYPE_U32, "min_faction"},
	/* 840 */ {WOW_DBC_TYPE_U32, "min_reputation"},
	/* 844 */ {WOW_DBC_TYPE_U32, "required_aura_vision"},
	/* 848 */ {WOW_DBC_TYPE_U32, "totem_category1"},
	/* 852 */ {WOW_DBC_TYPE_U32, "totem_category2"},
	/* 856 */ {WOW_DBC_TYPE_U32, "area"},
	/* 860 */ {WOW_DBC_TYPE_U32, "school"},
	          {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_spell_icon_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "file"},
	        {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_startup_strings_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "name"},
	/* 8 */ {WOW_DBC_TYPE_LSTR, "text"},
	        {WOW_DBC_TYPE_END, ""},
};

const struct wow_dbc_def
wow_dbc_string_lookups_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "filename"},
	        {WOW_DBC_TYPE_END, ""},
};

const struct wow_dbc_def
wow_dbc_talent_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "tab"},
	/*  8 */ {WOW_DBC_TYPE_U32, "tier"},
	/* 12 */ {WOW_DBC_TYPE_U32, "column"},
	/* 16 */ {WOW_DBC_TYPE_U32, "rank1"},
	/* 20 */ {WOW_DBC_TYPE_U32, "rank2"},
	/* 24 */ {WOW_DBC_TYPE_U32, "rank3"},
	/* 28 */ {WOW_DBC_TYPE_U32, "rank4"},
	/* 32 */ {WOW_DBC_TYPE_U32, "rank5"},
	/* 36 */ {WOW_DBC_TYPE_U32, "rank6"},
	/* 40 */ {WOW_DBC_TYPE_U32, "rank7"},
	/* 44 */ {WOW_DBC_TYPE_U32, "rank8"},
	/* 48 */ {WOW_DBC_TYPE_U32, "rank9"},
	/* 52 */ {WOW_DBC_TYPE_U32, "prerequisite_talent1"},
	/* 56 */ {WOW_DBC_TYPE_U32, "prerequisite_talent2"},
	/* 60 */ {WOW_DBC_TYPE_U32, "prerequisite_talent3"},
	/* 64 */ {WOW_DBC_TYPE_U32, "prerequisite_rank1"},
	/* 68 */ {WOW_DBC_TYPE_U32, "prerequisite_rank2"},
	/* 72 */ {WOW_DBC_TYPE_U32, "prerequisite_rank3"},
	/* 76 */ {WOW_DBC_TYPE_U32, "flags"},
	/* 80 */ {WOW_DBC_TYPE_U32, "required_spell"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_talent_tab_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_LSTR, "name"},
	/* 72 */ {WOW_DBC_TYPE_U32, "spell_icon"},
	/* 76 */ {WOW_DBC_TYPE_U32, "race_mask"},
	/* 80 */ {WOW_DBC_TYPE_U32, "class_mask"},
	/* 84 */ {WOW_DBC_TYPE_U32, "order_index"},
	/* 88 */ {WOW_DBC_TYPE_STR, "background_file"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_taxi_nodes_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "continent"},
	/*  8 */ {WOW_DBC_TYPE_FLT, "x"},
	/* 12 */ {WOW_DBC_TYPE_FLT, "y"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "z"},
	/* 20 */ {WOW_DBC_TYPE_LSTR, "name"},
	/* 88 */ {WOW_DBC_TYPE_U32, "mount0"},
	/* 92 */ {WOW_DBC_TYPE_U32, "mount1"},
	         {WOW_DBC_TYPE_END, ""},
};

const struct wow_dbc_def
wow_dbc_taxi_path_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "from_node"},
	/*  8 */ {WOW_DBC_TYPE_U32, "to_node"},
	/* 12 */ {WOW_DBC_TYPE_U32, "cost"},
	         {WOW_DBC_TYPE_END, ""},
};

const struct wow_dbc_def
wow_dbc_taxi_path_node_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "path"},
	/*  8 */ {WOW_DBC_TYPE_U32, "node"},
	/* 12 */ {WOW_DBC_TYPE_U32, "map"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "x"},
	/* 20 */ {WOW_DBC_TYPE_FLT, "y"},
	/* 24 */ {WOW_DBC_TYPE_FLT, "z"},
	/* 28 */ {WOW_DBC_TYPE_U32, "flags"},
	/* 32 */ {WOW_DBC_TYPE_U32, "delay"},
	/* 36 */ {WOW_DBC_TYPE_U32, "arrival_event"},
	/* 40 */ {WOW_DBC_TYPE_U32, "departure_event"},
	         {WOW_DBC_TYPE_END, ""},
};

const struct wow_dbc_def
wow_dbc_world_map_area_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "mapid"},
	/*  8 */ {WOW_DBC_TYPE_U32, "areaid"},
	/* 12 */ {WOW_DBC_TYPE_STR, "area_name"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "loc_left"},
	/* 20 */ {WOW_DBC_TYPE_FLT, "loc_right"},
	/* 24 */ {WOW_DBC_TYPE_FLT, "loc_top"},
	/* 28 */ {WOW_DBC_TYPE_FLT, "loc_bottom"},
	/* 32 */ {WOW_DBC_TYPE_I32, "display_mapid"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_world_map_continent_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "mapid"},
	/*  8 */ {WOW_DBC_TYPE_U32, "left_boundary"},
	/* 12 */ {WOW_DBC_TYPE_U32, "right_boundary"},
	/* 16 */ {WOW_DBC_TYPE_U32, "top_boundary"},
	/* 20 */ {WOW_DBC_TYPE_U32, "bottom_boundary"},
	/* 24 */ {WOW_DBC_TYPE_FLT, "offset_x"},
	/* 28 */ {WOW_DBC_TYPE_FLT, "offset_y"},
	/* 32 */ {WOW_DBC_TYPE_FLT, "scale"},
	/* 36 */ {WOW_DBC_TYPE_FLT, "taxi_min_x"},
	/* 40 */ {WOW_DBC_TYPE_FLT, "taxi_min_y"},
	/* 44 */ {WOW_DBC_TYPE_FLT, "taxi_max_x"},
	/* 48 */ {WOW_DBC_TYPE_FLT, "taxi_max_y"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_world_map_overlay_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "zoneid"},
	/*  8 */ {WOW_DBC_TYPE_U32, "area1"},
	/* 12 */ {WOW_DBC_TYPE_U32, "area2"},
	/* 16 */ {WOW_DBC_TYPE_U32, "area3"},
	/* 20 */ {WOW_DBC_TYPE_U32, "area4"},
	/* 24 */ {WOW_DBC_TYPE_U32, "point_x"},
	/* 28 */ {WOW_DBC_TYPE_U32, "point_y"},
	/* 32 */ {WOW_DBC_TYPE_STR, "texture_name"},
	/* 36 */ {WOW_DBC_TYPE_U32, "texture_width"},
	/* 40 */ {WOW_DBC_TYPE_U32, "texture_height"},
	/* 44 */ {WOW_DBC_TYPE_U32, "texture_x"},
	/* 48 */ {WOW_DBC_TYPE_U32, "texture_y"},
	/* 52 */ {WOW_DBC_TYPE_U32, "hit_rect_top"},
	/* 56 */ {WOW_DBC_TYPE_U32, "hit_rect_left"},
	/* 60 */ {WOW_DBC_TYPE_U32, "hit_rect_bottom"},
	/* 64 */ {WOW_DBC_TYPE_U32, "hit_rect_right"},
	         {WOW_DBC_TYPE_END, ""}
};

const struct wow_dbc_def
wow_dbc_world_map_transforms_def[] =
{
	/*  0 */ {WOW_DBC_TYPE_U32, "id"},
	/*  4 */ {WOW_DBC_TYPE_U32, "mapid"},
	/*  8 */ {WOW_DBC_TYPE_FLT, "min_x"},
	/* 12 */ {WOW_DBC_TYPE_FLT, "min_y"},
	/* 16 */ {WOW_DBC_TYPE_FLT, "max_x"},
	/* 20 */ {WOW_DBC_TYPE_FLT, "max_y"},
	/* 24 */ {WOW_DBC_TYPE_U32, "new_mapid"},
	/* 28 */ {WOW_DBC_TYPE_FLT, "offset_x"},
	/* 32 */ {WOW_DBC_TYPE_FLT, "offset_y"},
	         {WOW_DBC_TYPE_END, ""},
};

const struct wow_dbc_def
wow_dbc_wow_error_strings_def[] =
{
	/* 0 */ {WOW_DBC_TYPE_U32, "id"},
	/* 4 */ {WOW_DBC_TYPE_STR, "name"},
	/* 8 */ {WOW_DBC_TYPE_LSTR, "value"},
	        {WOW_DBC_TYPE_END, ""}
};
