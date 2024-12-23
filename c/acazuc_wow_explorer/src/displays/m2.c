#include "table_macro.h"
#include "display.h"

#include <wow/m2.h>

#include <inttypes.h>
#include <stdbool.h>

enum m2_category
{
	M2_CATEGORY_HEADER = 1,
	M2_CATEGORY_TEXTURE_COMBINERS_COMBOS,
	M2_CATEGORY_TEXTURE_UNIT_LOOKUPS,
	M2_CATEGORY_TEXTURE_TRANSFORMS_LOOKUPS,
	M2_CATEGORY_TEXTURE_TRANSFORMS,
	M2_CATEGORY_TEXTURE_WEIGHTS_LOOKUPS,
	M2_CATEGORY_TEXTURE_WEIGHTS,
	M2_CATEGORY_TEXTURE_LOOKUPS,
	M2_CATEGORY_SKIN_PROFILES,
	M2_CATEGORY_MATERIALS,
	M2_CATEGORY_SEQUENCES,
	M2_CATEGORY_PARTICLES,
	M2_CATEGORY_TEXTURES,
	M2_CATEGORY_CAMERAS,
	M2_CATEGORY_CAMERA_LOOKUPS,
	M2_CATEGORY_RIBBONS,
	M2_CATEGORY_LIGHTS,
	M2_CATEGORY_BONES,
};

struct m2_display
{
	struct display display;
	struct wow_m2_file *file;
};

static GtkWidget *build_tree(struct m2_display *display, struct wow_m2_file *file);

static void dtr(struct display *ptr)
{
	struct m2_display *display = (struct m2_display*)ptr;
	wow_m2_file_delete(display->file);
}

static char *generate_track_values_uint8(char *d, size_t n, const struct wow_m2_track *track)
{
	char *s = d;
	d[0] = '\0';
	for (size_t i = 0; i < track->values_nb; ++i)
	{
		size_t ret = snprintf(d, n, "{%" PRIu8 ", %" PRIu32 "}%s", ((uint8_t*)track->values)[i], track->timestamps[i], (i + 1) < track->values_nb ? ", " : "");
		if (ret >= n)
			break;
		d += ret;
		n -= ret;
	}
	return s;
}

static char *generate_track_values_int16(char *d, size_t n, const struct wow_m2_track *track)
{
	char *s = d;
	d[0] = '\0';
	for (size_t i = 0; i < track->values_nb; ++i)
	{
		size_t ret = snprintf(d, n, "{%" PRId16 ", %" PRIu32 "}%s", ((int16_t*)track->values)[i], track->timestamps[i], (i + 1) < track->values_nb ? ", " : "");
		if (ret >= n)
			break;
		d += ret;
		n -= ret;
	}
	return s;
}

static char *generate_track_values_flt(char *d, size_t n, const struct wow_m2_track *track)
{
	char *s = d;
	d[0] = '\0';
	for (size_t i = 0; i < track->values_nb; ++i)
	{
		size_t ret = snprintf(d, n, "{%f, %" PRIu32 "}%s", ((float*)track->values)[i], track->timestamps[i], (i + 1) < track->values_nb ? ", " : "");
		if (ret >= n)
			break;
		d += ret;
		n -= ret;
	}
	return s;
}

static char *generate_track_values_vec3f(char *d, size_t n, const struct wow_m2_track *track)
{
	char *s = d;
	d[0] = '\0';
	for (size_t i = 0; i < track->values_nb; ++i)
	{
		struct wow_vec3f *v = &((struct wow_vec3f*)track->values)[i];
		size_t ret = snprintf(d, n, "{{%f, %f, %f}, %" PRIu32 "}%s", v->x, v->y, v->z, track->timestamps[i], (i + 1) < track->values_nb ? ", " : "");
		if (ret >= n)
			break;
		d += ret;
		n -= ret;
	}
	return s;
}

static char *generate_track_values_quatf(char *d, size_t n, const struct wow_m2_track *track)
{
	char *s = d;
	d[0] = '\0';
	for (size_t i = 0; i < track->values_nb; ++i)
	{
		struct wow_quatf *v = &((struct wow_quatf*)track->values)[i];
		size_t ret = snprintf(d, n, "{{%f, %f, %f, %f}, %" PRIu32 "}%s", v->x, v->y, v->z, v->w, track->timestamps[i], (i + 1) < track->values_nb ? ", " : "");
		if (ret >= n)
			break;
		d += ret;
		n -= ret;
	}
	return s;
}

static char *generate_track_str_flt(char *d, size_t n, const struct wow_m2_track *track)
{
	char values[4096];
	snprintf(d, n, "{interpolation_type: %" PRIu16 ", global_sequence: %" PRId16 ", interpolation_ranges_nb: %" PRIu32 ", values: %s}", track->interpolation_type, track->global_sequence, track->interpolation_ranges_nb, generate_track_values_flt(values, sizeof(values), track));
	return d;
}

