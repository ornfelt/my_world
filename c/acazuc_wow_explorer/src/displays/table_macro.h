#ifndef TABLE_MACRO_H
#define TABLE_MACRO_H

#define TREE_BEGIN(ncolumns, ...) \
	GtkListStore *store = gtk_list_store_new(ncolumns, ##__VA_ARGS__); \
	GtkWidget *tree = gtk_tree_view_new(); \
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true); \
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new()

#define TREE_END() \
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store)); \
	gtk_widget_show(tree); \
	return tree

#define ADD_TREE_COLUMN(id, name) \
do \
{ \
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(name, renderer, "text", id, NULL); \
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column); \
	gtk_tree_view_column_set_sort_column_id(column, id); \
	gtk_tree_view_column_set_resizable(column, true); \
} while (0)

#define SET_TREE_VALUE_FMT(id, fmt, ...) \
do \
{ \
	char tmp[256]; \
	GValue value = G_VALUE_INIT; \
	g_value_init(&value, G_TYPE_STRING); \
	snprintf(tmp, sizeof(tmp), fmt, ##__VA_ARGS__); \
	g_value_set_string(&value, tmp); \
	gtk_list_store_set_value(store, &iter, id, &value); \
} while (0)

#define SET_TREE_VALUE_U64(id, v) \
do \
{ \
	GValue value = G_VALUE_INIT; \
	g_value_init(&value, G_TYPE_UINT64); \
	g_value_set_uint64(&value, v); \
	gtk_list_store_set_value(store, &iter, id, &value); \
} while (0)

#define SET_TREE_VALUE_I64(id, v) \
do \
{ \
	GValue value = G_VALUE_INIT; \
	g_value_init(&value, G_TYPE_INT64); \
	g_value_set_int64(&value, v); \
	gtk_list_store_set_value(store, &iter, id, &value); \
} while (0)

#define SET_TREE_VALUE_FLT(id, v) \
do \
{ \
	GValue value = G_VALUE_INIT; \
	g_value_init(&value, G_TYPE_FLOAT); \
	g_value_set_float(&value, v); \
	gtk_list_store_set_value(store, &iter, id, &value); \
} while (0)

#define SET_TREE_VALUE_STR(id, v) \
do \
{ \
	GValue value = G_VALUE_INIT; \
	g_value_init(&value, G_TYPE_STRING); \
	g_value_set_string(&value, v); \
	gtk_list_store_set_value(store, &iter, id, &value); \
} while (0)

#endif
