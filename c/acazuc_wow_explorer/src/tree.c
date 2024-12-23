#include "explorer.h"
#include "nodes.h"
#include "tree.h"

#include <wow/mpq.h>
#include <wow/blp.h>

#include <sys/stat.h>

#include <ctype.h>

static void on_gtk_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data);
static void on_gtk_row_button_pressed(GtkGestureClick *gesture, int n_press, double x, double y, GtkWidget *widget);
static void add_child(struct tree *tree, GtkTreeIter *parent, struct node *node);

struct tree *tree_new(struct explorer *explorer)
{
	struct tree *tree = malloc(sizeof(*tree));
	if (!tree)
		return NULL;
	tree->explorer = explorer;
	tree->store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	tree->treeview = gtk_tree_view_new();
	tree->renderer = gtk_cell_renderer_text_new();
	tree->column = gtk_tree_view_column_new_with_attributes(NULL, tree->renderer, "text", 0, NULL);
	g_signal_connect(tree->treeview, "row-activated", G_CALLBACK(on_gtk_row_activated), tree);
	gtk_widget_add_css_class(tree->treeview, "navigation-sidebar");
	GtkGesture *gesture = gtk_gesture_click_new ();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE (gesture), 3);
	g_signal_connect(gesture, "pressed", G_CALLBACK(on_gtk_row_button_pressed), tree->treeview);
	gtk_widget_add_controller(tree->treeview, GTK_EVENT_CONTROLLER(gesture));
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree->treeview), tree->column);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree->treeview), false);
	struct node *node;
	TAILQ_FOREACH(node, &tree->explorer->root->childs, chain)
		add_child(tree, NULL, node);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree->treeview), GTK_TREE_MODEL(tree->store));
	gtk_widget_show(tree->treeview);
	return tree;
}

void tree_delete(struct tree *tree)
{
	if (!tree)
		return;
	free(tree);
}

static void on_gtk_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data)
{
	(void)treeview;
	(void)column;
	struct tree *tree = data;
	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(tree->store), &iter, path);
	struct node *node;
	gtk_tree_model_get(GTK_TREE_MODEL(tree->store), &iter, 1, &node, -1);
	if (node)
		node_on_click(node);
}

static void copy_path(GtkWidget *widget, gpointer data)
{
	gtk_popover_popdown(GTK_POPOVER(gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(widget)))));
	GdkClipboard *clipboard = gtk_widget_get_clipboard(g_explorer->window);
	char path[4096];
	struct node *node = data;
	node_get_path(node, path, sizeof(path));
	gdk_clipboard_set_text(clipboard, path);
}

static void save_mpq_file(struct wow_mpq_file *file, const char *path)
{
	FILE *fp = fopen(path, "wb");
	if (!fp)
	{
		fprintf(stderr, "failed to open '%s': %s\n", path, strerror(errno));
		return;
	}
	if (fwrite(file->data, 1, file->size, fp) != file->size)
		fprintf(stderr, "failed to write data\n");
	fclose(fp);
	printf("saved '%s'\n", path);
}

static void save_node(struct node *node, const char *path)
{
	if (!TAILQ_EMPTY(&node->childs))
	{
		mkdir(path, 0755); /* best effort.. */
		struct node *child;
		TAILQ_FOREACH(child, &node->childs, chain)
		{
			char child_path[4096];
			snprintf(child_path, sizeof(child_path), "%s/%s", path, child->name);
			save_node(child, child_path);
		}
		return;
	}
	char mpq_path[4096];
	node_get_path(node, mpq_path, sizeof(mpq_path));
	wow_mpq_normalize_mpq_fn(mpq_path, sizeof(mpq_path));
	struct wow_mpq_file *file = wow_mpq_get_file(g_explorer->mpq_compound, mpq_path);
	if (file)
	{
		save_mpq_file(file, path);
		wow_mpq_file_delete(file);
	}
	else
	{
		fprintf(stderr, "failed to open mpq '%s'", mpq_path);
	}
}

static void on_extract_response(GtkDialog *dialog, gint id, gpointer user_data)
{
	if (id != GTK_RESPONSE_ACCEPT)
		return;
	struct node *node = user_data;
	GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
	char *filename = g_file_get_path(file);
	save_node(node, filename);
	g_object_unref(file);
}

static void node_extract(GtkWidget *widget, gpointer data)
{
	gtk_popover_popdown(GTK_POPOVER(gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(widget)))));
	struct node *node = data;
	GtkWidget *dialog = gtk_file_chooser_dialog_new("destination directory", GTK_WINDOW(g_explorer->window), GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
	gtk_file_chooser_set_current_name(chooser, node->name);
	gtk_file_chooser_set_create_folders(chooser, TRUE);
	g_signal_connect(dialog, "response", G_CALLBACK(on_extract_response), node);
	gtk_widget_show(dialog);
}

#include <wow/blp.h>
#include <wow/m2.h>
#include <jansson.h>
#include <png.h>

