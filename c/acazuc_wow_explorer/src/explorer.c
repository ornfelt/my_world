#include "displays/display.h"

#include "explorer.h"
#include "nodes.h"
#include "tree.h"

#include <wow/mpq.h>

#include <jks/array.h>

#include <inttypes.h>
#include <getopt.h>
#include <ctype.h>

struct explorer *g_explorer;

static struct explorer *explorer_new(void)
{
	struct explorer *explorer = calloc(sizeof(*explorer), 1);
	if (!explorer)
	{
		fprintf(stderr, "explorer allocation failed\n");
		return NULL;
	}
	explorer->game_path = "WoW";
	explorer->locale = "frFR";
	return explorer;
}

static void explorer_delete(struct explorer *explorer)
{
	if (!explorer)
		return;
	tree_delete(explorer->tree);
	node_delete(explorer->root);
	g_object_unref(explorer->application);
	free(explorer);
}

static void archive_delete(void *ptr)
{
	wow_mpq_archive_delete(*(struct wow_mpq_archive**)ptr);
}

static bool setup_game_files(struct explorer *explorer)
{
	explorer->mpq_archives = malloc(sizeof(*explorer->mpq_archives));
	if (!explorer->mpq_archives)
	{
		fprintf(stderr, "mpq archives allocation failed\n");
		return EXIT_FAILURE;
	}
	jks_array_init(explorer->mpq_archives, sizeof(struct wow_mpq_archive*), archive_delete, NULL);
	char files[14][256];
	snprintf(files[0] , sizeof(files[0]) , "patch-5.MPQ");
	snprintf(files[1] , sizeof(files[1]) , "patch-3.MPQ");
	snprintf(files[2] , sizeof(files[2]) , "patch-2.MPQ");
	snprintf(files[3] , sizeof(files[3]) , "patch.MPQ");
	snprintf(files[4] , sizeof(files[4]) , "%s/patch-%s-2.MPQ", explorer->locale, explorer->locale);
	snprintf(files[5] , sizeof(files[5]) , "%s/patch-%s.MPQ", explorer->locale, explorer->locale);
	snprintf(files[6] , sizeof(files[6]) , "expansion.MPQ");
	snprintf(files[7] , sizeof(files[7]) , "common.MPQ");
	snprintf(files[8] , sizeof(files[8]) , "%s/base-%s.MPQ", explorer->locale, explorer->locale);
	snprintf(files[9] , sizeof(files[9]) , "%s/backup-%s.MPQ", explorer->locale, explorer->locale);
	snprintf(files[10], sizeof(files[10]), "%s/expansion-locale-%s.MPQ", explorer->locale, explorer->locale);
	snprintf(files[11], sizeof(files[11]), "%s/locale-%s.MPQ", explorer->locale, explorer->locale);
	snprintf(files[12], sizeof(files[12]), "%s/expansion-speech-%s.MPQ", explorer->locale, explorer->locale);
	snprintf(files[13], sizeof(files[13]), "%s/speech-%s.MPQ", explorer->locale, explorer->locale);
	for (size_t i = 0; i < sizeof(files) / sizeof(*files); ++i)
	{
		char name[512];
		snprintf(name, sizeof(name), "%s/Data/%s", explorer->game_path, files[i]);
		struct wow_mpq_archive *archive = wow_mpq_archive_new(name);
		if (!archive)
		{
			fprintf(stderr, "failed to open archive \"%s\"\n", name);
			continue;
		}
		if (!jks_array_push_back(explorer->mpq_archives, &archive))
		{
			fprintf(stderr, "failed to add archive to list\n");
			continue;
		}
	}
	explorer->mpq_compound = wow_mpq_compound_new();
	if (!explorer->mpq_compound)
	{
		fprintf(stderr, "failed to get compound\n");
		return false;
	}
	for (size_t i = 0; i < explorer->mpq_archives->size; ++i)
	{
		struct wow_mpq_archive *archive = *(struct wow_mpq_archive**)jks_array_get(explorer->mpq_archives, i);
		if (!wow_mpq_compound_add_archive(explorer->mpq_compound, archive))
		{
			wow_mpq_compound_delete(explorer->mpq_compound);
			explorer->mpq_compound = NULL;
			return false;
		}
	}
	return true;
}

