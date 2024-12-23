#include "displays/display.h"

#include "explorer.h"
#include "nodes.h"

#include <wow/mpq.h>

#include <ctype.h>

typedef struct display *(*display_ctr_t)(const struct node *node, const char *path, struct wow_mpq_file *file);

static const struct
{
	const char *ext;
	display_ctr_t ctr;
} display_constructors[] =
{
	{".blp" , blp_display_new},
	{".dbc" , dbc_display_new},
	{".bls" , bls_display_new},
	{".wdl" , wdl_display_new},
	{".wdt" , wdt_display_new},
	{".adt" , adt_display_new},
	{".m2"  , m2_display_new},
	{".mdl" , m2_display_new},
	{".mdx" , m2_display_new},
	{".gif" , img_display_new},
	{".png" , img_display_new},
	{".jpg" , img_display_new},
	{".jpeg", img_display_new},
	{".tiff", img_display_new},
	{".js"  , txt_display_new},
	{".xml" , txt_display_new},
	{".lua" , txt_display_new},
	{".wtf" , txt_display_new},
	{".wfx" , txt_display_new},
	{".ini" , txt_display_new},
	{".txt" , txt_display_new},
	{".toc" , txt_display_new},
	{".url" , txt_display_new},
	{".css" , txt_display_new},
	{".html", txt_display_new},
	{".zmp" , txt_display_new},
	{".wmo" , wmo_display_new},
};

struct node *node_new(const char *name, struct node *parent)
{
	struct node *node = malloc(sizeof(*node));
	if (!node)
		return NULL;
	node->name = strdup(name);
	if (!node->name)
		goto err;
	node->parent = parent;
	TAILQ_INIT(&node->childs);
	return node;

err:
	free(node);
	return NULL;
}

void node_delete(struct node *node)
{
	if (!node)
		return;
	struct node *child;
	TAILQ_FOREACH(child, &node->childs, chain)
		node_delete(child);
	free(node);
}

void node_add_child(struct node *node, struct node *child)
{
	struct node *it;
	TAILQ_FOREACH(it, &node->childs, chain)
	{
		if (strcmp(child->name, it->name) >= 0)
			continue;
		TAILQ_INSERT_BEFORE(it, child, chain);
		return;
	}
	TAILQ_INSERT_TAIL(&node->childs, child, chain);
}

void node_get_path(struct node *node, char *str, size_t len)
{
	if (!node->parent)
	{
		snprintf(str, len, "%s", node->name);
		return;
	}
	node_get_path(node->parent, str, len);
	size_t pos = strlen(str);
	if (str[0])
	{
		snprintf(str + pos, len - pos, "\\");
		pos++;
	}
	snprintf(str + pos, len - pos, "%s", node->name);
}

static bool is_ext(const char *path, const char *ext)
{
	size_t len = strlen(path);
	size_t ext_len = strlen(ext);
	if (len < ext_len)
		return false;
	return !strcmp(&path[len - ext_len], ext);
}

void node_on_click(struct node *node)
{
	if (!TAILQ_EMPTY(&node->childs))
	{
		char path[512];
		node_get_path(node, path, sizeof(path));
		explorer_set_display(g_explorer, dir_display_new(node, path, NULL));
		return;
	}
	char path[512];
	node_get_path(node, path, sizeof(path));
	struct wow_mpq_file *file = wow_mpq_get_file(g_explorer->mpq_compound, path);
	if (!file)
		return;
	display_ctr_t ctr = NULL;
	for (size_t i = 0; i < sizeof(display_constructors) / sizeof(*display_constructors); ++i)
	{
		if (!is_ext(path, display_constructors[i].ext))
			continue;
		ctr = display_constructors[i].ctr;
		break;
	}
	if (!ctr)
		ctr = txt_display_new;
	struct display *display = display = ctr(node, path, file);
	if (display)
		explorer_set_display(g_explorer, display);
	else
		fprintf(stderr, "can't find handler for file \"%s\"\n", path);
	wow_mpq_file_delete(file);
}
