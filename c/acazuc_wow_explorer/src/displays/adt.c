#include "../explorer.h"

#include "table_macro.h"
#include "display.h"

#include <wow/adt.h>

#include <inttypes.h>
#include <stdbool.h>

struct adt_display
{
	struct display display;
	struct wow_adt_file *file;
};

static void on_gtk_block_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data);

static void dummy_free(unsigned char *ptr, void *osef)
{
	(void)osef;
	free(ptr);
}

static void dtr(struct display *ptr)
{
	struct adt_display *display = (struct adt_display*)ptr;
	wow_adt_file_delete(display->file);
}

struct display *adt_display_new(const struct node *node, const char *path, struct wow_mpq_file *mpq_file)
{
	(void)node;
	(void)path;
	/*
	   Split between left treeview and display
	   Treeview is:
	   > MCNK
	    > 1
	     > Layers
	       1
	       2
	       3
	       Texture Mapping
	       Shadow Mapping
	       HeightMap
	       NormalMap
	       Holes
	       Liquids
	     > 2
	       ...
	     MTEX
	     MWMO
	     MWID
	     MODF
	     MMDX
	     MMID
	     MDDF
	     MFBO
	     Textures
	     Next is everything available in MCNK but generalized over all the chunks (textures, height, ...)
	 */
	struct wow_adt_file *file = wow_adt_file_new(mpq_file);
	if (!file)
	{
		fprintf(stderr, "failed to parse adt file\n");
		return NULL;
	}
	struct adt_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "adt display allocation failed\n");
		wow_adt_file_delete(file);
		return NULL;
	}
	display->display.name = strdup(path);
	display->display.dtr = dtr;
	display->file = file;
	/* Tree */
	GtkTreeStore *store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	GtkWidget *tree = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), false);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	g_signal_connect(tree, "row-activated", G_CALLBACK(on_gtk_block_row_activated), display);
	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(tree), true);
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Blocks", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	gtk_widget_set_vexpand(tree, true);
	GtkTreeIter layer_iter;
	GtkTreeIter mcnk_iter;
	GtkTreeIter iter;
	{
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, "texture mapping", 1, 0x1000001, -1);
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, "shadow mapping", 1, 0x1000002, -1);
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, "height map", 1, 0x1000003, -1);
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, "normal map", 1, 0x1000004, -1);
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, "holes", 1, 0x1000005, -1);
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, "liquids", 1, 0x1000006, -1);
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, "objects", 1, 0x1000007, -1);
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, "wmo", 1, 0x1000008, -1);
	}
	for (uint32_t i = 0; i < 256; ++i)
	{
		char name[64];
		snprintf(name, sizeof(name), "MCNK_%" PRIu32 "_%" PRIu32, i / 16, i % 16);
		gtk_tree_store_append(store, &mcnk_iter, NULL);
		gtk_tree_store_set(store, &mcnk_iter, 0, name, 1, 0, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "layer 1", 1, (i << 16) | 1, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "layer 2", 1, (i << 16) | 2, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "layer 3", 1, (i << 16) | 3, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "layer 4", 1, (i << 16) | 4, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "texture mapping", 1, (i << 16) | 5, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "shadow mapping", 1, (i << 16) | 6, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "height map", 1, (i << 16) | 7, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "normal map", 1, (i << 16) | 8, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "holes", 1, (i << 16) | 9, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "liquids", 1, (i << 16) | 10, -1);
		gtk_tree_store_append(store, &iter, &mcnk_iter);
		gtk_tree_store_set(store, &iter, 0, "objects", 1, (i << 16) | 11, -1);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store));
	gtk_widget_show(tree);
	/* Scrolled */
	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_vexpand(scrolled, true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), tree);
	gtk_widget_show(scrolled);
	/* Paned */
	GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_start_child(GTK_PANED(paned), scrolled);
	gtk_paned_set_position(GTK_PANED(paned), 200);
	gtk_widget_show(paned);
	display->display.root = paned;
	return &display->display;
}

