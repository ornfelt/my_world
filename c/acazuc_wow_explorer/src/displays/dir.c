#include "../explorer.h"
#include "../nodes.h"

#include "display.h"

#include <wow/mpq.h>

#define ADD_TREE_COLUMN(id, name) \
do \
{ \
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(name, renderer, "text", id, NULL); \
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column); \
	gtk_tree_view_column_set_sort_column_id(column, id); \
	gtk_tree_view_column_set_resizable(column, true); \
} while (0)

struct dir_display
{
	struct display display;
};

static void pretty_size(char *str, size_t len, uint32_t size)
{
	if (size > 1000000000)
		snprintf(str, len, "%3.3f GB", size / 1000000000.f);
	else if (size > 1000000)
		snprintf(str, len, "%3.3f MB", size / 1000000.f);
	else if (size > 1000)
		snprintf(str, len, "%3.3f KB", size / 1000.f);
	else
		snprintf(str, len, "%3u B", size);
}

static void dtr(struct display *ptr)
{
	(void)ptr;
}

struct display *dir_display_new(const struct node *node, const char *path, struct wow_mpq_file *file)
{
	(void)path;
	(void)file;
	struct dir_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "dir display allocation failed\n");
		return NULL;
	}
	display->display.name = strdup(path);
	display->display.dtr = dtr;
	GtkListStore *store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	GtkWidget *tree = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	ADD_TREE_COLUMN(0, "name");
	ADD_TREE_COLUMN(1, "offset");
	ADD_TREE_COLUMN(2, "block size");
	ADD_TREE_COLUMN(3, "file size");
	ADD_TREE_COLUMN(4, "compression");
	ADD_TREE_COLUMN(5, "flags");
	struct node *child;
	TAILQ_FOREACH(child, &node->childs, chain)
	{
		char child_path[512];
		char tmp[64];
		node_get_path(child, child_path, sizeof(child_path));
		wow_mpq_normalize_mpq_fn(child_path, sizeof(child_path));
		const struct wow_mpq_block *block = wow_mpq_get_block(g_explorer->mpq_compound, child_path);
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		GValue value = G_VALUE_INIT;
		g_value_init(&value, G_TYPE_STRING);
		g_value_set_string(&value, child->name);
		gtk_list_store_set_value(store, &iter, 0, &value);
		if (!TAILQ_EMPTY(&child->childs))
		{
			g_value_set_string(&value, "N/A");
		}
		else if (block)
		{
			pretty_size(tmp, sizeof(tmp), block->offset);
			g_value_set_string(&value, tmp);
		}
		else
		{
			g_value_set_string(&value, "err");
		}
		gtk_list_store_set_value(store, &iter, 1, &value);
		if (!TAILQ_EMPTY(&child->childs))
		{
			g_value_set_string(&value, "N/A");
		}
		else if (block)
		{
			pretty_size(tmp, sizeof(tmp), block->block_size);
			g_value_set_string(&value, tmp);
		}
		else
		{
			g_value_set_string(&value, "err");
		}
		gtk_list_store_set_value(store, &iter, 2, &value);
		if (!TAILQ_EMPTY(&child->childs))
		{
			g_value_set_string(&value, "N/A");
		}
		else if (block)
		{
			pretty_size(tmp, sizeof(tmp), block->file_size);
			g_value_set_string(&value, tmp);
		}
		else
		{
			g_value_set_string(&value, "err");
		}
		gtk_list_store_set_value(store, &iter, 3, &value);
		if (!TAILQ_EMPTY(&child->childs))
		{
			g_value_set_string(&value, "N/A");
		}
		else if (block)
		{
			snprintf(tmp, sizeof(tmp), "%2.2f%%", (float)block->block_size / block->file_size * 100);
			g_value_set_string(&value, tmp);
		}
		else
		{
			g_value_set_string(&value, "err");
		}
		gtk_list_store_set_value(store, &iter, 4, &value);
		if (!TAILQ_EMPTY(&child->childs))
		{
			g_value_set_string(&value, "D----------");
		}
		else if (block)
		{
			char flags[11];
			flags[0] = 'R';
			flags[1] = (block->flags & WOW_MPQ_BLOCK_IMPLODE) ? 'I' : '-';
			flags[2] = (block->flags & WOW_MPQ_BLOCK_COMPRESS) ? 'C' : '-';
			flags[3] = (block->flags & WOW_MPQ_BLOCK_ENCRYPTED) ? 'E' : '-';
			flags[4] = (block->flags & WOW_MPQ_BLOCK_FIX_KEY) ? 'F' : '-';
			flags[5] = (block->flags & WOW_MPQ_BLOCK_PATCH_FILE) ? 'P' : '-';
			flags[6] = (block->flags & WOW_MPQ_BLOCK_DELETE_MARKER) ? 'D' : '-';
			flags[7] = (block->flags & WOW_MPQ_BLOCK_SINGLE_UNIT) ? 'S' : '-';
			flags[8] = (block->flags & WOW_MPQ_BLOCK_SECTOR_CRC) ? 'R' : '-';
			flags[9] = (block->flags & WOW_MPQ_BLOCK_EXISTS) ? 'X' : '-';
			flags[10] = '\0';
			g_value_set_string(&value, flags);
		}
		else
		{
			g_value_set_string(&value, "err");
		}
		gtk_list_store_set_value(store, &iter, 5, &value);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store));
	gtk_widget_show(tree);
	display->display.root = tree;
	return &display->display;
}
