#include "display.h"

#include <wow/mpq.h>

struct img_display
{
	struct display display;
};

struct display *img_display_new(const struct node *node, const char *path, struct wow_mpq_file *file)
{
	(void)node;
	struct img_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "img display allocation failed\n");
		return NULL;
	}
	display->display.name = strdup(path);
	display->display.dtr = NULL;
	const char *ext = strrchr(path, '.');
	if (!strcmp(ext, ".jpg"))
		ext = "jpeg";
	else
		ext++;
	GdkPixbufLoader *loader = gdk_pixbuf_loader_new_with_type(ext, NULL);
	if (!loader)
	{
		fprintf(stderr, "failed to create gdk loader with ext %s\n", ext);
		free(display);
		return NULL;
	}
	if (!gdk_pixbuf_loader_write(loader, file->data, file->size, NULL))
	{
		fprintf(stderr, "failed to write data to loader\n");
		/* XXX free loader */
		free(display);
		return NULL;
	}
	GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	if (!pixbuf)
	{
		fprintf(stderr, "failed to get pixbuf from loader\n");
		/* XXX free loader */
		free(display);
		return NULL;
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_widget_show(image);
	display->display.root = image;
	return &display->display;
}