static const char *animation_names[] =
{
	"Stand",
	"Death",
	"Spell",
	"Stop",
	"Walk",
	"Run",
	"Dead",
	"Rise",
	"StandWound",
	"CombatWound",
	"CombatCritical",
	"ShuffleLeft",
	"ShuffleRight",
	"Walkbackwards",
	"Stun",
	"HandsClosed",
	"AttackUnarmed",
	"Attack1H",
	"Attack2H",
	"Attack2HL",
	"ParryUnarmed",
	"Parry1H",
	"Parry2H",
	"Parry2HL",
	"ShieldBlock",
	"ReadyUnarmed",
	"Ready1H",
	"Ready2H",
	"Ready2HL",
	"ReadyBow",
	"Dodge",
	"SpellPrecast",
	"SpellCast",
	"SpellCastArea",
	"NPCWelcome",
	"NPCGoodbye",
	"Block",
	"JumpStart",
	"Jump",
	"JumpEnd",
	"Fall",
	"SwimIdle",
	"Swim",
	"SwimLeft",
	"SwimRight",
	"SwimBackwards",
	"AttackBow",
	"FireBow",
	"ReadyRifle",
	"AttackRifle",
	"Loot",
	"ReadySpellDirected",
	"ReadySpellOmni",
	"SpellCastDirected",
	"SpellCastOmni",
	"BattleRoar",
	"ReadyAbility",
	"Special1H",
	"Special2H",
	"ShieldBash",
	"EmoteTalk",
	"EmoteEat",
	"EmoteWork",
	"EmoteUseStanding",
	"EmoteTalkExclamation",
	"EmoteTalkQuestion",
	"EmoteBow",
	"EmoteWave",
	"EmoteCheer",
	"EmoteDance",
	"EmoteLaugh",
	"EmoteSleep",
	"EmoteSitGround",
	"EmoteRude",
	"EmoteRoar",
	"EmoteKneel",
	"EmoteKiss",
	"EmoteCry",
	"EmoteChicken",
	"EmoteBeg",
	"EmoteApplaud",
	"EmoteShout",
	"EmoteFlex",
	"EmoteShy",
	"EmotePoint",
	"Attack1HPierce",
	"Attack2HLoosePierce",
	"AttackOff",
	"AttackOffPierce",
	"Sheath",
	"HipSheath",
	"Mount",
	"RunRight",
	"RunLeft",
	"MountSpecial",
	"Kick",
	"SitGroundDown",
	"SitGround",
	"SitGroundUp",
	"SleepDown",
	"Sleep",
	"SleepUp",
	"SitChairLow",
	"SitChairMed",
	"SitChairHigh",
	"LoadBow",
	"LoadRifle",
	"AttackThrown",
	"ReadyThrown",
	"HoldBow",
	"HoldRifle",
	"HoldThrown",
	"LoadThrown",
	"EmoteSalute",
	"KneelStart",
	"KneelLoop",
	"KneelEnd",
	"AttackUnarmedOff",
	"SpecialUnarmed",
	"StealthWalk",
	"StealthStand",
	"Knockdown",
	"EatingLoop",
	"UseStandingLoop",
	"ChannelCastDirected",
	"ChannelCastOmni",
	"Whirlwind",
	"Birth",
	"UseStandingStart",
	"UseStandingEnd",
	"CreatureSpecial",
	"Drown",
	"Drowned",
	"FishingCast",
	"FishingLoop",
	"Fly",
	"EmoteWorkNoSheathe",
	"EmoteStunNoSheathe",
	"EmoteUseStandingNoSheathe",
	"SpellSleepDown",
	"SpellKneelStart",
	"SpellKneelLoop",
	"SpellKneelEnd",
	"Sprint",
	"InFlight",
	"Spawn",
	"Close",
	"Closed",
	"Open",
	"Opened",
	"Destroy",
	"Destroyed",
	"Rebuild",
	"Custom0",
	"Custom1",
	"Custom2",
	"Custom3",
	"Despawn",
	"Hold",
	"Decay",
	"BowPull",
	"BowRelease",
	"ShipStart",
	"ShipMoving",
	"ShipStop",
	"GroupArrow",
	"Arrow",
	"CorpseArrow",
	"GuideArrow",
	"Sway",
	"DruidCatPounce",
	"DruidCatRip",
	"DruidCatRake",
	"DruidCatRavage",
	"DruidCatClaw",
	"DruidCatCower",
	"DruidBearSwipe",
	"DruidBearBite",
	"DruidBearMaul",
	"DruidBearBash",
	"DragonTail",
	"DragonStomp",
	"DragonSpit",
	"DragonSpitHover",
	"DragonSpitFly",
	"EmoteYes",
	"EmoteNo",
	"JumpLandRun",
	"LootHold",
	"LootUp",
	"StandHigh",
	"Impact",
	"LiftOff",
	"Hover",
	"SuccubusEntice",
	"EmoteTrain",
	"EmoteDead",
	"EmoteDanceOnce",
	"Deflect",
	"EmoteEatNoSheathe",
	"Land",
	"Submerge",
	"Submerged",
	"Cannibalize",
	"ArrowBirth",
	"GroupArrowBirth",
	"CorpseArrowBirth",
	"GuideArrowBirth",
	"EmoteTalkNoSheathe",
	"EmotePointNoSheathe",
	"EmoteSaluteNoSheathe",
	"EmoteDanceSpecial",
	"Mutilate",
	"CustomSpell01",
	"CustomSpell02",
	"CustomSpell03",
	"CustomSpell04",
	"CustomSpell05",
	"CustomSpell06",
	"CustomSpell07",
	"CustomSpell08",
	"CustomSpell09",
	"CustomSpell10",
	"StealthRun",
	"Emerge",
	"Cower",
};

bool png_write(const char *file, const uint8_t *data, uint32_t width, uint32_t height)
{
	png_bytep row_pointers[height];
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	FILE *fp = NULL;
	if (!(fp = fopen(file, "wb")))
		goto error1;
	if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
		goto error2;
	if (!(info_ptr = png_create_info_struct(png_ptr)))
		goto error3;
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	png_init_io(png_ptr, fp);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	for (uint32_t i = 0; i < height; ++i)
		row_pointers[i] = (uint8_t*)data + i * width * 4;
	png_write_image(png_ptr, row_pointers);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	png_write_end(png_ptr, NULL);
	fclose(fp);
	return true;
error3:
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
error2:
	fclose(fp);
error1:
	return false;
}

static json_t *json_accessor(int buffer_view, int offset, int component_type,
                             int normalized, int count, const char *type)
{
	json_t *accessor = json_object();
	json_object_set_new(accessor, "bufferView", json_integer(buffer_view));
	json_object_set_new(accessor, "byteOffset", json_integer(offset));
	json_object_set_new(accessor, "componentType", json_integer(component_type));
	json_object_set_new(accessor, "normalized", normalized ? json_true() : json_false());
	json_object_set_new(accessor, "count", json_integer(count));
	json_object_set_new(accessor, "type", json_string(type));
	return accessor;
}

static json_t *json_buffer_view(int buffer, int offset, int length, int stride,
                                int target)
{
	json_t *buffer_view = json_object();
	json_object_set_new(buffer_view, "buffer", json_integer(buffer));
	json_object_set_new(buffer_view, "byteOffset", json_integer(offset));
	json_object_set_new(buffer_view, "byteLength", json_integer(length));
	if (stride)
		json_object_set_new(buffer_view, "byteStride", json_integer(stride));
	if (target)
		json_object_set_new(buffer_view, "target", json_integer(target));
	return buffer_view;
}

