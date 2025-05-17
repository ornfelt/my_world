#include "xsrv.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static void cursor_free(struct xsrv *xsrv, struct object *object);

static const struct object_def
cursor_def =
{
	.name = "cursor",
	.free = cursor_free,
};

void
cursor_register(struct xsrv *xsrv)
{
	xsrv->obj_cursor = register_object_type(xsrv, &cursor_def);
}

struct cursor *
cursor_new(struct xsrv *xsrv,
           struct client *client,
           uint32_t id,
           struct pixmap *color,
           struct pixmap *mask,
           uint16_t fore_red,
           uint16_t fore_green,
           uint16_t fore_blue,
           uint16_t back_red,
           uint16_t back_green,
           uint16_t back_blue,
           uint16_t xhot,
           uint16_t yhot,
           int16_t maskx,
           int16_t masky)
{
	struct cursor *cursor;

	cursor = malloc(sizeof(*cursor));
	if (!cursor)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	object_init(xsrv, client, &cursor->object, xsrv->obj_cursor, id);
	cursor->color = color;
	cursor->mask = mask;
	cursor->fore_red = fore_red;
	cursor->fore_green = fore_green;
	cursor->fore_blue = fore_blue;
	cursor->back_red = back_red;
	cursor->back_green = back_green;
	cursor->back_blue = back_blue;
	cursor->xhot = xhot;
	cursor->yhot = yhot;
	cursor->maskx = maskx;
	cursor->masky = masky;
	return cursor;
}

static void
cursor_free(struct xsrv *xsrv, struct object *object)
{
	struct cursor *cursor = (struct cursor*)object;

	object_free(xsrv, OBJECT(cursor->color));
	object_free(xsrv, OBJECT(cursor->mask));
}

struct cursor *
cursor_get(struct xsrv *xsrv, uint32_t id)
{
	return (struct cursor*)object_get_typed(xsrv, id, xsrv->obj_cursor);
}
