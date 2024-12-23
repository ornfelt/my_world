#include "display.h"

#include <wow/wdt.h>

#include <stdbool.h>

struct wdt_display
{
	struct display display;
};

static void dummy_free(unsigned char *ptr, void *osef)
{
	(void)osef;
	free((uint8_t*)ptr);
}

static void dtr(struct display *ptr)
{
	(void)ptr;
}

struct display *wdt_display_new(const struct node *node, const char *path, struct wow_mpq_file *mpq_file)
{
	(void)node;
	(void)path;
	struct wow_wdt_file *file = wow_wdt_file_new(mpq_file);
	if (!file)
	{
		fprintf(stderr, "failed to parse wdt file\n");
		return NULL;
	}
	struct wdt_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "wdt display allocation failed\n");
		wow_wdt_file_delete(file);
		return NULL;
	}
	display->display.name = strdup(path);
	display->display.dtr = dtr;
	/*GType types[64];
	fpr (soze_t i = 0; i < sizeof(types) / sizeof(*types); ++i)
		types[i] = G_TYPE_BOOLEAN;
	GtkListStore *store = gtk_list_store_newv(sizeof(types) / sizeof(*types), types);
	GValue value = G_VALUE_INIT;
	g_value_init(&value, G_TYPE_BOOLEAN);
	for (size_t y = 0; y < 64; ++y)
	{
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		for (size_t x = 0; x < 64; ++x)
		{
			g_value_set_boolean(&value, (file->main.data[y * 64 + x].flags & WOW_MAIN_FLAG_ADT) ? TRUE : FALSE);
			gtk_list_store_set_value(store, &iter, x, &value);
		}
	}
	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	for (size_t i = 0; i < types.size(); ++i)
	{
		GtkCellRenderer *renderer = gtk_cell_renderer_toggle_new();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("test", renderer, "active", i, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	}
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true);
	gtk_widget_show(tree);*/
	size_t scale = 5;
	size_t width = 64 * scale;
	size_t height = 64 * scale;
	uint8_t *data = malloc(width * height * 3);
	if (!data)
	{
		fprintf(stderr, "wdt data allocation failed\n");
		wow_wdt_file_delete(file);
		free(display);
		return NULL;
	}
	for (size_t y = 0; y < height; y += scale)
	{
		for (size_t x = 0; x < width; x += scale)
		{
			uint8_t val = file->main.data[y / scale * 64 + x / scale].flags & WOW_MAIN_FLAG_ADT ? 0xff : 0;
			for (size_t yy = 0; yy < scale; ++yy)
			{
				for (size_t xx = 0; xx < scale; ++xx)
				{
					size_t i = ((yy + y) * width + xx + x) * 3;
					data[i + 0] = val;
					data[i + 1] = val;
					data[i + 2] = val;
				}
			}
		}
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, false, 8, width, height, width * 3, dummy_free, NULL));
	gtk_widget_show(image);
	display->display.root = image;
	wow_wdt_file_delete(file);
	return &display->display;
}