static json_t *json_buffer(const char *directory, const char *model_name, const char *name,
                           const void *data, int length)
{
	char filename[512];
	snprintf(filename, sizeof(filename), "%s_%s.bin", model_name, name);
	char filepath[1024];
	snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
	FILE *fp = fopen(filepath, "wb");
	fwrite(data, 1, length, fp);
	fclose(fp);
	json_t *buffer = json_object();
	json_object_set_new(buffer, "byteLength", json_integer(length));
	json_object_set_new(buffer, "uri", json_string(filename));
	return buffer;
}

static json_t *json_blp(const char *directory, const char *model_name, const char *path)
{
	char texture_name[1024];
	json_t *image = json_object();
	if (!path || !path[0])
	{
		static const uint8_t data[4] = {0xFF, 0xFF, 0xFF, 0xFF};
		snprintf(texture_name, sizeof(texture_name), "%s_placeholder.png", model_name);
		char texture_path[2048];
		snprintf(texture_path, sizeof(texture_path), "%s/%s", directory, texture_name);
		png_write(texture_path, data, 1, 1);
		json_object_set_new(image, "uri", json_string(texture_name));
		return image;
	}
	struct wow_mpq_file *file = wow_mpq_get_file(g_explorer->mpq_compound, path);
	if (!file)
		return image;
	struct wow_blp_file *blp = wow_blp_file_new(file);
	wow_mpq_file_delete(file);
	if (!blp)
		return image;
	uint8_t *data;
	uint32_t width;
	uint32_t height;
	if (!wow_blp_decode_rgba(blp, 0, &width, &height, &data))
	{
		wow_blp_file_delete(blp);
		return image;
	}
	wow_blp_file_delete(blp);
	const char *base = strrchr(path, '\\');
	if (base)
		base++;
	else
		base = path;
	snprintf(texture_name, sizeof(texture_name), "%s_%.*s.png", model_name, (int)strlen(base) - 4, base);
	for (size_t i = 0; texture_name[i]; ++i)
		texture_name[i] = tolower(texture_name[i]);
	char texture_path[2048];
	snprintf(texture_path, sizeof(texture_path), "%s/%s", directory, texture_name);
	png_write(texture_path, data, width, height);
	free(data);

	json_object_set_new(image, "uri", json_string(texture_name));
	return image;
}

static void gltf_batches(const struct wow_m2_file *m2, json_t *nodes)
{
	const struct wow_m2_skin_profile *skin_profile = &m2->skin_profiles[0];
	for (size_t i = 0; i < skin_profile->batches_nb; ++i)
	{
		char name[256];
		snprintf(name, sizeof(name), "batch_%u", (unsigned)i);
		json_t *jnode = json_object();
		json_object_set_new(jnode, "mesh", json_integer(i));
		json_object_set_new(jnode, "name", json_string(name));
		json_object_set_new(jnode, "skin", json_integer(0));
		json_array_append_new(nodes, jnode);
	}
}

static float *gltf_bones(const struct wow_m2_file *m2, json_t *nodes)
{
	const struct wow_m2_skin_profile *skin_profile = &m2->skin_profiles[0];
	float *bones_mats = malloc(sizeof(float) * 16 * m2->bones_nb);
	for (size_t i = 0; i < m2->bones_nb; ++i)
	{
		const struct wow_m2_bone *m2_bone = &m2->bones[i];
		char name[256];
		snprintf(name, sizeof(name), "bone_%u", (unsigned)i);
		json_t *jnode = json_object();
		json_object_set_new(jnode, "name", json_string(name));
		json_t *children = json_array();
		for (size_t j = 0; j < m2->bones_nb; ++j)
		{
			const struct wow_m2_bone *child = &m2->bones[j];
			if ((size_t)child->parent_bone == i)
				json_array_append_new(children, json_integer(skin_profile->batches_nb + 1 + j));
		}
		json_object_set_new(jnode, "children", children);
		struct wow_vec3f pivot = m2_bone->pivot;
		if (m2_bone->parent_bone != -1)
		{
			struct wow_m2_bone *parent = &m2->bones[m2_bone->parent_bone];
			pivot.x -= parent->pivot.x;
			pivot.y -= parent->pivot.y;
			pivot.z -= parent->pivot.z;
		}
		json_t *translation = json_array();
		json_array_append_new(translation, json_real(pivot.x));
		json_array_append_new(translation, json_real(pivot.y));
		json_array_append_new(translation, json_real(pivot.z));
		json_object_set_new(jnode, "translation", translation);
		json_array_append_new(nodes, jnode);
		bones_mats[i * 0x10 + 0x0] = 1;
		bones_mats[i * 0x10 + 0x1] = 0;
		bones_mats[i * 0x10 + 0x2] = 0;
		bones_mats[i * 0x10 + 0x3] = 0;
		bones_mats[i * 0x10 + 0x4] = 0;
		bones_mats[i * 0x10 + 0x5] = 1;
		bones_mats[i * 0x10 + 0x6] = 0;
		bones_mats[i * 0x10 + 0x7] = 0;
		bones_mats[i * 0x10 + 0x8] = 0;
		bones_mats[i * 0x10 + 0x9] = 0;
		bones_mats[i * 0x10 + 0xA] = 1;
		bones_mats[i * 0x10 + 0xB] = 0;
		bones_mats[i * 0x10 + 0xC] = -m2_bone->pivot.x;
		bones_mats[i * 0x10 + 0xD] = -m2_bone->pivot.y;
		bones_mats[i * 0x10 + 0xE] = -m2_bone->pivot.z;
		bones_mats[i * 0x10 + 0xF] = 1;
	}
	return bones_mats;
}

