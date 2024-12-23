#ifndef EXPLORER_DISPLAY_H
#define EXPLORER_DISPLAY_H

#include <gtk/gtk.h>

struct wow_mpq_file;
struct display;
struct node;

typedef void (*display_dtr_t)(struct display *display);

struct display
{
	display_dtr_t dtr;
	GtkWidget *root;
	char *name;
};

void display_delete(struct display *display);
struct display *adt_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *blp_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *bls_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *dbc_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *dir_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *txt_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *wdl_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *wdt_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *m2_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *img_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *wmo_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);
struct display *wmo_group_display_new(const struct node *node, const char *path, struct wow_mpq_file *file);

#endif