static char *generate_track_str_vec3f(char *d, size_t n, const struct wow_m2_track *track)
{
	char values[4096];
	snprintf(d, n, "{interpolation_type: %" PRIu16 ", global_sequence: %" PRId16 ", interpolation_ranges_nb: %" PRIu32 ", values: %s}", track->interpolation_type, track->global_sequence, track->interpolation_ranges_nb, generate_track_values_vec3f(values, sizeof(values), track));
	return d;
}

static char *generate_track_str_uint8(char *d, size_t n, const struct wow_m2_track *track)
{
	char values[4096];
	snprintf(d, n, "{interpolation_type: %" PRIu16 ", global_sequence: %" PRId16 ", interpolation_ranges_nb: %" PRIu32 ", values: %s}", track->interpolation_type, track->global_sequence, track->interpolation_ranges_nb, generate_track_values_uint8(values, sizeof(values), track));
	return d;
}

struct display *m2_display_new(const struct node *node, const char *path, struct wow_mpq_file *mpq_file)
{
	(void)node;
	(void)path;
	struct wow_m2_file *file = wow_m2_file_new(mpq_file);
	if (!file)
	{
		fprintf(stderr, "failed to parse m2 file\n");
		return NULL;
	}
	struct m2_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "m2 display allocation failed\n");
		wow_m2_file_delete(file);
		return NULL;
	}
	display->file = file;
	display->display.name = strdup(path);
	display->display.dtr = dtr;
	GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	GtkWidget *left_paned = gtk_scrolled_window_new();
	GtkWidget *right_paned = gtk_scrolled_window_new();
	GtkWidget *tree = build_tree(display, file);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(left_paned), tree);
	gtk_widget_show(left_paned);
	gtk_widget_show(right_paned);
	gtk_paned_set_start_child(GTK_PANED(paned), left_paned);
	gtk_paned_set_end_child(GTK_PANED(paned), right_paned);
	gtk_paned_set_position(GTK_PANED(paned), 200);
	gtk_widget_show(paned);
	display->display.root = paned;
	return &display->display;
}

static GtkWidget *build_text_width(char *str)
{
	GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, str, strlen(str));
	GtkWidget *text = gtk_text_view_new_with_buffer(buffer);
	gtk_text_view_set_monospace(GTK_TEXT_VIEW(text), true);
	gtk_widget_show(text);
	return text;
}

static GtkWidget *build_main_header(struct m2_display *display)
{
	char data[4096];
	snprintf(data, sizeof(data),
	         "magic: %08" PRIx32 "\n"
	         "version: %" PRIu32 "\n"
	         "name: %s\n"
	         "flags: 0x%08" PRIx32 "\n",
	         display->file->header.magic,
	         display->file->header.version,
	         display->file->name,
	         display->file->header.flags);
	return build_text_width(data);
}

static GtkWidget *build_profile_sections(struct wow_m2_skin_profile *skin_profile)
{
	TREE_BEGIN(12, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_FLOAT);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "skin_section_id");
	ADD_TREE_COLUMN(2, "level");
	ADD_TREE_COLUMN(3, "vertex_start");
	ADD_TREE_COLUMN(4, "vertex_count");
	ADD_TREE_COLUMN(5, "index_start");
	ADD_TREE_COLUMN(6, "index_count");
	ADD_TREE_COLUMN(7, "bone_count");
	ADD_TREE_COLUMN(8, "bone_combo_index");
	ADD_TREE_COLUMN(9, "bone_influences");
	ADD_TREE_COLUMN(10, "center_bone_index");
	ADD_TREE_COLUMN(11, "sort_radius");
	for (uint32_t i = 0; i < skin_profile->sections_nb; ++i)
	{
		const struct wow_m2_skin_section *section = &skin_profile->sections[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, section->skin_section_id);
		SET_TREE_VALUE_U64(2, section->level);
		SET_TREE_VALUE_U64(3, section->vertex_start);
		SET_TREE_VALUE_U64(4, section->vertex_count);
		SET_TREE_VALUE_U64(5, section->index_start);
		SET_TREE_VALUE_U64(6, section->index_count);
		SET_TREE_VALUE_U64(7, section->bone_count);
		SET_TREE_VALUE_U64(8, section->bone_combo_index);
		SET_TREE_VALUE_U64(9, section->bone_influences);
		SET_TREE_VALUE_U64(10, section->center_bone_index);
		SET_TREE_VALUE_FLT(11, section->sort_radius);
	}
	TREE_END();
}