static void gltf_textures(const struct wow_m2_file *m2, json_t *root,
                          const char *directory_path, const char *model_name)
{
	json_t *images = json_array();
	json_t *samplers = json_array();
	json_t *textures = json_array();
	for (size_t i = 0; i < m2->textures_nb; ++i)
	{
		const struct wow_m2_texture *m2_texture = &m2->textures[i];
		json_t *image = json_blp(directory_path, model_name, m2_texture->filename);
		json_t *sampler = json_object();
		json_t *texture = json_object();
		json_object_set_new(sampler, "magFilter", json_integer(9987));
		json_object_set_new(sampler, "minFilter", json_integer(9987));
		json_object_set_new(sampler, "wrapS", json_integer((m2_texture->flags & WOW_M2_TEXTURE_FLAG_CLAMP_S) ? 10497 : 33071));
		json_object_set_new(sampler, "wrapT", json_integer((m2_texture->flags & WOW_M2_TEXTURE_FLAG_CLAMP_T) ? 10497 : 33071));
		json_object_set_new(texture, "sampler", json_integer(i));
		json_object_set_new(texture, "source", json_integer(i));
		json_array_append_new(images, image);
		json_array_append_new(samplers, sampler);
		json_array_append_new(textures, texture);
	}
	json_object_set_new(root, "images", images);
	json_object_set_new(root, "samplers", samplers);
	json_object_set_new(root, "textures", textures);
}

static void gltf_skins(const struct wow_m2_file *m2, json_t *root)
{
	const struct wow_m2_skin_profile *skin_profile = &m2->skin_profiles[0];
	json_t *skins = json_array();
	json_t *skin = json_object();
	json_t *joints = json_array();
	for (size_t i = 0; i < m2->bones_nb; ++i)
		json_array_append_new(joints, json_integer(skin_profile->batches_nb + 1 + i));
	json_object_set_new(skin, "joints", joints);
	json_object_set_new(skin, "inverseBindMatrices", json_integer(6 + skin_profile->sections_nb));
	json_object_set_new(skin, "skeleton", json_integer(skin_profile->batches_nb + 1));
	json_array_append_new(skins, skin);
	json_object_set_new(root, "skins", skins);
}

static void gltf_materials(const struct wow_m2_file *m2, json_t *root)
{
	const struct wow_m2_skin_profile *skin_profile = &m2->skin_profiles[0];
	json_t *materials = json_array();
	for (size_t i = 0; i < skin_profile->batches_nb; ++i)
	{
		const struct wow_m2_batch *m2_batch = &skin_profile->batches[i];
		const struct wow_m2_material *m2_material = &m2->materials[m2_batch->material_index];
		json_t *material = json_object();
		json_t *pbr = json_object();
		json_t *base_color = json_object();
		json_object_set_new(base_color, "index", json_integer(m2->texture_lookups[m2_batch->texture_combo_index]));
		json_object_set_new(base_color, "texCoord", json_integer(0));
		json_object_set_new(pbr, "baseColorTexture", base_color);
		json_object_set_new(material, "pbrMetallicRoughness", pbr);
		json_object_set_new(material, "doubleSided", (m2_material->flags & WOW_M2_MATERIAL_FLAGS_UNCULLED) ? json_true() : json_false());
		switch (m2_material->blend_mode)
		{
			case 0:
				json_object_set_new(material, "alphaMode", json_string("OPAQUE"));
				break;
			case 1:
				json_object_set_new(material, "alphaMode", json_string("MASK"));
				json_object_set_new(material, "alphaCutoff", json_real(224 / 255.));
				break;
			default:
				json_object_set_new(material, "alphaMode", json_string("OPAQUE")); /* XXX blend */
				break;
		}
		json_array_append_new(materials, material);
	}
	json_object_set_new(root, "materials", materials);
}

