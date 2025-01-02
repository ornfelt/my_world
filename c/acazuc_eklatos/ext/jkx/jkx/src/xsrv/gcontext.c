#include "xsrv.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static void gcontext_free(struct xsrv *xsrv, struct object *object);

static const struct object_def gcontext_def =
{
	.name = "gcontext",
	.free = gcontext_free,
};

void gcontext_register(struct xsrv *xsrv)
{
	xsrv->obj_gcontext = register_object_type(xsrv, &gcontext_def);
}

struct gcontext *gcontext_new(struct xsrv *xsrv, struct client *client,
                              uint32_t id, struct drawable *drawable,
                              struct gcontext_values *values)
{
	struct gcontext *gcontext = malloc(sizeof(*gcontext));
	if (!gcontext)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	object_init(xsrv, client, &gcontext->object, xsrv->obj_gcontext, id);
	gcontext->values = *values;
	gcontext->drawable = drawable;
	return gcontext;
}

static void gcontext_free(struct xsrv *xsrv, struct object *object)
{
	struct gcontext *gcontext = (struct gcontext*)object;
	object_free(xsrv, OBJECT(gcontext->values.tile));
	object_free(xsrv, OBJECT(gcontext->values.stipple));
	object_free(xsrv, OBJECT(gcontext->values.clip_mask));
	object_free(xsrv, OBJECT(gcontext->values.font));
	object_free(xsrv, OBJECT(gcontext->drawable));
}

struct gcontext *gcontext_get(struct xsrv *xsrv, uint32_t id)
{
	return (struct gcontext*)object_get_typed(xsrv, id, xsrv->obj_gcontext);
}
