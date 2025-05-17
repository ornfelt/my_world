#include "../nodes.h"

#include "display.h"

#include <wow/dbc.h>

#include <inttypes.h>
#include <stdbool.h>

#define VALUE_SET_I64(v) \
do \
{ \
	g_value_init(&value, G_TYPE_INT64); \
	g_value_set_int64(&value, v); \
} while (0)

#define VALUE_SET_U64(v) \
do \
{ \
	g_value_init(&value, G_TYPE_UINT64); \
	g_value_set_uint64(&value, v); \
} while (0)

#define VALUE_SET_FLT(v) \
do \
{ \
	g_value_init(&value, G_TYPE_FLOAT); \
	g_value_set_float(&value, v); \
} while (0)

static const struct
{
	const char *file;
	const struct wow_dbc_def *def;
} defs[] =
{
	{"animationdata.dbc"            , wow_dbc_animation_data_def},
	{"areapoi.dbc"                  , wow_dbc_area_poi_def},
	{"areatable.dbc"                , wow_dbc_area_table_def},
	{"auctionhouse.dbc"             , wow_dbc_auction_house_def},
	{"charbaseinfo.dbc"             , wow_dbc_char_base_info_def},
	{"charhairgeosets.dbc"          , wow_dbc_char_hair_geosets_def},
	{"charhairtextures.dbc"         , wow_dbc_char_hair_textures_def},
	{"charsections.dbc"             , wow_dbc_char_sections_def},
	{"charstartoutfit.dbc"          , wow_dbc_char_start_outfit_def},
	{"chartitles.dbc"               , wow_dbc_char_titles_def},
	{"characterfacialhairstyles.dbc", wow_dbc_character_facial_hair_styles_def},
	{"chatprofanity.dbc"            , wow_dbc_chat_profanity_def},
	{"chrclasses.dbc"               , wow_dbc_chr_classes_def},
	{"chrraces.dbc"                 , wow_dbc_chr_races_def},
	{"creaturedisplayinfo.dbc"      , wow_dbc_creature_display_info_def},
	{"creaturedisplayinfoextra.dbc" , wow_dbc_creature_display_info_extra_def},
	{"creaturemodeldata.dbc"        , wow_dbc_creature_model_data_def},
	{"declinedword.dbc"             , wow_dbc_declined_word_def},
	{"declinedwordcases.dbc"        , wow_dbc_declined_word_cases_def},
	{"gameobjectdisplayinfo.dbc"    , wow_dbc_game_object_display_info_def},
	{"groundeffecttexture.dbc"      , wow_dbc_ground_effect_texture_def},
	{"groundeffectdoodad.dbc"       , wow_dbc_ground_effect_doodad_def},
	{"helmetgeosetvisdata.dbc"      , wow_dbc_helmet_geoset_vis_data_def},
	{"item.dbc"                     , wow_dbc_item_def},
	{"itemclass.dbc"                , wow_dbc_item_class_def},
	{"itemdisplayinfo.dbc"          , wow_dbc_item_display_info_def},
	{"itemset.dbc"                  , wow_dbc_item_set_def},
	{"itemsubclass.dbc"             , wow_dbc_item_sub_class_def},
	{"languagewords.dbc"            , wow_dbc_language_words_def},
	{"languages.dbc"                , wow_dbc_languages_def},
	{"light.dbc"                    , wow_dbc_light_def},
	{"lightfloatband.dbc"           , wow_dbc_light_float_band_def},
	{"lightintband.dbc"             , wow_dbc_light_int_band_def},
	{"lightparams.dbc"              , wow_dbc_light_params_def},
	{"lightskybox.dbc"              , wow_dbc_light_skybox_def},
	{"liquidtype.dbc"               , wow_dbc_liquid_type_def},
	{"loadingscreens.dbc"           , wow_dbc_loading_screens_def},
	{"loadingscreentaxisplines.dbc" , wow_dbc_loading_screen_taxi_splines_def},
	{"map.dbc"                      , wow_dbc_map_def},
	{"namegen.dbc"                  , wow_dbc_name_gen_def},
	{"startup_strings.dbc"          , wow_dbc_startup_strings_def},
	{"stringlookups.dbc"            , wow_dbc_string_lookups_def},
	{"soundentries.dbc"             , wow_dbc_sound_entries_def},
	{"spell.dbc"                    , wow_dbc_spell_def},
	{"spellicon.dbc"                , wow_dbc_spell_icon_def},
	{"talent.dbc"                   , wow_dbc_talent_def},
	{"talenttab.dbc"                , wow_dbc_talent_tab_def},
	{"taxinodes.dbc"                , wow_dbc_taxi_nodes_def},
	{"taxipath.dbc"                 , wow_dbc_taxi_path_def},
	{"taxipathnode.dbc"             , wow_dbc_taxi_path_node_def},
	{"worldmaparea.dbc"             , wow_dbc_world_map_area_def},
	{"worldmapcontinent.dbc"        , wow_dbc_world_map_continent_def},
	{"worldmapoverlay.dbc"          , wow_dbc_world_map_overlay_def},
	{"worldmaptransforms.dbc"       , wow_dbc_world_map_transforms_def},
	{"wowerror_strings.dbc"         , wow_dbc_wow_error_strings_def},
};