static void gltf_animation_buffers(const struct wow_m2_file *m2, json_t *buffers,
                                   const char *directory_path, const char *model_name)
{
	for (size_t i = 0; i < m2->bones_nb; ++i)
	{
		const struct wow_m2_bone *bone = &m2->bones[i];
		char filename[1024];
		char filepath[2048];
		snprintf(filename, sizeof(filename), "%s_bone_%d.bin", model_name, (int)i);
		snprintf(filepath, sizeof(filepath), "%s/%s", directory_path, filename);
		FILE *fp = fopen(filepath, "wb");
		size_t buf_len = 0;
		if (bone->translation.global_sequence == -1 && bone->translation.timestamps_nb)
		{
			float *timestamps = malloc(sizeof(float) * bone->translation.timestamps_nb);
			const struct wow_m2_sequence *current_sequence = NULL;
			for (size_t j = 0; j < bone->translation.timestamps_nb; ++j)
			{
				if (!current_sequence)
				{
					for (size_t k = 0; k < m2->sequences_nb; ++k)
					{
						const struct wow_m2_sequence *sequence = &m2->sequences[k];
						if (bone->translation.timestamps[j] >= sequence->start
						 && bone->translation.timestamps[j] <= sequence->end)
						{
							current_sequence = sequence;
							break;
						}
					}
				}
				if (current_sequence)
				{
					timestamps[j] = (bone->translation.timestamps[j] - current_sequence->start) / 1000.f;
					if (bone->translation.timestamps[j] >= current_sequence->end)
						current_sequence = NULL;
				}
				else
				{
					timestamps[j] = bone->translation.timestamps[j] / 1000.f;
				}
			}
			fwrite(timestamps, sizeof(float), bone->translation.timestamps_nb, fp);
			free(timestamps);
			struct wow_vec3f pivot = bone->pivot;
			if (bone->parent_bone != -1)
			{
				struct wow_m2_bone *parent = &m2->bones[bone->parent_bone];
				pivot.x -= parent->pivot.x;
				pivot.y -= parent->pivot.y;
				pivot.z -= parent->pivot.z;
			}
			float *values = malloc(sizeof(float) * 3 * bone->translation.values_nb);
			for (size_t j = 0; j < bone->translation.values_nb; ++j)
			{
				values[j * 3 + 0] = ((float*)bone->translation.values)[j * 3 + 0] + pivot.x;
				values[j * 3 + 1] = ((float*)bone->translation.values)[j * 3 + 1] + pivot.y;
				values[j * 3 + 2] = ((float*)bone->translation.values)[j * 3 + 2] + pivot.z;
			}
			fwrite(values, sizeof(float) * 3, bone->translation.values_nb, fp);
			free(values);
			buf_len += bone->translation.timestamps_nb * sizeof(float);
			buf_len += bone->translation.values_nb * sizeof(float) * 3;
		}
		if (bone->rotation.global_sequence == -1 && bone->rotation.timestamps_nb)
		{
			float *timestamps = malloc(sizeof(float) * bone->rotation.timestamps_nb);
			const struct wow_m2_sequence *current_sequence = NULL;
			for (size_t j = 0; j < bone->rotation.timestamps_nb; ++j)
			{
				if (!current_sequence)
				{
					for (size_t k = 0; k < m2->sequences_nb; ++k)
					{
						const struct wow_m2_sequence *sequence = &m2->sequences[k];
						if (bone->rotation.timestamps[j] >= sequence->start
						 && bone->rotation.timestamps[j] <= sequence->end)
						{
							current_sequence = sequence;
							break;
						}
					}
				}
				if (current_sequence)
				{
					timestamps[j] = (bone->rotation.timestamps[j] - current_sequence->start) / 1000.f;
					if (bone->rotation.timestamps[j] >= current_sequence->end)
						current_sequence = NULL;
				}
				else
				{
					timestamps[j] = bone->rotation.timestamps[j] / 1000.f;
				}
			}
			fwrite(timestamps, sizeof(float), bone->rotation.timestamps_nb, fp);
			free(timestamps);
			float *values = malloc(sizeof(float) * 4 * bone->rotation.values_nb);
			for (size_t j = 0; j < bone->rotation.values_nb * 4; ++j)
			{
				int16_t v = ((int16_t*)bone->rotation.values)[j];
				values[j] = (v < 0 ? v + 32768 : v - 32767) / 32767.f;
			}
			fwrite(values, sizeof(float) * 4, bone->rotation.values_nb, fp);
			free(values);
			buf_len += bone->rotation.timestamps_nb * sizeof(float);
			buf_len += bone->rotation.values_nb * sizeof(float) * 4;
		}
		if (bone->scale.global_sequence == -1 && bone->scale.timestamps_nb)
		{
			float *timestamps = malloc(sizeof(float) * bone->scale.timestamps_nb);
			const struct wow_m2_sequence *current_sequence = NULL;
			for (size_t j = 0; j < bone->scale.timestamps_nb; ++j)
			{
				if (!current_sequence)
				{
					for (size_t k = 0; k < m2->sequences_nb; ++k)
					{
						const struct wow_m2_sequence *sequence = &m2->sequences[k];
						if (bone->scale.timestamps[j] >= sequence->start
						 && bone->scale.timestamps[j] <= sequence->end)
						{
							current_sequence = sequence;
							break;
						}
					}
				}
				if (current_sequence)
				{
					timestamps[j] = (bone->scale.timestamps[j] - current_sequence->start) / 1000.f;
					if (bone->scale.timestamps[j] >= current_sequence->end)
						current_sequence = NULL;
				}
				else
				{
					timestamps[j] = bone->scale.timestamps[j] / 1000.f;
				}
			}
			fwrite(timestamps, sizeof(float), bone->scale.timestamps_nb, fp);
			free(timestamps);
			fwrite(bone->scale.values, sizeof(float) * 3, bone->scale.values_nb, fp);
			buf_len += bone->scale.timestamps_nb * sizeof(float);
			buf_len += bone->scale.values_nb * sizeof(float) * 3;
		}
		fclose(fp);
		json_t *buffer = json_object();
		json_object_set_new(buffer, "byteLength", json_integer(buf_len));
		json_object_set_new(buffer, "uri", json_string(filename));
		json_array_append_new(buffers, buffer);
	}
}

static void gltf_buffers(const struct wow_m2_file *m2, json_t *root,
                         uint16_t *indices, uint32_t indices_nb,
                         float *bones_mats, bool export_animations,
                         const char *directory_path, const char *model_name)
{
	json_t *buffers = json_array();
	json_array_append_new(buffers, json_buffer(directory_path, model_name, "vertexes", m2->vertexes, m2->vertexes_nb * sizeof(struct wow_m2_vertex)));
	json_array_append_new(buffers, json_buffer(directory_path, model_name, "indices", indices, indices_nb * sizeof(*indices)));
	json_array_append_new(buffers, json_buffer(directory_path, model_name, "bones", bones_mats, m2->bones_nb * sizeof(float) * 16));
	if (export_animations)
		gltf_animation_buffers(m2, buffers, directory_path, model_name);
	json_object_set_new(root, "buffers", buffers);
}

static void gltf_animation_buffer_views(const struct wow_m2_file *m2, json_t *buffer_views)
{
	for (size_t i = 0; i < m2->sequences_nb; ++i)
	{
		const struct wow_m2_sequence *sequence = &m2->sequences[i];
		for (size_t j = 0; j < m2->bones_nb; ++j)
		{
			const struct wow_m2_bone *bone = &m2->bones[j];
			size_t buf_base = 0;
			if (bone->translation.global_sequence == -1 && bone->translation.timestamps_nb)
			{
				size_t start = 0;
				while (start < bone->translation.timestamps_nb
				    && bone->translation.timestamps[start] < sequence->start)
					++start;
				size_t end = start;
				while (end < bone->translation.timestamps_nb
				    && bone->translation.timestamps[end] <= sequence->end)
					++end;
				json_array_append_new(buffer_views, json_buffer_view(3 + j, buf_base + start * sizeof(float), (end - start) * sizeof(float), 0, 0));
				buf_base += bone->translation.timestamps_nb * sizeof(float);
				json_array_append_new(buffer_views, json_buffer_view(3 + j, buf_base + start * sizeof(float) * 3, (end - start) * sizeof(float) * 3, 0, 0));
				buf_base += bone->translation.values_nb * sizeof(float) * 3;
			}
			if (bone->rotation.global_sequence == -1 && bone->rotation.timestamps_nb)
			{
				size_t start = 0;
				while (start < bone->rotation.timestamps_nb
				    && bone->rotation.timestamps[start] < sequence->start)
					++start;
				size_t end = start;
				while (end < bone->rotation.timestamps_nb
				    && bone->rotation.timestamps[end] <= sequence->end)
					++end;
				json_array_append_new(buffer_views, json_buffer_view(3 + j, buf_base + start * sizeof(float), (end - start) * sizeof(float), 0, 0));
				buf_base += bone->rotation.timestamps_nb * sizeof(float);
				json_array_append_new(buffer_views, json_buffer_view(3 + j, buf_base + start * sizeof(float) * 4, (end - start) * sizeof(float) * 4, 0, 0));
				buf_base += bone->rotation.values_nb * sizeof(float) * 4;
			}
			if (bone->scale.global_sequence == -1 && bone->scale.timestamps_nb)
			{
				size_t start = 0;
				while (start < bone->scale.timestamps_nb
				    && bone->scale.timestamps[start] < sequence->start)
					++start;
				size_t end = start;
				while (end < bone->scale.timestamps_nb
				    && bone->scale.timestamps[end] <= sequence->end)
					++end;
				json_array_append_new(buffer_views, json_buffer_view(3 + j, buf_base + start * sizeof(float), (end - start) * sizeof(float), 0, 0));
				buf_base += bone->scale.timestamps_nb * sizeof(float);
				json_array_append_new(buffer_views, json_buffer_view(3 + j, buf_base + start * sizeof(float) * 3, (end - start) * sizeof(float) * 3, 0, 0));
				buf_base += bone->scale.values_nb * sizeof(float) * 3;
			}
		}
	}
}