static GtkWidget *build_profile_batchs(struct wow_m2_skin_profile *skin_profile)
{
	TREE_BEGIN(14, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_INT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "flags");
	ADD_TREE_COLUMN(2, "priority_plane");
	ADD_TREE_COLUMN(3, "shader_id");
	ADD_TREE_COLUMN(4, "skin_section_index");
	ADD_TREE_COLUMN(5, "geoset_index");
	ADD_TREE_COLUMN(6, "color_index");
	ADD_TREE_COLUMN(7, "material_index");
	ADD_TREE_COLUMN(8, "material_layer");
	ADD_TREE_COLUMN(9, "texture_count");
	ADD_TREE_COLUMN(10, "texture_combo_index");
	ADD_TREE_COLUMN(11, "texture_coord_combo_index");
	ADD_TREE_COLUMN(12, "texture_weight_combo_index");
	ADD_TREE_COLUMN(13, "texture_transform_combo_index");
	for (uint32_t i = 0; i < skin_profile->batches_nb; ++i)
	{
		const struct wow_m2_batch *batch = &skin_profile->batches[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, batch->flags);
		SET_TREE_VALUE_U64(2, batch->priority_plane);
		SET_TREE_VALUE_I64(3, batch->shader_id);
		SET_TREE_VALUE_U64(4, batch->skin_section_index);
		SET_TREE_VALUE_U64(5, batch->geoset_index);
		SET_TREE_VALUE_U64(6, batch->color_index);
		SET_TREE_VALUE_U64(7, batch->material_index);
		SET_TREE_VALUE_U64(8, batch->material_layer);
		SET_TREE_VALUE_U64(9, batch->texture_count);
		SET_TREE_VALUE_U64(10, batch->texture_combo_index);
		SET_TREE_VALUE_U64(11, batch->texture_coord_combo_index);
		SET_TREE_VALUE_U64(12, batch->texture_weight_combo_index);
		SET_TREE_VALUE_U64(13, batch->texture_transform_combo_index);
	}
	TREE_END();
}

static GtkWidget *build_profile_vertexes(struct wow_m2_skin_profile *skin_profile)
{
	TREE_BEGIN(2, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "vertex");
	for (uint32_t i = 0; i < skin_profile->vertexes_nb; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, skin_profile->vertexes[i]);
	}
	TREE_END();
}

static GtkWidget *build_profile_indices(struct wow_m2_skin_profile *skin_profile)
{
	TREE_BEGIN(2, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "indice");
	for (uint32_t i = 0; i < skin_profile->indices_nb; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, skin_profile->indices[i]);
	}
	TREE_END();
}

static GtkWidget *build_profile_bones(struct wow_m2_skin_profile *skin_profile)
{
	TREE_BEGIN(5, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "bone1");
	ADD_TREE_COLUMN(2, "bone2");
	ADD_TREE_COLUMN(3, "bone3");
	ADD_TREE_COLUMN(4, "bone4");
	for (uint32_t i = 0; i < skin_profile->vertexes_nb; ++i)
	{
		struct wow_m2_skin_profile_bone *bone = &skin_profile->bones[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, bone->values[0]);
		SET_TREE_VALUE_U64(2, bone->values[1]);
		SET_TREE_VALUE_U64(3, bone->values[2]);
		SET_TREE_VALUE_U64(4, bone->values[3]);
	}
	TREE_END();
}

static GtkWidget *build_skin_profile(struct m2_display *display, uint32_t val)
{
	uint8_t profile_id = val & 0xff;
	struct wow_m2_skin_profile *skin_profile = &display->file->skin_profiles[profile_id - 1];
	switch ((val >> 8) & 0xf)
	{
		case 1:
			return build_profile_sections(skin_profile);
		case 2:
			return build_profile_batchs(skin_profile);
		case 3:
			return build_profile_vertexes(skin_profile);
		case 4:
			return build_profile_indices(skin_profile);
		case 5:
			return build_profile_bones(skin_profile);
	}
	return NULL;
}

static GtkWidget *build_materials(struct m2_display *display)
{
	TREE_BEGIN(3, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "flags");
	ADD_TREE_COLUMN(2, "blend_mode");
	for (uint32_t i = 0; i < display->file->materials_nb; ++i)
	{
		const struct wow_m2_material *material = &display->file->materials[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, material->flags);
		SET_TREE_VALUE_U64(2, material->blend_mode);
	}
	TREE_END();
}

static GtkWidget *build_textures(struct m2_display *display)
{
	TREE_BEGIN(4, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "type");
	ADD_TREE_COLUMN(2, "flags");
	ADD_TREE_COLUMN(3, "filename");
	for (uint32_t i = 0; i < display->file->textures_nb; ++i)
	{
		const struct wow_m2_texture *texture = &display->file->textures[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, texture->type);
		SET_TREE_VALUE_U64(2, texture->flags);
		SET_TREE_VALUE_STR(3, texture->filename);
	}
	TREE_END();
}

static GtkWidget *build_cameras(struct m2_display *display)
{
	TREE_BEGIN(7, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "type");
	ADD_TREE_COLUMN(2, "fov");
	ADD_TREE_COLUMN(3, "far_clip");
	ADD_TREE_COLUMN(4, "near_clip");
	ADD_TREE_COLUMN(5, "position_base");
	ADD_TREE_COLUMN(6, "target_position_base");
	for (uint32_t i = 0; i < display->file->cameras_nb; ++i)
	{
		const struct wow_m2_camera *camera = &display->file->cameras[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, camera->type);
		SET_TREE_VALUE_FLT(2, camera->fov);
		SET_TREE_VALUE_FLT(3, camera->far_clip);
		SET_TREE_VALUE_FLT(4, camera->near_clip);
		SET_TREE_VALUE_FMT(5, "{%f, %f, %f}", camera->position_base.x, camera->position_base.y, camera->position_base.z);
		SET_TREE_VALUE_FMT(6, "{%f, %f, %f}", camera->target_position_base.x, camera->target_position_base.y, camera->target_position_base.z);
	}
	TREE_END();
}

