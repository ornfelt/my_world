#ifndef EXPLORER_NODES_H
#define EXPLORER_NODES_H

#include <jks/array.h>

#include <sys/queue.h>

struct node;

struct node
{
	TAILQ_HEAD(, node) childs;
	char *name;
	struct node *parent;
	TAILQ_ENTRY(node) chain;
};

struct node *node_new(const char *name, struct node *parent);
void node_delete(struct node *node);
void node_add_child(struct node *node, struct node *child);
void node_get_path(struct node *node, char *str, size_t len);
void node_on_click(struct node *node);

#endif