static void gltf_buffer_views(const struct wow_m2_file *m2, json_t *root,
                              uint16_t *indices, uint32_t indices_nb,
                              bool export_animations)
{
	json_t *buffer_views = json_array();
	json_array_append_new(buffer_views, json_buffer_view(0, 0, m2->vertexes_nb * sizeof(struct wow_m2_vertex), sizeof(struct wow_m2_vertex), 34962));
	json_array_append_new(buffer_views, json_buffer_view(1, 0, indices_nb * sizeof(*indices), 0, 34963));
	json_array_append_new(buffer_views, json_buffer_view(2, 0, m2->bones_nb * sizeof(float) * 16, 0, 0));
	if (export_animations)
		gltf_animation_buffer_views(m2, buffer_views);
	json_object_set_new(root, "bufferViews", buffer_views);
}

static void on_export_response(GtkDialog *dialog, gint id, gpointer user_data)
{
	if (id != GTK_RESPONSE_ACCEPT)
		return;
	struct node *node = user_data;
	GFile *directory = NULL;
	char *directory_path = NULL;
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
	directory = gtk_file_chooser_get_file(chooser);
	directory_path = g_file_get_path(directory);
	bool export_animations = !strcmp(gtk_file_chooser_get_choice(chooser, "export_animations"), "true");
	struct wow_m2_file *m2 = NULL;
	char tmp[1024];
	char path[512];
	node_get_path(node, path, sizeof(path));
	struct wow_mpq_file *file = wow_mpq_get_file(g_explorer->mpq_compound, path);
	if (!file)
		goto end;
	m2 = wow_m2_file_new(file);
	wow_mpq_file_delete(file);
	if (!m2)
		goto end;
	const struct wow_m2_skin_profile *skin_profile = &m2->skin_profiles[0];
	uint16_t *indices = NULL;
	uint32_t indices_nb = 0;
	uint32_t base = 0;
	for (size_t i = 0; i < skin_profile->sections_nb; ++i)
	{
		const struct wow_m2_skin_section *section = &skin_profile->sections[i];
		indices_nb += section->index_count;
		indices = realloc(indices, indices_nb * sizeof(*indices));
		for (size_t j = 0; j < section->index_count; ++j)
			indices[base++] = skin_profile->vertexes[skin_profile->indices[section->index_start + j]];
	}
	char model_name[512];
	snprintf(model_name, sizeof(model_name), "%.*s", (int)strlen(node->name) - 3, node->name);
	json_t *root = json_object();
	json_t *asset = json_object();
	json_t *asset_version = json_string("2.0");
	json_object_set_new(asset, "version", asset_version);
	json_object_set_new(root, "asset", asset);
	json_object_set_new(root, "scene", json_integer(0));
	json_t *scenes = json_array();
	json_t *scene = json_object();
	json_t *scene_nodes = json_array();
	json_array_append_new(scene_nodes, json_integer(0));
	json_object_set_new(scene, "nodes", scene_nodes);
	json_array_append_new(scenes, scene);
	json_object_set_new(root, "scenes", scenes);
	json_t *nodes = json_array();
	{
		json_t *jnode = json_object();
		json_t *children = json_array();
		for (size_t i = 0; i < skin_profile->batches_nb; ++i)
			json_array_append_new(children, json_integer(i + 1));
		json_object_set_new(jnode, "children", children);
		json_object_set_new(jnode, "name", json_string("root"));
		json_array_append_new(nodes, jnode);
	}
	gltf_batches(m2, nodes);
	float *bones_mats = gltf_bones(m2, nodes);
	json_object_set_new(root, "nodes", nodes);
	json_t *meshes = json_array();
	for (size_t i = 0; i < skin_profile->batches_nb; ++i)
	{
		const struct wow_m2_batch *m2_batch = &skin_profile->batches[i];
		json_t *mesh = json_object();
		json_t *mesh_primitives = json_array();
		json_t *mesh_primitive = json_object();
		json_t *primitive_attributes = json_object();
		json_object_set_new(primitive_attributes, "POSITION", json_integer(0));
		json_object_set_new(primitive_attributes, "WEIGHTS_0", json_integer(1));
		json_object_set_new(primitive_attributes, "JOINTS_0", json_integer(2));
		json_object_set_new(primitive_attributes, "NORMAL", json_integer(3));
		json_object_set_new(primitive_attributes, "TEXCOORD_0", json_integer(4));
		json_object_set_new(primitive_attributes, "TEXCOORD_1", json_integer(5));
		json_object_set_new(mesh_primitive, "attributes", primitive_attributes);
		json_object_set_new(mesh_primitive, "indices", json_integer(6 + m2_batch->skin_section_index));
		json_object_set_new(mesh_primitive, "mode", json_integer(4));
		json_object_set_new(mesh_primitive, "material", json_integer(i));
		json_array_append_new(mesh_primitives, mesh_primitive);
		json_object_set_new(mesh, "primitives", mesh_primitives);
		json_array_append_new(meshes, mesh);
	}
	json_object_set_new(root, "meshes", meshes);
	{
		json_t *accessors = json_array();
		json_array_append_new(accessors, json_accessor(0, 0x00, 5126, 0, m2->vertexes_nb, "VEC3"));
		json_array_append_new(accessors, json_accessor(0, 0x0C, 5121, 1, m2->vertexes_nb, "VEC4"));
		json_array_append_new(accessors, json_accessor(0, 0x10, 5121, 0, m2->vertexes_nb, "VEC4"));
		json_array_append_new(accessors, json_accessor(0, 0x14, 5126, 0, m2->vertexes_nb, "VEC3"));
		json_array_append_new(accessors, json_accessor(0, 0x20, 5126, 0, m2->vertexes_nb, "VEC2"));
		json_array_append_new(accessors, json_accessor(0, 0x28, 5126, 0, m2->vertexes_nb, "VEC2"));
		base = 0;
		for (size_t i = 0; i < skin_profile->sections_nb; ++i)
		{
			json_array_append_new(accessors, json_accessor(1, base * sizeof(*indices), 5123, 0, skin_profile->sections[i].index_count, "SCALAR"));
			base += skin_profile->sections[i].index_count;
		}
		json_array_append_new(accessors, json_accessor(2, 0, 5126, 0, m2->bones_nb, "MAT4"));
		if (export_animations)
		{
			int bone_buffer_view = 3;
			for (size_t i = 0; i < m2->sequences_nb; ++i)
			{
				const struct wow_m2_sequence *sequence = &m2->sequences[i];
				for (size_t j = 0; j < m2->bones_nb; ++j)
				{
					const struct wow_m2_bone *bone = &m2->bones[j];
					if (bone->translation.global_sequence == -1 && bone->translation.timestamps_nb)
					{
						size_t start = 0;
						while (start < bone->translation.timestamps_nb
						    && bone->translation.timestamps[start] < sequence->start)
							++start;
						size_t end = start;
						while (end < bone->translation.timestamps_nb
						    && bone->translation.timestamps[end] <= sequence->end)
							++end;
						size_t nb = end - start;
						json_array_append_new(accessors, json_accessor(bone_buffer_view++, 0, 5126, 0, nb, "SCALAR"));
						json_array_append_new(accessors, json_accessor(bone_buffer_view++, 0, 5126, 0, nb, "VEC3"));
					}
					if (bone->rotation.global_sequence == -1 && bone->rotation.timestamps_nb)
					{
						size_t start = 0;
						while (start < bone->rotation.timestamps_nb
						    && bone->rotation.timestamps[start] < sequence->start)
							++start;
						size_t end = start;
						while (end < bone->rotation.timestamps_nb
						    && bone->rotation.timestamps[end] <= sequence->end)
							++end;
						size_t nb = end - start;
						json_array_append_new(accessors, json_accessor(bone_buffer_view++, 0, 5126, 0, nb, "SCALAR"));
						json_array_append_new(accessors, json_accessor(bone_buffer_view++, 0, 5126, 0, nb, "VEC4"));
					}
					if (bone->scale.global_sequence == -1 && bone->scale.timestamps_nb)
					{
						size_t start = 0;
						while (start < bone->scale.timestamps_nb
						    && bone->scale.timestamps[start] < sequence->start)
							++start;
						size_t end = start;
						while (end < bone->scale.timestamps_nb
						    && bone->scale.timestamps[end] <= sequence->end)
							++end;
						size_t nb = end - start;
						json_array_append_new(accessors, json_accessor(bone_buffer_view++, 0, 5126, 0, nb, "SCALAR"));
						json_array_append_new(accessors, json_accessor(bone_buffer_view++, 0, 5126, 0, nb, "VEC3"));
					}
				}
			}
		}
		json_object_set_new(root, "accessors", accessors);
	}
	gltf_buffer_views(m2, root, indices, indices_nb, export_animations);
	gltf_buffers(m2, root, indices, indices_nb, bones_mats, export_animations, directory_path, model_name);
	gltf_materials(m2, root);
	gltf_textures(m2, root, directory_path, model_name);
	gltf_skins(m2, root);
	if (export_animations)
	{
		json_t *animations = json_array();
		int animation_accessor = 6 + skin_profile->sections_nb + 1;
		for (size_t i = 0; i < m2->sequences_nb; ++i)
		{
			const struct wow_m2_sequence *sequence = &m2->sequences[i];
			json_t *animation = json_object();
			json_t *channels = json_array();
			int channel_sampler = 0;
			for (size_t j = 0; j < m2->bones_nb; ++j)
			{
				const struct wow_m2_bone *bone = &m2->bones[j];
				if (bone->translation.global_sequence == -1 && bone->translation.timestamps_nb)
				{
					json_t *channel = json_object();
					json_object_set_new(channel, "sampler", json_integer(channel_sampler++));
					json_t *target = json_object();
					json_object_set(target, "node", json_integer(skin_profile->batches_nb + 1 + j));
					json_object_set_new(target, "path", json_string("translation"));
					json_object_set_new(channel, "target", target);
					json_array_append_new(channels, channel);
				}
				if (bone->rotation.global_sequence == -1 && bone->rotation.timestamps_nb)
				{
					json_t *channel = json_object();
					json_object_set_new(channel, "sampler", json_integer(channel_sampler++));
					json_t *target = json_object();
					json_object_set_new(target, "node", json_integer(skin_profile->batches_nb + 1 + j));
					json_object_set_new(target, "path", json_string("rotation"));
					json_object_set_new(channel, "target", target);
					json_array_append_new(channels, channel);
				}
				if (bone->scale.global_sequence == -1 && bone->scale.timestamps_nb)
				{
					json_t *channel = json_object();
					json_object_set_new(channel, "sampler", json_integer(channel_sampler++));
					json_t *target = json_object();
					json_object_set_new(target, "node", json_integer(skin_profile->batches_nb + 1 + j));
					json_object_set_new(target, "path", json_string("scale"));
					json_object_set_new(channel, "target", target);
					json_array_append_new(channels, channel);
				}
			}
			json_object_set_new(animation, "channels", channels);
			json_t *samplers = json_array();
			for (size_t j = 0; j < m2->bones_nb; ++j)
			{
				const struct wow_m2_bone *bone = &m2->bones[j];
				static const char *interpolations[] =
				{
					"STEP",
					"LINEAR",
					"CUBICSPLINE",
				};
				if (bone->translation.global_sequence == -1 && bone->translation.timestamps_nb)
				{
					json_t *sampler = json_object();
					json_object_set_new(sampler, "input", json_integer(animation_accessor++));
					json_object_set_new(sampler, "output", json_integer(animation_accessor++));
					json_object_set_new(sampler, "interpolation", json_string(interpolations[bone->translation.interpolation_type]));
					json_array_append_new(samplers, sampler);
				}
				if (bone->rotation.global_sequence == -1 && bone->rotation.timestamps_nb)
				{
					json_t *sampler = json_object();
					json_object_set_new(sampler, "input", json_integer(animation_accessor++));
					json_object_set_new(sampler, "output", json_integer(animation_accessor++));
					json_object_set_new(sampler, "interpolation", json_string(interpolations[bone->rotation.interpolation_type]));
					json_array_append_new(samplers, sampler);
				}
				if (bone->scale.global_sequence == -1 && bone->scale.timestamps_nb)
				{
					json_t *sampler = json_object();
					json_object_set_new(sampler, "input", json_integer(animation_accessor++));
					json_object_set_new(sampler, "output", json_integer(animation_accessor++));
					json_object_set_new(sampler, "interpolation", json_string(interpolations[bone->scale.interpolation_type]));
					json_array_append_new(samplers, sampler);
				}
			}
			json_object_set_new(animation, "samplers", samplers);
			char name[256];
			if (sequence->variation_index)
				snprintf(name, sizeof(name), "%s_%d", animation_names[sequence->id], (int)sequence->variation_index);
			else
				snprintf(name, sizeof(name), "%s", animation_names[sequence->id]);
			json_object_set_new(animation, "name", json_string(name));
			json_array_append_new(animations, animation);
		}
		json_object_set_new(root, "animations", animations);
	}
	snprintf(tmp, sizeof(tmp), "%s/%s.gltf", directory_path, model_name);
	json_dump_file(root, tmp, JSON_INDENT(2));
	json_decref(root);
	fprintf(stderr, "export ended\n");

end:
	wow_m2_file_delete(m2);
	if (directory)
		g_object_unref(directory);
	gtk_widget_hide(GTK_WIDGET(dialog));
}