static bool add_mpq_file(struct explorer *explorer, const char *path)
{
	struct node *parent = explorer->root;
	struct node *new_node;
	const char *prev = path;
	const char *pos;
	while ((pos = strchr(prev, '\\')))
	{
		if (pos == prev)
		{
			pos++;
			continue;
		}
		char dir[512];
		snprintf(dir, sizeof(dir), "%.*s", (int)(pos - prev), prev);
		for (size_t i = 0; dir[i]; ++i)
			dir[i] = tolower(dir[i]);
		struct node *node;
		TAILQ_FOREACH(node, &parent->childs, chain)
		{
			if (!strcmp(node->name, dir))
			{
				parent = node;
				goto next_iter;
			}
		}
		new_node = node_new(dir, parent);
		node_add_child(parent, new_node);
		parent = new_node;
next_iter:
		pos++;
		prev = pos;
	}
	size_t rem = strlen(path) - (prev - path);
	if (rem > 0)
	{
		char file[512];
		snprintf(file, sizeof(file), "%s", prev);
		size_t len = strlen(file);
		for (size_t i = 0; i < len; ++i)
			file[i] = tolower(file[i]);
		if (file[len - 1] == '\r')
			file[len - 1] = '\0';
		struct node *node;
		TAILQ_FOREACH(node, &parent->childs, chain)
		{
			if (!strcmp(node->name, file))
			{
				parent = node;
				return true;
			}
		}
		node_add_child(parent, node_new(file, parent));
		explorer->files_count++;
	}
	return true;
}

static void load_files(struct explorer *explorer)
{
	/* XXX create popup to diplay loading files */
	explorer->root = node_new("", NULL);
	struct wow_mpq_compound *compound = explorer->mpq_compound;
	for (uint32_t i = 0; i < compound->archives_nb; ++i)
	{
		struct wow_mpq_archive_view *archive = &compound->archives[i];
		struct wow_mpq_file *file = wow_mpq_get_archive_file(archive, "(listfile)");
		if (!file)
		{
			fprintf(stderr, "failed to get (listfile) in archive %s\n", archive->archive->filename);
			continue;
		}
		const char *prev = (const char*)file->data;
		const char *pos;
		while ((pos = (const char*)memchr(prev, '\n', file->size - (prev - (const char*)file->data))))
		{
			if (pos == prev)
			{
				pos++;
				continue;
			}
			char path[256];
			snprintf(path, sizeof(path), "%.*s", (int)(pos - prev), prev);
			size_t len = strlen(path);
			if (path[len - 1] == '\r')
				path[len - 1] = '\0';
			if (!add_mpq_file(explorer, path))
				fprintf(stderr, "failed to add mpq file\n");
			pos++;
			prev = pos;
		}
		wow_mpq_file_delete(file);
	}
}