static GtkWidget *build_camera_lookups(struct m2_display *display)
{
	TREE_BEGIN(2, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "index");
	for (uint32_t i = 0; i < display->file->camera_lookups_nb; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, display->file->camera_lookups[i]);
	}
	TREE_END();
}

static GtkWidget *build_lights(struct m2_display *display)
{
	TREE_BEGIN(11, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_INT64, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "type");
	ADD_TREE_COLUMN(2, "bone");
	ADD_TREE_COLUMN(3, "position");
	ADD_TREE_COLUMN(4, "ambient_color");
	ADD_TREE_COLUMN(5, "ambient_intensity");
	ADD_TREE_COLUMN(6, "diffuse_color");
	ADD_TREE_COLUMN(7, "diffuse_intensity");
	ADD_TREE_COLUMN(8, "attenuation_start");
	ADD_TREE_COLUMN(9, "attenuation_end");
	ADD_TREE_COLUMN(10, "visibility");
	for (uint32_t i = 0; i < display->file->lights_nb; ++i)
	{
		char buf[4096];
		const struct wow_m2_light *light = &display->file->lights[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, light->type);
		SET_TREE_VALUE_I64(2, light->bone);
		SET_TREE_VALUE_FMT(3, "{%f, %f, %f}", light->position.x, light->position.y, light->position.z);
		SET_TREE_VALUE_STR(4, generate_track_str_vec3f(buf, sizeof(buf), &light->ambient_color));
		SET_TREE_VALUE_STR(5, generate_track_str_flt(buf, sizeof(buf), &light->ambient_intensity));
		SET_TREE_VALUE_STR(6, generate_track_str_vec3f(buf, sizeof(buf), &light->diffuse_color));
		SET_TREE_VALUE_STR(7, generate_track_str_flt(buf, sizeof(buf), &light->diffuse_intensity));
		SET_TREE_VALUE_STR(8, generate_track_str_flt(buf, sizeof(buf), &light->attenuation_start));
		SET_TREE_VALUE_STR(9, generate_track_str_flt(buf, sizeof(buf), &light->attenuation_end));
		SET_TREE_VALUE_STR(10, generate_track_str_uint8(buf, sizeof(buf), &light->visibility));
	}
	TREE_END();
}

static GtkWidget *build_sequences(struct m2_display *display)
{
	TREE_BEGIN(11, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_FLOAT, G_TYPE_UINT64, G_TYPE_INT64, G_TYPE_UINT64, G_TYPE_INT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "id");
	ADD_TREE_COLUMN(2, "variation_index");
	ADD_TREE_COLUMN(3, "start");
	ADD_TREE_COLUMN(4, "end");
	ADD_TREE_COLUMN(5, "movespeed");
	ADD_TREE_COLUMN(6, "flags");
	ADD_TREE_COLUMN(7, "frequency");
	ADD_TREE_COLUMN(8, "blend_time");
	ADD_TREE_COLUMN(9, "variation_next");
	ADD_TREE_COLUMN(10, "alias_next");
	for (uint32_t i = 0; i < display->file->sequences_nb; ++i)
	{
		const struct wow_m2_sequence *sequence = &display->file->sequences[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, sequence->id);
		SET_TREE_VALUE_U64(2, sequence->variation_index);
		SET_TREE_VALUE_U64(3, sequence->start);
		SET_TREE_VALUE_U64(4, sequence->end);
		SET_TREE_VALUE_FLT(5, sequence->movespeed);
		SET_TREE_VALUE_U64(6, sequence->flags);
		SET_TREE_VALUE_I64(7, sequence->frequency);
		SET_TREE_VALUE_U64(8, sequence->blend_time);
		SET_TREE_VALUE_I64(9, sequence->variation_next);
		SET_TREE_VALUE_U64(10, sequence->alias_next);
	}
	TREE_END();
}

