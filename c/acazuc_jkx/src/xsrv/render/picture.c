#include "render/render.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static void picture_destroy(struct xsrv *xsrv, struct object *object);

static const struct object_def picture_def =
{
	.name = "picture",
	.destroy = picture_destroy,
};

void picture_register(struct xsrv *xsrv)
{
	xsrv->obj_picture = register_object_type(xsrv, &picture_def);
}

struct picture *picture_new(struct xsrv *xsrv, struct client *client,
                            uint32_t id, struct drawable *drawable)
{
	struct picture *picture = malloc(sizeof(*picture));
	if (!picture)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	object_init(xsrv, client, &picture->object, xsrv->obj_picture, id);
	picture->drawable = drawable;
	return picture;
}

static void picture_destroy(struct xsrv *xsrv, struct object *object)
{
	struct picture *picture = (struct picture*)object;
	object_free(xsrv, OBJECT(picture->drawable));
}

struct picture *picture_get(struct xsrv *xsrv, uint32_t id)
{
	return (struct picture*)object_get_typed(xsrv, id, xsrv->obj_picture);
}
