#include "xsrv.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static void pixmap_free(struct xsrv *xsrv, struct object *object);

static const struct object_def
pixmap_def =
{
	.name = "pixmap",
	.free = pixmap_free,
};

void
pixmap_register(struct xsrv *xsrv)
{
	xsrv->obj_pixmap = register_object_type(xsrv, &pixmap_def);
}

struct pixmap *
pixmap_new(struct xsrv *xsrv,
           struct client *client,
           uint32_t id,
           uint16_t width,
           uint16_t height,
           const struct format *format,
           struct window *root)
{
	struct pixmap *pixmap;

	pixmap = malloc(sizeof(*pixmap));
	if (!pixmap)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	if (drawable_init(xsrv, &pixmap->drawable, width, height, format, root))
	{
		free(pixmap);
		return NULL;
	}
	object_init(xsrv, client, &pixmap->object, xsrv->obj_pixmap, id);
	return pixmap;
}

static void
pixmap_free(struct xsrv *xsrv, struct object *object)
{
	struct pixmap *pixmap = (struct pixmap*)object;

	(void)xsrv;
	free(pixmap->drawable.data);
}

struct pixmap *
pixmap_get(struct xsrv *xsrv, uint32_t id)
{
	return (struct pixmap*)object_get_typed(xsrv, id, xsrv->obj_pixmap);
}
