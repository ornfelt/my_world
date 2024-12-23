#include "../explorer.h"

#include "display.h"

#include <wow/mpq.h>

struct txt_display
{
	struct display display;
};

struct display *txt_display_new(const struct node *node, const char *path, struct wow_mpq_file *file)
{
	(void)node;
	(void)path;
	struct txt_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "txt display allocation failed\n");
		return NULL;
	}
	display->display.name = strdup(path);
	display->display.dtr = NULL;
	size_t len = strlen(path);
	const char *language_name;
	if (len >= 4 && !strcmp(&path[len - 4], ".lua"))
		language_name = "lua";
	else if (len >= 4 && !strcmp(&path[len - 4], ".xml"))
		language_name = "xml";
	else if ((len >= 4 && !strcmp(&path[len - 4], ".ini"))
	      || (len >= 4 && !strcmp(&path[len - 4], ".url")))
		language_name = "ini";
	else if (len >= 5 && !strcmp(&path[len - 5], ".html"))
		language_name = "html";
	else
		language_name = NULL;
	GtkSourceBuffer *buffer;
	if (language_name)
	{
		GtkSourceLanguage *language = gtk_source_language_manager_get_language(g_explorer->source_language_manager, language_name);
		buffer = gtk_source_buffer_new_with_language(language);
	}
	else
	{
		buffer = gtk_source_buffer_new(NULL);
	}
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), (const char*)file->data, file->size);
	GtkWidget *text = gtk_source_view_new_with_buffer(buffer);
	gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(text), true);
	gtk_source_view_set_show_line_marks(GTK_SOURCE_VIEW(text), true);
	gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(text), 4);
	gtk_text_view_set_monospace(GTK_TEXT_VIEW(text), true);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text), false);
	gtk_widget_set_vexpand(text, true);
	gtk_widget_show(text);
	display->display.root = text;
	return &display->display;
}
