#include "xsrv.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static const struct object_def
colormap_def =
{
	.name = "colormap",
};

void
colormap_register(struct xsrv *xsrv)
{
	xsrv->obj_colormap = register_object_type(xsrv, &colormap_def);
}

struct colormap *
colormap_new(struct xsrv *xsrv, struct client *client, uint32_t id)
{
	struct colormap *colormap;

	colormap = malloc(sizeof(*colormap));
	if (!colormap)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	object_init(xsrv, client, &colormap->object, xsrv->obj_colormap, id);
	return colormap;
}

struct colormap *
colormap_get(struct xsrv *xsrv, uint32_t id)
{
	struct object *object;

	object = object_get(xsrv, id);
	if (!object)
		return NULL;
	if (object->type != xsrv->obj_colormap)
	{
		object->refs--;
		return NULL;
	}
	return (struct colormap*)object_get_typed(xsrv, id, xsrv->obj_colormap);
}