static void application_activate(GtkApplication *application,
                                 gpointer userdata)
{
	struct explorer *explorer = userdata;

	/* MenuBar */
	//explorer->menu_bar = gtk_popover_menu_bar_new_from_model(NULL);
	//gtk_widget_show(explorer->menu_bar);
	/* LeftPaned */
	explorer->tree = tree_new(explorer);
	explorer->left_paned_scroll = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(explorer->left_paned_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_vexpand(explorer->left_paned_scroll, true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(explorer->left_paned_scroll), explorer->tree->treeview);
	gtk_widget_show(explorer->left_paned_scroll);
	/* TabView */
	explorer->tab_view = GTK_WIDGET(adw_tab_view_new());
	gtk_widget_show(explorer->tab_view);
	/* TabBar */
	explorer->tab_bar = GTK_WIDGET(adw_tab_bar_new());
	adw_tab_bar_set_view(ADW_TAB_BAR(explorer->tab_bar), ADW_TAB_VIEW(explorer->tab_view));
	adw_tab_bar_set_autohide(ADW_TAB_BAR(explorer->tab_bar), false);
	gtk_widget_set_hexpand(explorer->tab_bar, true);
	gtk_widget_show(explorer->tab_bar);
	/* RightPaned */
	explorer->right_paned = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_append(GTK_BOX(explorer->right_paned), explorer->tab_view);
	gtk_widget_show(explorer->right_paned);
	/* Paned */
	explorer->paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_start_child(GTK_PANED(explorer->paned), explorer->left_paned_scroll);
	gtk_paned_set_end_child(GTK_PANED(explorer->paned), explorer->right_paned);
	gtk_paned_set_position(GTK_PANED(explorer->paned), 300);
	gtk_widget_show(explorer->paned);
	/* ActionBar */
	explorer->action_bar = gtk_action_bar_new();
	gtk_widget_show(explorer->action_bar);
	/* MPQ num */
	char nummpq[256];
	snprintf(nummpq, sizeof(nummpq), "MPQs: %" PRIu32, explorer->mpq_compound->archives_nb);
	GtkWidget *mpq_num = gtk_label_new(nummpq);
	gtk_widget_show(mpq_num);
	gtk_action_bar_pack_end(GTK_ACTION_BAR(explorer->action_bar), mpq_num);
	/* Files count */
	char numfiles[256];
	snprintf(numfiles, sizeof(numfiles), "files: %" PRIu32, explorer->files_count);
	GtkWidget *file_count = gtk_label_new(numfiles);
	gtk_widget_show(file_count);
	gtk_action_bar_pack_end(GTK_ACTION_BAR(explorer->action_bar), file_count);
	/* Style */
	AdwStyleManager *manager = adw_style_manager_get_default();
	adw_style_manager_set_color_scheme(manager, ADW_COLOR_SCHEME_DEFAULT);
	/* Header */
	explorer->header_bar = adw_header_bar_new();
	adw_header_bar_set_title_widget(ADW_HEADER_BAR(explorer->header_bar), explorer->tab_bar);
	gtk_widget_show(explorer->header_bar);
	/* Box */
	explorer->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_append(GTK_BOX(explorer->box), explorer->header_bar);
	gtk_box_append(GTK_BOX(explorer->box), explorer->paned);
	gtk_box_append(GTK_BOX(explorer->box), explorer->action_bar);
	gtk_widget_show(explorer->box);
	/* Window */
	explorer->window = g_object_new(ADW_TYPE_APPLICATION_WINDOW, "application", application, NULL);
	gtk_window_set_default_size(GTK_WINDOW(explorer->window), 1280, 720);
	gtk_window_present(GTK_WINDOW(explorer->window));
	adw_application_window_set_content(ADW_APPLICATION_WINDOW(explorer->window), explorer->box);
}

static int explorer_run(struct explorer *explorer, int argc, char **argv)
{
	explorer->application = adw_application_new("org.acazuc.wow_explorer", G_APPLICATION_NON_UNIQUE);
	if (!setup_game_files(explorer))
	{
		fprintf(stderr, "failed to setup game files\n");
		return EXIT_FAILURE;
	}
	g_signal_connect(explorer->application, "activate", G_CALLBACK(application_activate), explorer);
	gtk_source_init();
	explorer->source_language_manager = gtk_source_language_manager_get_default ();
	load_files(explorer);
	return g_application_run(G_APPLICATION(explorer->application), argc, argv);
}

static void display_destroy(GtkWidget *self, gpointer userdata)
{
	(void)self;
	struct display *display = userdata;
	display_delete(display);
}

void explorer_set_display(struct explorer *explorer, struct display *display)
{
	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_widget_set_vexpand(display->root, true);
	gtk_widget_set_hexpand(display->root, true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), display->root);
	g_signal_connect(scrolled, "destroy", G_CALLBACK(display_destroy), display);
	gtk_widget_show(scrolled);
	AdwTabPage *page = adw_tab_view_append(ADW_TAB_VIEW(explorer->tab_view), scrolled);
	if (display->name)
		adw_tab_page_set_title(page, display->name);
	adw_tab_view_set_selected_page(ADW_TAB_VIEW(explorer->tab_view), page);
}

uint32_t get_color_from_height(float height, float min, float max)
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	float range = max - min;
	float step = range / 7;
	if (height <= min)
	{
		r = 0;
		g = 0;
		b = 0;
	}
	else if (height < min + step * 1)
	{
		r = (height - min) / step * 255;
		g = 0;
		b = 0;
	}
	else if (height < min + step * 2)
	{
		r = 255;
		g = (height - (min + step * 1)) / step * 255;
		b = 0;
	}
	else if (height < min + step * 3)
	{
		r = 255 - (height - (min + step * 2)) / step * 255;
		g = 255;
		b = 0;
	}
	else if (height < min + step * 4)
	{
		r = 0;
		g = 255;
		b = (height - (min + step * 3)) / step * 255;
	}
	else if (height < min + step * 5)
	{
		r = 0;
		g = 255 - (height - (min + step * 4)) / step * 255;
		b = 255;
	}
	else if (height < min + step * 6)
	{
		r = (height - (min + step * 5)) / step * 255;
		g = 0;
		b = 255;
	}
	else if (height < min + step * 7)
	{
		r = 255;
		g = (height - (min + step * 6)) / step * 255;
		b = 255;
	}
	else
	{
		r = 255;
		g = 255;
		b = 255;
	}
	return (r << 16) | (g << 8) | b;
}

static void usage(void)
{
	printf("explorer [-h] [-p <path>] [-l <locale>]\n");
	printf("-h: show this help\n");
	printf("-p: set the game path\n");
	printf("-l: set the locale (frFR, enUS, ..)\n");
}

int main(int argc, char **argv)
{
	g_explorer = explorer_new();
	if (!g_explorer)
		return EXIT_FAILURE;
	int c;
	while ((c = getopt(argc, argv, "hp:l:")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage();
				return EXIT_SUCCESS;
			case 'p':
				g_explorer->game_path = optarg;
				break;
			case 'l':
				g_explorer->locale = optarg;
				break;
			default:
				usage();
				return EXIT_FAILURE;
		}
	}
	int ret = explorer_run(g_explorer, argc, argv);
	explorer_delete(g_explorer);
	return ret;
}
