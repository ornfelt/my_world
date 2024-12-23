#include "obj/update_fields.h"
#include "obj/creature.h"

#include "gx/text.h"
#include "gx/blp.h"
#include "gx/m2.h"

#include "memory.h"
#include "cache.h"
#include "log.h"
#include "wow.h"
#include "dbc.h"
#include "wdb.h"

#include <jks/optional.h>

#include <wow/dbc.h>
#include <wow/wdb.h>
#include <wow/mpq.h>

#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define UNIT ((struct unit*)object)
#define WORLD_OBJECT ((struct worldobj*)object)
#define CREATURE ((struct creature*)object)

static bool ctr(struct object *object, uint64_t guid)
{
	if (!unit_object_vtable.ctr(object, guid))
		return false;
	WORLD_OBJECT->worldobj_vtable = &creature_worldobj_vtable;
	UNIT->unit_vtable = &creature_unit_vtable;
	CREATURE->db_get = false;
	CREATURE->clean_batches = false;
	return true;
}

static void dtr(struct object *object)
{
	unit_object_vtable.dtr(object);
}

static void update_render_batches_items(struct object *object)
{
	struct wow_dbc_row display_info_row;
	if (!dbc_get_row_indexed(g_wow->dbc.creature_display_info, &display_info_row, object_fields_get_u32(&object->fields, UNIT_FIELD_DISPLAYID)))
	{
		LOG_ERROR("failed to get create display info row: %" PRIu32, object_fields_get_u32(&object->fields, UNIT_FIELD_DISPLAYID));
		return;
	}
	uint32_t extra_id = wow_dbc_get_u32(&display_info_row, 12);
	bool has_extra;
	/* should be overriden by fields if set ? */
	struct wow_dbc_row display_info_extra_row;
	if (extra_id && dbc_get_row_indexed(g_wow->dbc.creature_display_info_extra, &display_info_extra_row, extra_id))
	{
		has_extra = true;
		for (size_t i = 0; i < 11; ++i)
			UNIT->items[i].id = wow_dbc_get_u32(&display_info_extra_row, 32 + i * 4);
	}
	else
	{
		has_extra = false;
	}
	struct gx_blp *hair_tex;
	unit_get_hair_textures(UNIT, &hair_tex, NULL, NULL);
	gx_m2_instance_set_hair_texture(WORLD_OBJECT->m2, hair_tex);
	gx_blp_free(hair_tex);
	gx_m2_instance_clear_batches(WORLD_OBJECT->m2);
	unit_update_items_batches(object, has_extra ? &CREATURE->hair_style : NULL, has_extra ? &CREATURE->facial_hair : NULL);
	unit_update_items(object);
}

static void update_displayid(struct object *object)
{
	struct wow_dbc_row display_info_row;
	if (!dbc_get_row_indexed(g_wow->dbc.creature_display_info, &display_info_row, object_fields_get_u32(&object->fields, UNIT_FIELD_DISPLAYID)))
	{
		LOG_ERROR("failed to get create display info row: %" PRIu32, object_fields_get_u32(&object->fields, UNIT_FIELD_DISPLAYID));
		return;
	}
	uint32_t model = wow_dbc_get_u32(&display_info_row, 4);
	uint32_t extra_id = wow_dbc_get_u32(&display_info_row, 12);
	UNIT->scale = wow_dbc_get_flt(&display_info_row, 16);
	char monster_textures[3][512];
	snprintf(monster_textures[0], sizeof(monster_textures[0]), "%s", wow_dbc_get_str(&display_info_row, 24));
	snprintf(monster_textures[1], sizeof(monster_textures[1]), "%s", wow_dbc_get_str(&display_info_row, 28));
	snprintf(monster_textures[2], sizeof(monster_textures[2]), "%s", wow_dbc_get_str(&display_info_row, 32));
	struct wow_dbc_row model_data_row;
	if (!dbc_get_row_indexed(g_wow->dbc.creature_model_data, &model_data_row, model))
	{
		LOG_ERROR("failed to get creature model data row: %" PRIu32, model);
		return;
	}
	UNIT->scale *= wow_dbc_get_flt(&model_data_row, 16);
	char filename[512];
	snprintf(filename, sizeof(filename), "%s", wow_dbc_get_str(&model_data_row, 8));
	wow_mpq_normalize_m2_fn(filename, sizeof(filename));
	worldobj_set_m2(WORLD_OBJECT, filename);
	char batch_texture[512] = "";
	struct wow_dbc_row display_info_extra_row;
	struct gx_blp *skin_texture;
	if (dbc_get_row_indexed(g_wow->dbc.creature_display_info_extra, &display_info_extra_row, extra_id))
	{
		uint8_t race = wow_dbc_get_u32(&display_info_extra_row, 4);
		uint8_t sex = wow_dbc_get_i32(&display_info_extra_row, 8);
		CREATURE->skin = wow_dbc_get_i32(&display_info_extra_row, 12);
		CREATURE->face = wow_dbc_get_u32(&display_info_extra_row, 16);
		CREATURE->hair_style = wow_dbc_get_u32(&display_info_extra_row, 20);
		CREATURE->hair_color = wow_dbc_get_u32(&display_info_extra_row, 24);
		CREATURE->facial_hair = wow_dbc_get_u32(&display_info_extra_row, 28);
		object_fields_set_u32(&object->fields, UNIT_FIELD_BYTES_0, race | (sex << 16));
		const char *batch_tex = wow_dbc_get_str(&display_info_extra_row, 80);
		if (batch_tex[0])
		{
			snprintf(batch_texture, sizeof(batch_texture), "textures\\BakedNpcTextures\\%s", batch_tex);
			wow_mpq_normalize_blp_fn(batch_texture, sizeof(batch_texture));
			if (cache_ref_blp(g_wow->cache, batch_texture, &skin_texture))
				gx_blp_ask_load(skin_texture);
			else
				skin_texture = NULL;
		}
		else
		{
			LOG_ERROR("no batch file for display %" PRIu32, extra_id);
			skin_texture = NULL;
		}
	}
	else
	{
		CREATURE->skin = 0;
		CREATURE->face = 0;
		CREATURE->hair_style = 0;
		CREATURE->hair_color = 0;
		CREATURE->facial_hair = 0;
		object_fields_set_u32(&object->fields, UNIT_FIELD_BYTES_0, 0);
		skin_texture = NULL;
	}
	gx_m2_instance_set_skin_texture(WORLD_OBJECT->m2, skin_texture);
	gx_blp_free(skin_texture);
	struct gx_blp *skin_extra_texture;
	unit_get_skin_textures(UNIT, NULL, &skin_extra_texture);
	gx_m2_instance_set_skin_extra_texture(WORLD_OBJECT->m2, skin_extra_texture);
	gx_blp_free(skin_extra_texture);
	for (int i = 0; i < 3; ++i)
	{
		if (!monster_textures[i][0])
			continue;
		char texture_filename[1024];
		char *pos = strrchr(filename, '\\');
		if (!pos)
			continue;
		snprintf(texture_filename, sizeof(texture_filename), "%.*s%s.blp", (int)(pos - filename + 1), filename, monster_textures[i]);
		wow_mpq_normalize_blp_fn(texture_filename, sizeof(texture_filename));
		struct gx_blp *monster_texture;
		if (cache_ref_blp(g_wow->cache, texture_filename, &monster_texture))
			gx_blp_ask_load(monster_texture);
		else
			monster_texture = NULL;
		gx_m2_instance_set_monster_texture(WORLD_OBJECT->m2, i, monster_texture);
		gx_blp_free(monster_texture);
	}
	if (gx_m2_flag_get(WORLD_OBJECT->m2->parent, GX_M2_FLAG_LOADED))
	{
		update_render_batches_items(object);
		CREATURE->clean_batches = true;
	}
	else
	{
		CREATURE->clean_batches = false;
	}
}

