#ifndef EXPLORER_EXPLORER_H
#define EXPLORER_EXPLORER_H

#include <gtksourceview/gtksource.h>

#include <adwaita.h>
#include <stdint.h>

struct wow_mpq_compound;
struct jks_array;
struct display;
struct tree;
struct node;

struct explorer
{
	GtkSourceLanguageManager *source_language_manager;
	AdwApplication *application;
	GtkWidget *left_paned_scroll;
	GtkWidget *right_paned;
	GtkWidget *header_bar;
	GtkWidget *action_bar;
	GtkWidget *menu_bar;
	GtkWidget *tab_view;
	GtkWidget *tab_bar;
	GtkWidget *window;
	GtkWidget *paned;
	GtkWidget *box;
	struct wow_mpq_compound *mpq_compound;
	struct jks_array *mpq_archives; /* struct wow_mpq_archive* */
	struct node *root;
	struct tree *tree;
	uint32_t files_count;
	const char *locale;
	const char *game_path;
};

void explorer_set_display(struct explorer *explorer, struct display *display);
uint32_t get_color_from_height(float height, float min, float max);

extern struct explorer *g_explorer;

#endif