static GtkWidget *build_bones(struct m2_display *display)
{
	TREE_BEGIN(7, G_TYPE_UINT64, G_TYPE_INT64, G_TYPE_UINT64, G_TYPE_INT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "key_bone_id");
	ADD_TREE_COLUMN(2, "flags");
	ADD_TREE_COLUMN(3, "parent_bone");
	ADD_TREE_COLUMN(4, "submesh_id");
	ADD_TREE_COLUMN(5, "bone_name_crc");
	ADD_TREE_COLUMN(6, "pivot");
	for (uint32_t i = 0; i < display->file->bones_nb; ++i)
	{
		const struct wow_m2_bone *bone = &display->file->bones[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_I64(1, bone->key_bone_id);
		SET_TREE_VALUE_U64(2, bone->flags);
		SET_TREE_VALUE_I64(3, bone->parent_bone);
		SET_TREE_VALUE_U64(4, bone->submesh_id);
		SET_TREE_VALUE_U64(5, bone->bone_name_crc);
		SET_TREE_VALUE_FMT(6, "{%f, %f, %f}", bone->pivot.x, bone->pivot.y, bone->pivot.z);
	}
	TREE_END();
}

static GtkWidget *build_particles(struct m2_display *display)
{
	TREE_BEGIN(45, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_STRING, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "flags");
	ADD_TREE_COLUMN(2, "position");
	ADD_TREE_COLUMN(3, "bone");
	ADD_TREE_COLUMN(4, "texture");
	ADD_TREE_COLUMN(5, "geometry_model");
	ADD_TREE_COLUMN(6, "recursion_model");
	ADD_TREE_COLUMN(7, "blending_type");
	ADD_TREE_COLUMN(8, "emitter_type");
	ADD_TREE_COLUMN(9, "particle_type");
	ADD_TREE_COLUMN(10, "hear_or_tail");
	ADD_TREE_COLUMN(11, "texture_tile_rotation");
	ADD_TREE_COLUMN(12, "texture_dimensions_rows");
	ADD_TREE_COLUMN(13, "texture_dimensions_columns");
	ADD_TREE_COLUMN(14, "mid_point");
	ADD_TREE_COLUMN(15, "color_values");
	ADD_TREE_COLUMN(16, "scale_values");
	ADD_TREE_COLUMN(17, "lifespan_uv_anim");
	ADD_TREE_COLUMN(18, "decay_uv_anim");
	ADD_TREE_COLUMN(19, "tail_uv_anim");
	ADD_TREE_COLUMN(20, "tail_decay_uv_anim");
	ADD_TREE_COLUMN(21, "tail_length");
	ADD_TREE_COLUMN(22, "twinkle_speed");
	ADD_TREE_COLUMN(23, "twinkle_percent");
	ADD_TREE_COLUMN(24, "twinkle_scale_min");
	ADD_TREE_COLUMN(25, "twinkle_scale_max");
	ADD_TREE_COLUMN(26, "burst_multiplier");
	ADD_TREE_COLUMN(27, "drag");
	ADD_TREE_COLUMN(28, "spin");
	ADD_TREE_COLUMN(29, "wind_vector");
	ADD_TREE_COLUMN(30, "wind_time");
	ADD_TREE_COLUMN(31, "follow_speed1");
	ADD_TREE_COLUMN(32, "follow_scale1");
	ADD_TREE_COLUMN(33, "follow_speed2");
	ADD_TREE_COLUMN(34, "follow_scale2");
	ADD_TREE_COLUMN(35, "emission_speed");
	ADD_TREE_COLUMN(36, "speed_variation");
	ADD_TREE_COLUMN(37, "vertical_range");
	ADD_TREE_COLUMN(38, "horizontal_range");
	ADD_TREE_COLUMN(39, "gravity");
	ADD_TREE_COLUMN(40, "lifespan");
	ADD_TREE_COLUMN(41, "emission_rate");
	ADD_TREE_COLUMN(42, "emission_area_length");
	ADD_TREE_COLUMN(43, "emission_area_width");
	ADD_TREE_COLUMN(44, "z_source");
	for (uint32_t i = 0; i < display->file->particles_nb; ++i)
	{
		char buf[4096];
		const struct wow_m2_particle *particle = &display->file->particles[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, particle->id);
		SET_TREE_VALUE_U64(1, particle->flags);
		SET_TREE_VALUE_FMT(2, "{%f, %f, %f}", particle->position.x, particle->position.y, particle->position.z);
		SET_TREE_VALUE_U64(3, particle->bone);
		SET_TREE_VALUE_U64(4, particle->texture);
		SET_TREE_VALUE_STR(5, particle->geometry_model_filename);
		SET_TREE_VALUE_STR(6, particle->recursion_model_filename);
		SET_TREE_VALUE_U64(7, particle->blending_type);
		SET_TREE_VALUE_U64(8, particle->emitter_type);
		SET_TREE_VALUE_U64(9, particle->particle_type);
		SET_TREE_VALUE_U64(10, particle->head_or_tail);
		SET_TREE_VALUE_U64(11, particle->texture_tile_rotation);
		SET_TREE_VALUE_U64(12, particle->texture_dimensions_rows);
		SET_TREE_VALUE_U64(13, particle->texture_dimensions_columns);
		SET_TREE_VALUE_FLT(14, particle->mid_point);
		SET_TREE_VALUE_FMT(15, "{{0x%" PRIx8 ", 0x%" PRIx8 ", 0x%" PRIx8 ", 0x%" PRIx8 "}, {0x%" PRIx8 ", 0x%" PRIx8 ", 0x%" PRIx8 ", 0x%" PRIx8 "}, {0x%" PRIx8 ", 0x%" PRIx8 ", 0x%" PRIx8 ", 0x%" PRIx8 "}}", particle->color_values[0].x, particle->color_values[0].y, particle->color_values[0].z, particle->color_values[0].w, particle->color_values[1].x, particle->color_values[1].y, particle->color_values[1].z, particle->color_values[1].w, particle->color_values[2].x, particle->color_values[2].y, particle->color_values[2].z, particle->color_values[2].w);
		SET_TREE_VALUE_FMT(16, "{%f, %f, %f}", particle->scale_values[0], particle->scale_values[1], particle->scale_values[2]);
		SET_TREE_VALUE_FMT(17, "{%" PRIu16 ", %" PRIu16 ", %" PRIu16 "}", particle->lifespan_uv_anim[0], particle->lifespan_uv_anim[1], particle->lifespan_uv_anim[2]);
		SET_TREE_VALUE_FMT(18, "{%" PRIu16 ", %" PRIu16 ", %" PRIu16 "}", particle->decay_uv_anim[0], particle->decay_uv_anim[1], particle->decay_uv_anim[2]);
		SET_TREE_VALUE_FMT(19, "{%" PRId16 ", %" PRId16 "}", particle->tail_uv_anim[0], particle->tail_uv_anim[1]);
		SET_TREE_VALUE_FMT(20, "{%" PRId16 ", %" PRId16 "}", particle->tail_decay_uv_anim[0], particle->tail_decay_uv_anim[1]);
		SET_TREE_VALUE_FLT(21, particle->tail_length);
		SET_TREE_VALUE_FLT(22, particle->twinkle_speed);
		SET_TREE_VALUE_FLT(23, particle->twinkle_percent);
		SET_TREE_VALUE_FLT(24, particle->twinkle_scale_min);
		SET_TREE_VALUE_FLT(25, particle->twinkle_scale_max);
		SET_TREE_VALUE_FLT(26, particle->burst_multiplier);
		SET_TREE_VALUE_FLT(27, particle->drag);
		SET_TREE_VALUE_FLT(28, particle->spin);
		SET_TREE_VALUE_FMT(29, "{%f, %f, %f}", particle->wind_vector.x, particle->wind_vector.y, particle->wind_vector.z);
		SET_TREE_VALUE_FLT(30, particle->wind_time);
		SET_TREE_VALUE_FLT(31, particle->follow_speed1);
		SET_TREE_VALUE_FLT(32, particle->follow_scale1);
		SET_TREE_VALUE_FLT(33, particle->follow_speed2);
		SET_TREE_VALUE_FLT(34, particle->follow_scale2);
		SET_TREE_VALUE_STR(35, generate_track_values_flt(buf, sizeof(buf), &particle->emission_speed));
		SET_TREE_VALUE_STR(36, generate_track_values_flt(buf, sizeof(buf), &particle->speed_variation));
		SET_TREE_VALUE_STR(37, generate_track_values_flt(buf, sizeof(buf), &particle->vertical_range));
		SET_TREE_VALUE_STR(38, generate_track_values_flt(buf, sizeof(buf), &particle->horizontal_range));
		SET_TREE_VALUE_STR(39, generate_track_values_flt(buf, sizeof(buf), &particle->gravity));
		SET_TREE_VALUE_STR(40, generate_track_values_flt(buf, sizeof(buf), &particle->lifespan));
		SET_TREE_VALUE_STR(41, generate_track_values_flt(buf, sizeof(buf), &particle->emission_rate));
		SET_TREE_VALUE_STR(42, generate_track_values_flt(buf, sizeof(buf), &particle->emission_area_length));
		SET_TREE_VALUE_STR(43, generate_track_values_flt(buf, sizeof(buf), &particle->emission_area_width));
		SET_TREE_VALUE_STR(44, generate_track_values_flt(buf, sizeof(buf), &particle->z_source));
	}
	TREE_END();
}

static GtkWidget *build_texture_combiners_combos(struct m2_display *display)
{
	TREE_BEGIN(2, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "value");
	for (uint32_t i = 0; i < display->file->texture_combiner_combos_nb; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, display->file->texture_combiner_combos[i]);
	}
	TREE_END();
}