struct dbc_display
{
	struct display display;
	struct wow_dbc_file *file;
};

static void dtr(struct display *ptr)
{
	struct dbc_display *display = (struct dbc_display*)ptr;
	wow_dbc_file_delete(display->file);
}

struct display *dbc_display_new(const struct node *node, const char *path, struct wow_mpq_file *mpq_file)
{
	(void)path;
	struct wow_dbc_file *file = wow_dbc_file_new(mpq_file);
	if (!file)
	{
		fprintf(stderr, "failed to parse dbc file\n");
		return NULL;
	}
	struct dbc_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "dbc display allocation failed\n");
		wow_dbc_file_delete(file);
		return NULL;
	}
	display->display.name = strdup(path);
	display->display.dtr = dtr;
	display->file = file;
	const struct wow_dbc_def *def = NULL;
	for (size_t i = 0; i < sizeof(defs) / sizeof(*defs); ++i)
	{
		if (!strcmp(defs[i].file, node->name))
		{
			def = defs[i].def;
			break;
		}
	}
	/* Tree */
	GType types[512];
	size_t types_nb = 0;
	if (def)
	{
		while (def[types_nb].type != WOW_DBC_TYPE_END)
		{
			switch (def[types_nb].type)
			{
				case WOW_DBC_TYPE_I8:
				case WOW_DBC_TYPE_I16:
				case WOW_DBC_TYPE_I32:
				case WOW_DBC_TYPE_I64:
					types[types_nb++] = G_TYPE_INT64;
					break;
				case WOW_DBC_TYPE_U8:
				case WOW_DBC_TYPE_U16:
				case WOW_DBC_TYPE_U32:
				case WOW_DBC_TYPE_U64:
					types[types_nb++] = G_TYPE_UINT64;
					break;
				case WOW_DBC_TYPE_FLT:
					types[types_nb++] = G_TYPE_FLOAT;
					break;
				default:
					types[types_nb++] = G_TYPE_STRING;
					break;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < file->header.record_size / 4; ++i)
			types[types_nb++] = G_TYPE_STRING;
	}
	GtkListStore *store = gtk_list_store_newv(types_nb, types);
	GtkWidget *tree = gtk_tree_view_new();
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(tree), true);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	if (def)
	{
		for (uint32_t i = 0; def[i].type != WOW_DBC_TYPE_END; ++i)
		{
			renderer = gtk_cell_renderer_text_new();
			char row_name[256];
			size_t len = strlen(def[i].name);
			char *tmp = row_name;;
			for (size_t j = 0; j < len && (unsigned)(tmp - row_name) < sizeof(row_name) - 1; ++j)
			{
				*tmp = def[i].name[j];
				if (*tmp == '_')
					*(++tmp) = '_';
				tmp++;
			}
			*tmp = '\0';
			GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(row_name, renderer, "text", i, NULL);
			gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
			gtk_tree_view_column_set_resizable(column, true);
			gtk_tree_view_column_set_sort_column_id(column, i);
			gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
		}
	}
	else
	{
		for (uint32_t i = 0; i < file->header.record_size / 4; ++i)
		{
			renderer = gtk_cell_renderer_text_new();
			char row_name[64];
			snprintf(row_name, sizeof(row_name), "row %" PRIu32, i);
			GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(row_name, renderer, "text", i, NULL);
			gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
			gtk_tree_view_column_set_resizable(column, true);
			gtk_tree_view_column_set_sort_column_id(column, i);
			gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
		}
	}
	for (size_t i = 0; i < file->header.record_count; ++i)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		struct wow_dbc_row row = wow_dbc_get_row(file, i);
		if (def)
		{
			size_t j = 0;
			for (uint32_t idx = 0; def[idx].type != WOW_DBC_TYPE_END; ++idx)
			{
				GValue value = G_VALUE_INIT;
				char str[512];
				switch (def[idx].type)
				{
					case WOW_DBC_TYPE_I8:
						VALUE_SET_I64(wow_dbc_get_i8(&row, j));
						j += 1;
						break;
					case WOW_DBC_TYPE_U8:
						VALUE_SET_U64(wow_dbc_get_u8(&row, j));
						j += 1;
						break;
					case WOW_DBC_TYPE_I16:
						VALUE_SET_I64(wow_dbc_get_i16(&row, j));
						j += 2;
						break;
					case WOW_DBC_TYPE_U16:
						VALUE_SET_U64(wow_dbc_get_u16(&row, j));
						j += 2;
						break;
					case WOW_DBC_TYPE_I32:
						VALUE_SET_I64(wow_dbc_get_i32(&row, j));
						j += 4;
						break;
					case WOW_DBC_TYPE_U32:
						VALUE_SET_U64(wow_dbc_get_u32(&row, j));
						j += 4;
						break;
					case WOW_DBC_TYPE_I64:
						VALUE_SET_I64(wow_dbc_get_i64(&row, j));
						j += 8;
						break;
					case WOW_DBC_TYPE_U64:
						VALUE_SET_U64(wow_dbc_get_u64(&row, j));
						j += 8;
						break;
					case WOW_DBC_TYPE_STR:
						g_value_init(&value, G_TYPE_STRING);
						snprintf(str, sizeof(str), "%s", wow_dbc_get_str(&row, j));
						g_value_set_string(&value, str);
						j += 4;
						break;
					case WOW_DBC_TYPE_LSTR:
						g_value_init(&value, G_TYPE_STRING);
						snprintf(str, sizeof(str), "%s", wow_dbc_get_str(&row, j + 8));
						g_value_set_string(&value, str);
						j += 4 * 17;
						break;
					case WOW_DBC_TYPE_FLT:
						VALUE_SET_FLT(wow_dbc_get_flt(&row, j));
						j += 4;
						break;
					case WOW_DBC_TYPE_END:
						break;
				}
				gtk_list_store_set_value(store, &iter, idx, &value);
			}
		}
		else
		{
			GValue value = G_VALUE_INIT;
			g_value_init(&value, G_TYPE_STRING);
			for (uint32_t j = 0; j < file->header.record_size / 4; ++j)
			{
				char str[64];
				snprintf(str, sizeof(str), "%" PRIu32, wow_dbc_get_u32(&row, j * 4));
				g_value_set_string(&value, str);
				gtk_list_store_set_value(store, &iter, j, &value);
			}
		}
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store));
	gtk_widget_show(tree);
	display->display.root = tree;
	return &display->display;
}
