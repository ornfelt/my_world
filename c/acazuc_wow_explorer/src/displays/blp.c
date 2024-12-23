#include "display.h"

#include <wow/blp.h>

#include <inttypes.h>

struct blp_display
{
	struct display display;
	struct wow_blp_file *file;
	GtkWidget *gtk_display;
	GtkWidget *image;
};

static void on_gtk_mipmap_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data);
static void set_mipmap_box(struct blp_display *display, uint32_t mipmap_id);
static GtkWidget *build_gtk_paned(struct blp_display *display);

static void dummy_free(unsigned char *ptr, void *osef)
{
	(void)osef;
	free(ptr);
}

static void dtr(struct display *ptr)
{
	struct blp_display *display = (struct blp_display*)ptr;
	wow_blp_file_delete(display->file);
}

struct display *blp_display_new(const struct node *node, const char *path, struct wow_mpq_file *mpq_file)
{
	(void)node;
	(void)path;
	struct wow_blp_file *file = wow_blp_file_new(mpq_file);
	if (!file)
	{
		fprintf(stderr, "failed to parse blp file\n");
		return NULL;
	}
	struct blp_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "blp display allocation failed\n");
		wow_blp_file_delete(file);
		return NULL;
	}
	display->display.name = strdup(path);
	display->display.dtr = dtr;
	display->file = file;
	display->image = NULL;
	display->display.root = build_gtk_paned(display);
	set_mipmap_box(display, 0);
	return &display->display;
}

static GtkWidget *build_gtk_tree(struct blp_display *display)
{
	GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("mipmaps", renderer, "text", 0, NULL);
	g_signal_connect(tree, "row-activated", G_CALLBACK(on_gtk_mipmap_row_activated), display);
	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(tree), true);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	GtkTreeIter iter;
	for (uint32_t i = 0; i < display->file->mipmaps_nb; ++i)
	{
		char name[64];
		snprintf(name, sizeof(name), "mipmaps %" PRId32, i);
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, name, 1, i, -1);
	}
	gtk_widget_show(tree);
	GtkTreePath *path = gtk_tree_path_new_from_indices(0, -1);
	gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(tree)), path);
	gtk_tree_path_free(path);
	return tree;
}

static GtkWidget *build_gtk_left_paned(struct blp_display *display)
{
	GtkWidget *tree = build_gtk_tree(display);
	GtkWidget *left_paned = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(left_paned), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_vexpand(left_paned, true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(left_paned), tree);
	gtk_widget_show(left_paned);
	return left_paned;
}

static GtkWidget *build_gtk_right_paned(struct blp_display *display)
{
	display->gtk_display = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_show(display->gtk_display);
	GtkWidget *right_paned = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(right_paned), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(right_paned), display->gtk_display);
	gtk_widget_show(right_paned);
	return right_paned;
}

static GtkWidget *build_gtk_top_paned(struct blp_display *display)
{
	GtkWidget *left_paned = build_gtk_left_paned(display);
	GtkWidget *right_paned = build_gtk_right_paned(display);
	GtkWidget *top_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_start_child(GTK_PANED(top_paned), left_paned);
	gtk_paned_set_end_child(GTK_PANED(top_paned), right_paned);
	gtk_widget_set_vexpand(top_paned, true);
	gtk_paned_set_position(GTK_PANED(top_paned), 200);
	gtk_widget_show(top_paned);
	return top_paned;
}

static GtkWidget *build_gtk_details(struct blp_display *display)
{
	GtkListStore *store = gtk_list_store_new(6, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);
	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column;
	column = gtk_tree_view_column_new_with_attributes("width", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("height", renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("type", renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("compression", renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("alpha depth", renderer, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("alpha type", renderer, "text", 5, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	{
		unsigned int width = display->file->header.width;
		unsigned int height = display->file->header.height;
		unsigned int type = display->file->header.type;
		unsigned int compression = display->file->header.compression;
		unsigned int alpha_depth = display->file->header.alpha_depth;
		unsigned int alpha_type = display->file->header.alpha_type;
		gtk_list_store_set(store, &iter, 0, width, 1, height, 2, type, 3, compression, 4, alpha_depth, 5, alpha_type, -1);
	}
	gtk_widget_show(tree);
	return tree;
}

static GtkWidget *build_gtk_bot_paned(struct blp_display *display)
{
	GtkWidget *details = build_gtk_details(display);
	GtkWidget *bot_paned = gtk_scrolled_window_new();
	gtk_widget_set_size_request(bot_paned, 0, 200);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(bot_paned), details);
	gtk_widget_show(bot_paned);
	return bot_paned;
}

static GtkWidget *build_gtk_paned(struct blp_display *display)
{
	GtkWidget *top_paned = build_gtk_top_paned(display);
	GtkWidget *bot_paned = build_gtk_bot_paned(display);
	GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	gtk_paned_set_start_child(GTK_PANED(paned), top_paned);
	gtk_paned_set_end_child(GTK_PANED(paned), bot_paned);
	gtk_paned_set_position(GTK_PANED(paned), 500);
	gtk_widget_show(paned);
	return paned;
}

static void on_gtk_mipmap_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data)
{
	struct blp_display *display = data;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(tree);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
	uint32_t val;
	gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 1, &val, -1);
	set_mipmap_box(display, val);
}

static void set_mipmap_box(struct blp_display *display, uint32_t mipmap_id)
{
	uint8_t *data;
	uint32_t width;
	uint32_t height;
	uint32_t line_width;
	if (!wow_blp_decode_rgba(display->file, mipmap_id, &width, &height, &data))
		return;
	if (width < 4 && display->file->header.type == 1 && display->file->header.compression == 2)
		line_width = 16;
	else
		line_width = width * 4;
	if (display->image)
		gtk_box_remove(GTK_BOX(display->gtk_display), display->image);
	display->image = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, true, 8, width, height, line_width, dummy_free, NULL));
	gtk_widget_set_vexpand(display->image, true);
	gtk_widget_set_hexpand(display->image, true);
	gtk_widget_show(display->image);
	gtk_box_append(GTK_BOX(display->gtk_display), display->image);
}