static GtkWidget *build_texture_unit_lookups(struct m2_display *display)
{
	TREE_BEGIN(2, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "value");
	for (uint32_t i = 0; i < display->file->texture_unit_lookups_nb; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, display->file->texture_unit_lookups[i]);
	}
	TREE_END();
}

static GtkWidget *build_texture_lookups(struct m2_display *display)
{
	TREE_BEGIN(2, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "value");
	for (uint32_t i = 0; i < display->file->texture_lookups_nb; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, display->file->texture_lookups[i]);
	}
	TREE_END();
}

static GtkWidget *build_texture_weights_lookups(struct m2_display *display)
{
	TREE_BEGIN(2, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "value");
	for (uint32_t i = 0; i < display->file->texture_weights_lookups_nb; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, display->file->texture_weights_lookups[i]);
	}
	TREE_END();
}

static GtkWidget *build_texture_weights(struct m2_display *display)
{
	TREE_BEGIN(6, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_INT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "interpolation_type");
	ADD_TREE_COLUMN(2, "global_sequence");
	ADD_TREE_COLUMN(3, "values_nb");
	ADD_TREE_COLUMN(4, "timestamps_nb");
	ADD_TREE_COLUMN(5, "values");
	for (uint32_t i = 0; i < display->file->texture_weights_nb; ++i)
	{
		const struct wow_m2_track *weight = &display->file->texture_weights[i].weight;
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		char data[4096];
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, weight->interpolation_type);
		SET_TREE_VALUE_I64(2, weight->global_sequence);
		SET_TREE_VALUE_U64(3, weight->values_nb);
		SET_TREE_VALUE_U64(4, weight->timestamps_nb);
		SET_TREE_VALUE_STR(5, generate_track_values_int16(data, sizeof(data), weight));
	}
	TREE_END();
}