static void gltf_m2(struct node *node)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new("destination directory", GTK_WINDOW(g_explorer->window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
	gtk_file_chooser_set_create_folders(chooser, TRUE);
	gtk_file_chooser_add_choice(chooser, "export_animations", "Export animations", NULL, NULL);
	g_signal_connect(dialog, "response", G_CALLBACK(on_export_response), node);
	gtk_widget_show(dialog);
}

static void node_export_gltf(GtkWidget *widget, gpointer data)
{
	struct node *node = data;
	gtk_popover_popdown(GTK_POPOVER(gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(widget)))));
	if (!strcmp(&node->name[strlen(node->name) - 3], ".m2"))
		gltf_m2(node);
}

static void on_gtk_row_button_pressed(GtkGestureClick *gesture, int n_press, double x, double y, GtkWidget *widget)
{
	(void)n_press;
	gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_CLAIMED);
	GtkTreeModel *treemodel = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if (!treemodel)
		return;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	if (gtk_tree_selection_count_selected_rows(selection) > 1)
		return;
	GtkTreePath *path;
	if (!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), x, y, &path, NULL, NULL, NULL))
		return;
	gtk_tree_selection_unselect_all(selection);
	gtk_tree_selection_select_path(selection, path);
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter(treemodel, &iter, path))
	{
		gtk_tree_path_free(path);
		return;
	}
	struct node *node;
	gtk_tree_model_get(treemodel, &iter, 1, &node, -1);
	GtkWidget *menu = gtk_popover_new();
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *item = gtk_button_new_with_label("copy path");
	g_signal_connect(item, "clicked", G_CALLBACK(copy_path), node);
	gtk_widget_add_css_class(item, "flat");
	gtk_widget_set_halign(gtk_button_get_child(GTK_BUTTON(item)), GTK_ALIGN_START);
	gtk_box_append(GTK_BOX(box), item);
	gtk_widget_show(item);
	item = gtk_button_new_with_label("extract");
	g_signal_connect(item, "clicked", G_CALLBACK(node_extract), node);
	gtk_widget_add_css_class(item, "flat");
	gtk_widget_set_halign(gtk_button_get_child(GTK_BUTTON(item)), GTK_ALIGN_START);
	gtk_box_append(GTK_BOX(box), item);
	gtk_widget_show(item);
	item = gtk_button_new_with_label("glTF export");
	g_signal_connect(item, "clicked", G_CALLBACK(node_export_gltf), node);
	gtk_widget_add_css_class(item, "flat");
	gtk_widget_set_halign(gtk_button_get_child(GTK_BUTTON(item)), GTK_ALIGN_START);
	gtk_box_append(GTK_BOX(box), item);
	gtk_widget_show(item);
	gtk_popover_set_child(GTK_POPOVER(menu), box);
	GdkRectangle rect;
	rect.x = x;
	rect.y = y;
	rect.width = 1;
	rect.height = 1;
	gtk_popover_set_pointing_to(GTK_POPOVER(menu), &rect);
	gtk_widget_set_parent(menu, g_explorer->paned);
	gtk_popover_popup(GTK_POPOVER(menu));
	gtk_widget_show(menu);
}

static void add_child(struct tree *tree, GtkTreeIter *parent, struct node *node)
{
	GtkTreeIter iter;
	gtk_tree_store_append(tree->store, &iter, parent);
	gtk_tree_store_set(tree->store, &iter, 0, node->name, 1, node, -1);
	struct node *child;
	TAILQ_FOREACH(child, &node->childs, chain)
		add_child(tree, &iter, child);
}
