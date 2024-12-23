#ifndef EXPLORER_TREE_H
#define EXPLORER_TREE_H

#include <gtk/gtk.h>

#include <stdint.h>

struct explorer;

struct tree
{
	struct explorer *explorer;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeStore *store;
	GtkWidget *treeview;
};

struct tree *tree_new(struct explorer *explorer);
void tree_delete(struct tree *tree);

#endif