static void add_to_render(struct object *object)
{
	if (object_fields_get_bit(&object->fields, OBJECT_FIELD_ENTRY))
	{
		object_fields_disable_bit(&object->fields, OBJECT_FIELD_ENTRY);
		CREATURE->db_get = false;
	}
	if (!CREATURE->db_get)
	{
		const struct wow_wdb_creature *creature = wdb_get_creature(g_wow->wdb, object_fields_get_u32(&object->fields, OBJECT_FIELD_ENTRY));
		/* XXX: as callback */
		if (creature && creature->id)
		{
			char text[1024];
			if (creature->subname && creature->subname[0])
				snprintf(text, sizeof(text), "%s\n<%s>", creature->name[0] ? creature->name[0] : "", creature->subname);
			else
				snprintf(text, sizeof(text), "%s", creature->name[0] ? creature->name[0] : "");
			CREATURE->db_get = true;
			gx_text_set_text(UNIT->text, text);
		}
	}
	if (object_fields_get_bit(&object->fields, UNIT_FIELD_DISPLAYID))
	{
		object_fields_disable_bit(&object->fields, UNIT_FIELD_DISPLAYID);
		update_displayid(object);
	}
	if (!CREATURE->clean_batches && WORLD_OBJECT->m2 && gx_m2_flag_get(WORLD_OBJECT->m2->parent, GX_M2_FLAG_LOADED))
	{
		update_render_batches_items(object);
		CREATURE->clean_batches = true;
	}
	unit_worldobj_vtable.add_to_render(object);
}

static void get_visual_settings(const struct object *object, uint8_t *face, uint8_t *hair_color, uint8_t *hair_style, uint8_t *skin_color, uint8_t *facial_hair)
{
	if (face)
		*face = CREATURE->face;
	if (hair_color)
		*hair_color = CREATURE->hair_color;
	if (hair_style)
		*hair_style = CREATURE->hair_style;
	if (skin_color)
		*skin_color = CREATURE->skin;
	if (facial_hair)
		*facial_hair = CREATURE->facial_hair;
}

static void on_field_changed(struct object *object, uint32_t field)
{
	return unit_object_vtable.on_field_changed(object, field);
}

const struct object_vtable creature_object_vtable =
{
	.ctr = ctr,
	.dtr = dtr,
	.on_field_changed = on_field_changed,
};

const struct unit_vtable creature_unit_vtable =
{
	.get_visual_settings = get_visual_settings,
};

const struct worldobj_vtable creature_worldobj_vtable =
{
	.add_to_render = add_to_render,
};

struct creature *creature_new(uint64_t guid)
{
	struct object *object = mem_malloc(MEM_OBJ, sizeof(struct creature));
	if (!object)
		return NULL;
	object->vtable = &creature_object_vtable;
	if (!object->vtable->ctr(object, guid))
	{
		object->vtable->dtr(object);
		mem_free(MEM_OBJ, object);
		return NULL;
	}
	return (struct creature*)object;
}