static GtkWidget *build_mcnk_layer(struct adt_display *display, uint8_t mcnk_id, uint8_t what)
{
	struct wow_mcnk *mcnk = &display->file->mcnk[mcnk_id];
	struct wow_mcly_data *layer = &display->file->mcnk[mcnk_id].mcly.data[what - 1];
	if (what > display->file->mcnk[mcnk_id].mcly.size)
		return NULL;
	size_t scale = 5;
	size_t width = 64 * scale;
	size_t height = 64 * scale;
	uint8_t *data = malloc(width * height * 3);
	memset(data, 0, width * height * 3);
	if (layer->flags.use_alpha_map)
	{
		size_t i = what - 1;
		for (size_t y = 0; y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				uint8_t tmp = mcnk->mcal.data[layer->offset_in_mcal + ((y / scale * 64 + x / scale) / 2)];
				if ((x / scale) & 1)
				{
					tmp &= 0xF0;
					tmp |= tmp >> 4;
				}
				else
				{
					tmp &= 0xF;
					tmp |= tmp << 4;
				}
				data[i] = tmp;
				i += 3;
			}
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	GtkWidget *top_scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(top_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(top_scrolled), image);
	gtk_widget_show(top_scrolled);
	GtkListStore *store = gtk_list_store_new(6, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);
	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column;
	column = gtk_tree_view_column_new_with_attributes("texture_id", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("mcal_offset", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("effect", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("adnimation_rotation", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("animation_speed", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("animation_enabled", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("overbright", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("use_alpha_map", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("alpha_map_compressed", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	gtk_widget_show(tree);
	GtkWidget *bot_scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(bot_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(bot_scrolled), tree);
	gtk_widget_show(bot_scrolled);
	GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	gtk_paned_set_start_child(GTK_PANED(paned), top_scrolled);
	gtk_paned_set_end_child(GTK_PANED(paned), bot_scrolled);
	gtk_paned_set_position(GTK_PANED(paned), 500);
	gtk_widget_show(paned);
	return paned;
}

static GtkWidget *build_mcnk_texture(struct adt_display *display, uint8_t mcnk_id)
{
	struct wow_mcnk *mcnk = &display->file->mcnk[mcnk_id];
	size_t scale = 5;
	size_t width = 64 * scale;
	size_t height = 64 * scale;
	uint8_t *data = malloc(width * height * 3);
	memset(data, 0, width * height * 3);
	for (size_t l = 1; l < mcnk->header.layers; ++l)
	{
		if (!mcnk->mcly.data[l].flags.use_alpha_map)
			continue;
		size_t i = l;
		for (size_t y = 0; y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				uint8_t tmp = mcnk->mcal.data[mcnk->mcly.data[l].offset_in_mcal + ((y / scale * 64 + x / scale) / 2)];
				if ((x / scale) & 1)
				{
					tmp &= 0xF0;
					tmp |= tmp >> 4;
				}
				else
				{
					tmp &= 0xF;
					tmp |= tmp << 4;
				}
				data[i] = tmp;
				i += 3;
			}
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	return image;
}

static GtkWidget *build_mcnk_shadow(struct adt_display *display, uint8_t mcnkId)
{
	struct wow_mcnk *mcnk = &display->file->mcnk[mcnkId];
	struct wow_mcsh *mcsh = &mcnk->mcsh;
	size_t scale = 5;
	size_t width = 64 * scale;
	size_t height = 64 * scale;
	uint8_t *data = malloc(width * height * 3);
	memset(data, 0, width * height * 3);
	if (mcnk->header.flags & WOW_MCNK_FLAGS_MCSH)
	{
		size_t i = 0;
		for (size_t y = 0; y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				uint8_t tmp = (mcsh->data[y / scale][x / scale / 8] & (1 << ((x / scale) % 8))) ? 0xff : 0;
				data[i++] = tmp;
				data[i++] = tmp;
				data[i++] = tmp;
			}
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	return image;
}

static GtkWidget *build_mcnk_height(struct adt_display *display, uint8_t mcnk_id)
{
	struct wow_mcnk *mcnk = &display->file->mcnk[mcnk_id];
	struct wow_mcvt *mcvt = &mcnk->mcvt;
	size_t scale = 5;
	size_t width = 9 * scale;
	size_t height = 9 * scale;
	uint8_t *data = malloc(width * height * 3);
	memset(data, 0, width * height * 3);
	size_t i = 0;
	float min = +999999;
	float max = -999999;
	for (size_t y = 0; y < height; ++y)
	{
		for (size_t x = 0; x < width; ++x)
		{
			size_t n = y / scale * 17 + x / scale;
			float h = mcvt->data[n] + mcnk->header.position.z;
			if (h < min)
				min = h;
			if (h > max)
				max = h;
		}
	}
	for (size_t y = 0; y < height; ++y)
	{
		for (size_t x = 0; x < width; ++x)
		{
			size_t n = y / scale * 17 + x / scale;
			float h = mcvt->data[n] + mcnk->header.position.z;
			uint32_t color = get_color_from_height(h, min, max);
			data[i++] = color >> 16;
			data[i++] = color >> 8;
			data[i++] = color >> 0;
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	return image;
}

static GtkWidget *build_mcnk_normal(struct adt_display *display, uint8_t mcnk_id)
{
	struct wow_mcnk *mcnk = &display->file->mcnk[mcnk_id];
	struct wow_mcnr *mcnr = &mcnk->mcnr;
	size_t scale = 5;
	size_t width = 9 * scale;
	size_t height = 9 * scale;
	uint8_t *data = malloc(width * height * 3);
	memset(data, 0, width * height * 3);
	size_t i = 0;
	for (size_t y = 0; y < height; ++y)
	{
		for (size_t x = 0; x < width; ++x)
		{
			size_t n = (y / scale * 17 + x / scale) * 3;
			data[i++] = mcnr->data[n + 0] + 0x7F;
			data[i++] = mcnr->data[n + 2] + 0x7F;
			data[i++] = -mcnr->data[n + 1] + 0x7F;
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	return image;
}

static GtkWidget *build_mcnk_holes(struct adt_display *display, uint8_t mcnk_id)
{
	struct wow_mcnk *mcnk = &display->file->mcnk[mcnk_id];
	uint8_t holes = mcnk->header.holes;
	return NULL;
}

static GtkWidget *build_mcnk_liquids(struct adt_display *display, uint8_t mcnk_id)
{
	return NULL;
}

static GtkWidget *build_mcnk_objects(struct adt_display *display, uint8_t mcnk_id)
{
	return NULL;
}

static GtkWidget *build_mcnk(struct adt_display *display, uint8_t mcnk_id, uint8_t what)
{
	GtkWidget *widget = NULL;
	if (what & 0xFF00)
	{
		widget = build_mcnk_layer(display, mcnk_id, what & 0xff);
	}
	else
	{
		switch (what)
		{
			case 1:
			case 2:
			case 3:
			case 4:
				widget = build_mcnk_layer(display, mcnk_id, what);
				break;
			case 5:
				widget = build_mcnk_texture(display, mcnk_id);
				break;
			case 6:
				widget = build_mcnk_shadow(display, mcnk_id);
				break;
			case 7:
				widget = build_mcnk_height(display, mcnk_id);
				break;
			case 8:
				widget = build_mcnk_normal(display, mcnk_id);
				break;
			case 9:
				widget = build_mcnk_holes(display, mcnk_id);
				break;
			case 10:
				widget = build_mcnk_liquids(display, mcnk_id);
				break;
			case 11:
				widget = build_mcnk_objects(display, mcnk_id);
				break;
		}
	}
	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_vexpand(scrolled, true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), widget);
	gtk_widget_show(scrolled);
	return scrolled;
}

static GtkWidget *build_adt_texture(struct adt_display *display)
{
	size_t scale = 1;
	size_t width = 16 * 64 * scale;
	size_t height = 16 * 64 * scale;
	uint8_t *data = malloc(width * height * 3);
	memset(data, 0, width * height * 3);
	for (size_t cy = 0; cy < 16; ++cy)
	{
		for (size_t cx = 0; cx < 16; ++cx)
		{
			struct wow_mcnk *mcnk = &display->file->mcnk[cx * 16 + cy];
			for (size_t l = 1; l < mcnk->header.layers; ++l)
			{
				if (!mcnk->mcly.data[l].flags.use_alpha_map)
					continue;
				for (size_t y = 0; y < height / 16; ++y)
				{
					for (size_t x = 0; x < width / 16; ++x)
					{
						uint8_t tmp = mcnk->mcal.data[mcnk->mcly.data[l].offset_in_mcal + ((y / scale * 64 + x / scale) / 2)];
						if ((x / scale) & 1)
						{
							tmp &= 0xF0;
							tmp |= tmp >> 4;
						}
						else
						{
							tmp &= 0xF;
							tmp |= tmp << 4;
						}
						size_t i = ((cx * 64 * scale + y) * width + (cy * 64 * scale + x)) * 3 + l;
						data[i] = tmp;
						i += 3;
					}
				}
			}
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	return image;
}

static GtkWidget *build_adt_shadow(struct adt_display *display)
{
	size_t scale = 1;
	size_t width = 16 * 64 * scale;
	size_t height = 16 * 64 * scale;
	uint8_t *data = malloc(width * height * 3);
	for (size_t cy = 0; cy < 16; ++cy)
	{
		for (size_t cx = 0; cx < 16; ++cx)
		{
			struct wow_mcnk *mcnk = &display->file->mcnk[cx * 16 + cy];
			if (mcnk->header.flags & WOW_MCNK_FLAGS_MCSH)
			{
				struct wow_mcsh *mcsh = &mcnk->mcsh;
				for (size_t y = 0; y < height / 16; ++y)
				{
					for (size_t x = 0; x < width / 16; ++x)
					{
						uint8_t tmp = (mcsh->data[y / scale][x / scale / 8] & (1 << ((x / scale) % 8))) ? 0 : 0xff;
						size_t i = ((cx * 64 * scale + y) * width + (cy * 64 * scale + x)) * 3;
						data[i++] = tmp;
						data[i++] = tmp;
						data[i++] = tmp;
					}
				}
			}
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	return image;
}

static GtkWidget *build_adt_height(struct adt_display *display)
{
	size_t scale = 5;
	size_t width = 16 * 9 * scale;
	size_t height = 16 * 9 * scale;
	uint8_t *data = malloc(width * height * 3);
	float min = +999999;
	float max = -999999;
	for (size_t cy = 0; cy < 16; ++cy)
	{
		for (size_t cx = 0; cx < 16; ++cx)
		{
			struct wow_mcnk *mcnk = &display->file->mcnk[cx * 16 + cy];
			struct wow_mcvt *mcvt = &mcnk->mcvt;
			for (size_t y = 0; y < height / 16; ++y)
			{
				for (size_t x = 0; x < width / 16; ++x)
				{
					size_t n = y / scale * 17 + x / scale;
					float h = mcvt->data[n] + mcnk->header.position.z;
					if (h < min)
						min = h;
					if (h > max)
						max = h;
				}
			}
		}
	}
	for (size_t cy = 0; cy < 16; ++cy)
	{
		for (size_t cx = 0; cx < 16; ++cx)
		{
			struct wow_mcnk *mcnk = &display->file->mcnk[cx * 16 + cy];
			struct wow_mcvt *mcvt = &mcnk->mcvt;
			for (size_t y = 0; y < height / 16; ++y)
			{
				for (size_t x = 0; x < width / 16; ++x)
				{
					size_t i = ((cx * 9 * scale + y) * width + (cy * 9 * scale + x)) * 3;
					size_t n = y / scale * 17 + x / scale;
					float h = mcvt->data[n] + mcnk->header.position.z;
					uint32_t color = get_color_from_height(h, min, max);
					data[i++] = color >> 16;
					data[i++] = color >> 8;
					data[i++] = color >> 0;
				}
			}
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	return image;
}

static GtkWidget *build_adt_normal(struct adt_display *display)
{
	size_t scale = 5;
	size_t width = 16 * 9 * scale;
	size_t height = 16 * 9 * scale;
	uint8_t *data = malloc(width * height * 3);
	for (size_t cy = 0; cy < 16; ++cy)
	{
		for (size_t cx = 0; cx < 16; ++cx)
		{
			struct wow_mcnk *mcnk = &display->file->mcnk[cx * 16 + cy];
			struct wow_mcnr *mcnr = &mcnk->mcnr;
			for (size_t y = 0; y < height / 16; ++y)
			{
				for (size_t x = 0; x < width / 16; ++x)
				{
					size_t i = ((cx * 9 * scale + y) * width + (cy * 9 * scale + x)) * 3;
					size_t n = (y / scale * 17 + x / scale) * 3;
					data[i++] = mcnr->data[n + 0] + 0x7F;
					data[i++] = mcnr->data[n + 2] + 0x7F;
					data[i++] = -mcnr->data[n + 1] + 0x7F;
				}
			}
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	return image;
}

static GtkWidget *build_adt_objects(struct adt_display *display)
{
	TREE_BEGIN(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "name");
	ADD_TREE_COLUMN(1, "id");
	ADD_TREE_COLUMN(2, "position");
	ADD_TREE_COLUMN(3, "rotation");
	ADD_TREE_COLUMN(4, "scale");
	ADD_TREE_COLUMN(5, "flags");
	for (size_t i = 0; i < display->file->mddf.size; ++i)
	{
		struct wow_mddf_data *mddf = &display->file->mddf.data[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_FMT(0, "%s", &display->file->mmdx.data[display->file->mmid.data[mddf->name_id]]);
		SET_TREE_VALUE_FMT(1, "%" PRIu32, mddf->unique_id);
		SET_TREE_VALUE_FMT(2, "{%f, %f, %f}", mddf->position.x, mddf->position.y, mddf->position.z);
		SET_TREE_VALUE_FMT(3, "{%f, %f, %f}", mddf->rotation.x, mddf->rotation.y, mddf->rotation.z);
		SET_TREE_VALUE_FMT(4, "%f", mddf->scale / 1024.f);
		SET_TREE_VALUE_FMT(5, "0x%" PRIx16, mddf->flags);
	}
	TREE_END();
}

static GtkWidget *build_adt_wmo(struct adt_display *display)
{
	TREE_BEGIN(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	ADD_TREE_COLUMN(0, "name");
	ADD_TREE_COLUMN(1, "id");
	ADD_TREE_COLUMN(2, "position");
	ADD_TREE_COLUMN(3, "rotation");
	ADD_TREE_COLUMN(4, "aabb");
	ADD_TREE_COLUMN(5, "flags");
	ADD_TREE_COLUMN(6, "doodad_set");
	ADD_TREE_COLUMN(7, "name_set");
	for (size_t i = 0; i < display->file->modf.size; ++i)
	{
		struct wow_modf_data *modf = &display->file->modf.data[i];
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		SET_TREE_VALUE_FMT(0, "%s", &display->file->mwmo.data[display->file->mwid.data[modf->name_id]]);
		SET_TREE_VALUE_FMT(1, "%" PRIu32, modf->unique_id);
		SET_TREE_VALUE_FMT(2, "{%f, %f, %f}", modf->position.x, modf->position.y, modf->position.z);
		SET_TREE_VALUE_FMT(3, "{%f, %f, %f}", modf->rotation.x, modf->rotation.y, modf->rotation.z);
		SET_TREE_VALUE_FMT(4, "{%f, %f, %f}, {%f, %f, %f}", modf->aabb0.x, modf->aabb0.y, modf->aabb0.z, modf->aabb1.x, modf->aabb1.y, modf->aabb1.z);
		SET_TREE_VALUE_FMT(5, "0x%" PRIx16, modf->flags);
		SET_TREE_VALUE_FMT(6, "%" PRIu16, modf->doodad_set);
		SET_TREE_VALUE_FMT(7, "%" PRIu16, modf->name_set);
	}
	TREE_END();
}

static GtkWidget *build_adt(struct adt_display *display, int what)
{
	GtkWidget *widget = NULL;
	switch (what)
	{
		case 1:
			widget = build_adt_texture(display);
			break;
		case 2:
			widget = build_adt_shadow(display);
			break;
		case 3:
			widget = build_adt_height(display);
			break;
		case 4:
			widget = build_adt_normal(display);
			break;
		case 7:
			widget = build_adt_objects(display);
			break;
		case 8:
			widget = build_adt_wmo(display);
			break;
	}
	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_vexpand(scrolled, true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), widget);
	gtk_widget_show(scrolled);
	return scrolled;
}

static void on_gtk_block_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data)
{
	struct adt_display *display = data;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(tree);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
	uint32_t val;
	gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 1, &val, -1);
	if (!val)
		return;
	GtkWidget *child;
	if ((val >> 16) == 0x100)
		child = build_adt(display, val & 0xFFFF);
	else
		child = build_mcnk(display, val >> 16, val & 0xFFFF);
	gtk_paned_set_end_child(GTK_PANED(display->display.root), child);
}