static GtkWidget *build_texture_transforms_lookups(struct m2_display *display)
{
	TREE_BEGIN(2, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "value");
	for (uint32_t i = 0; i < display->file->texture_transforms_lookups_nb; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, display->file->texture_transforms_lookups[i]);
	}
	TREE_END();
}

static GtkWidget *build_texture_transforms(struct m2_display *display)
{
	TREE_BEGIN(7, G_TYPE_UINT64, G_TYPE_STRING, G_TYPE_UINT64, G_TYPE_INT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "type");
	ADD_TREE_COLUMN(2, "interpolation_type");
	ADD_TREE_COLUMN(3, "global_sequence");
	ADD_TREE_COLUMN(4, "values_nb");
	ADD_TREE_COLUMN(5, "timestamps_nb");
	ADD_TREE_COLUMN(6, "values");
	for (uint32_t i = 0; i < display->file->texture_transforms_nb; ++i)
	{
		const struct wow_m2_texture_transform *transform = &display->file->texture_transforms[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		char data[4096];
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_STR(1, "translation");
		SET_TREE_VALUE_U64(2, transform->translation.interpolation_type);
		SET_TREE_VALUE_I64(3, transform->translation.global_sequence);
		SET_TREE_VALUE_U64(4, transform->translation.values_nb);
		SET_TREE_VALUE_U64(5, transform->translation.timestamps_nb);
		SET_TREE_VALUE_STR(6, generate_track_values_vec3f(data, sizeof(data), &transform->translation));
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_STR(1, "rotation");
		SET_TREE_VALUE_U64(2, transform->rotation.interpolation_type);
		SET_TREE_VALUE_I64(3, transform->rotation.global_sequence);
		SET_TREE_VALUE_U64(4, transform->rotation.values_nb);
		SET_TREE_VALUE_U64(5, transform->rotation.timestamps_nb);
		SET_TREE_VALUE_STR(6, generate_track_values_quatf(data, sizeof(data), &transform->rotation));
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_STR(1, "scaling");
		SET_TREE_VALUE_U64(2, transform->scaling.interpolation_type);
		SET_TREE_VALUE_I64(3, transform->scaling.global_sequence);
		SET_TREE_VALUE_U64(4, transform->scaling.values_nb);
		SET_TREE_VALUE_U64(5, transform->scaling.timestamps_nb);
		SET_TREE_VALUE_STR(6, generate_track_values_vec3f(data, sizeof(data), &transform->scaling));
	}
	TREE_END();
}

static GtkWidget *build_ribbons(struct m2_display *display)
{
	TREE_BEGIN(9, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_UINT64, G_TYPE_UINT64);
	ADD_TREE_COLUMN(0, "id");
	ADD_TREE_COLUMN(1, "ribbon");
	ADD_TREE_COLUMN(2, "bone");
	ADD_TREE_COLUMN(3, "position");
	ADD_TREE_COLUMN(4, "edges_per_second");
	ADD_TREE_COLUMN(5, "edge_lifetime");
	ADD_TREE_COLUMN(6, "gravity");
	ADD_TREE_COLUMN(7, "texture_rows");
	ADD_TREE_COLUMN(8, "texture_cols");
	for (uint32_t i = 0; i < display->file->ribbons_nb; ++i)
	{
		const struct wow_m2_ribbon *ribbon = &display->file->ribbons[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_U64(0, i);
		SET_TREE_VALUE_U64(1, ribbon->ribbon_id);
		SET_TREE_VALUE_U64(2, ribbon->bone_index);
		SET_TREE_VALUE_FMT(3, "{%f, %f, %f}", ribbon->position.x, ribbon->position.y, ribbon->position.z);
		SET_TREE_VALUE_FLT(4, ribbon->edges_per_second);
		SET_TREE_VALUE_FLT(5, ribbon->edge_lifetime);
		SET_TREE_VALUE_FLT(6, ribbon->gravity);
		SET_TREE_VALUE_U64(7, ribbon->texture_rows);
		SET_TREE_VALUE_U64(8, ribbon->texture_cols);
	}
	TREE_END();
}

static void on_gtk_block_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data)
{
	(void)column;
	struct m2_display *display = data;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(tree);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
	uint32_t val;
	gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 1, &val, -1);
	if (!val)
		return;
	GtkWidget *scrolled = gtk_paned_get_end_child(GTK_PANED(display->display.root));
	if (!scrolled)
		return;
	GtkWidget *child;
	switch (val & 0xFF)
	{
		case M2_CATEGORY_HEADER:
			child = build_main_header(display);
			break;
		case M2_CATEGORY_TEXTURE_COMBINERS_COMBOS:
			child = build_texture_combiners_combos(display);
			break;
		case M2_CATEGORY_TEXTURE_UNIT_LOOKUPS:
			child = build_texture_unit_lookups(display);
			break;
		case M2_CATEGORY_TEXTURE_LOOKUPS:
			child = build_texture_lookups(display);
			break;
		case M2_CATEGORY_TEXTURE_TRANSFORMS_LOOKUPS:
			child = build_texture_transforms_lookups(display);
			break;
		case M2_CATEGORY_TEXTURE_TRANSFORMS:
			child = build_texture_transforms(display);
			break;
		case M2_CATEGORY_TEXTURE_WEIGHTS_LOOKUPS:
			child = build_texture_weights_lookups(display);
			break;
		case M2_CATEGORY_TEXTURE_WEIGHTS:
			child = build_texture_weights(display);
			break;
		case M2_CATEGORY_SKIN_PROFILES:
			child = build_skin_profile(display, val >> 8);
			break;
		case M2_CATEGORY_MATERIALS:
			child = build_materials(display);
			break;
		case M2_CATEGORY_SEQUENCES:
			child = build_sequences(display);
			break;
		case M2_CATEGORY_PARTICLES:
			child = build_particles(display);
			break;
		case M2_CATEGORY_TEXTURES:
			child = build_textures(display);
			break;
		case M2_CATEGORY_CAMERAS:
			child = build_cameras(display);
			break;
		case M2_CATEGORY_CAMERA_LOOKUPS:
			child = build_camera_lookups(display);
			break;
		case M2_CATEGORY_RIBBONS:
			child = build_ribbons(display);
			break;
		case M2_CATEGORY_LIGHTS:
			child = build_lights(display);
			break;
		case M2_CATEGORY_BONES:
			child = build_bones(display);
			break;
		default:
			child = NULL;
			break;
	}
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), child);
}

