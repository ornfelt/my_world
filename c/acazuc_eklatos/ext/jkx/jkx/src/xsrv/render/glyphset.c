#include "render/render.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static const struct object_def glyphset_def =
{
	.name = "glyphset",
};

void glyphset_register(struct xsrv *xsrv)
{
	xsrv->obj_glyphset = register_object_type(xsrv, &glyphset_def);
}

struct glyphset *glyphset_new(struct xsrv *xsrv, struct client *client,
                            uint32_t id)
{
	struct glyphset *glyphset = malloc(sizeof(*glyphset));
	if (!glyphset)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	object_init(xsrv, client, &glyphset->object, xsrv->obj_glyphset, id);
	return glyphset;
}

struct glyphset *glyphset_get(struct xsrv *xsrv, uint32_t id)
{
	return (struct glyphset*)object_get_typed(xsrv, id, xsrv->obj_glyphset);
}