GtkWidget *build_tree(struct m2_display *display, struct wow_m2_file *file)
{
	GtkTreeStore *store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	GtkWidget *tree = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), false);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	g_signal_connect(tree, "row-activated", G_CALLBACK(on_gtk_block_row_activated), display);
	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(tree), true);
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Blocks", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	gtk_widget_set_vexpand(tree, true);
	GtkTreeIter iter;
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Header", 1, M2_CATEGORY_HEADER, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Texture combiners combos", 1, M2_CATEGORY_TEXTURE_COMBINERS_COMBOS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Texture unit lookups", 1, M2_CATEGORY_TEXTURE_UNIT_LOOKUPS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Texture lookups", 1, M2_CATEGORY_TEXTURE_LOOKUPS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Texture transforms lookups", 1, M2_CATEGORY_TEXTURE_TRANSFORMS_LOOKUPS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Texture transforms", 1, M2_CATEGORY_TEXTURE_TRANSFORMS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Texture weights lookups", 1, M2_CATEGORY_TEXTURE_WEIGHTS_LOOKUPS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Texture weights", 1, M2_CATEGORY_TEXTURE_WEIGHTS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Skin profiles", 1, M2_CATEGORY_SKIN_PROFILES, -1);
	for (uint32_t i = 0; i < file->skin_profiles_nb; ++i)
	{
		GtkTreeIter child;
		GtkTreeIter child2;
		char name[14];
		snprintf(name, sizeof(name), "%" PRIu32, i + 1);
		gtk_tree_store_append(store, &child, &iter);
		gtk_tree_store_set(store, &child, 0, name, 1, M2_CATEGORY_SKIN_PROFILES + 0x100 * (i + 1), -1);
		gtk_tree_store_append(store, &child2, &child);
		gtk_tree_store_set(store, &child2, 0, "Sections", 1, M2_CATEGORY_SKIN_PROFILES + 0x100 * (i + 1) + 0x10000, -1);
		gtk_tree_store_append(store, &child2, &child);
		gtk_tree_store_set(store, &child2, 0, "Batches", 1, M2_CATEGORY_SKIN_PROFILES + 0x100 * (i + 1) + 0x20000, -1);
		gtk_tree_store_append(store, &child2, &child);
		gtk_tree_store_set(store, &child2, 0, "Vertexes", 1, M2_CATEGORY_SKIN_PROFILES + 0x100 * (i + 1) + 0x30000, -1);
		gtk_tree_store_append(store, &child2, &child);
		gtk_tree_store_set(store, &child2, 0, "Indices", 1, M2_CATEGORY_SKIN_PROFILES + 0x100 * (i + 1) + 0x40000, -1);
		gtk_tree_store_append(store, &child2, &child);
		gtk_tree_store_set(store, &child2, 0, "Bones", 1, M2_CATEGORY_SKIN_PROFILES + 0x100 * (i + 1) + 0x50000, -1);
	}
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Materials", 1, M2_CATEGORY_MATERIALS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Sequences", 1, M2_CATEGORY_SEQUENCES, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Particles", 1, M2_CATEGORY_PARTICLES, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Textures", 1, M2_CATEGORY_TEXTURES, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Cameras", 1, M2_CATEGORY_CAMERAS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Camera lookups", 1, M2_CATEGORY_CAMERA_LOOKUPS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Ribbons", 1, M2_CATEGORY_RIBBONS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Lights", 1, M2_CATEGORY_LIGHTS, -1);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, "Bones", 1, M2_CATEGORY_BONES, -1);
	gtk_widget_show(tree);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store));
	return tree;
}
