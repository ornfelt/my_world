#include "xsrv.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static const struct object_def font_def =
{
	.name = "font",
};

void font_register(struct xsrv *xsrv)
{
	xsrv->obj_font = register_object_type(xsrv, &font_def);
}

struct font *font_new(struct xsrv *xsrv, struct client *client, uint32_t id,
                      struct font_def *def)
{
	struct font *font = malloc(sizeof(*font));
	if (!font)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	object_init(xsrv, client, &font->object, xsrv->obj_font, id);
	font->def = def;
	return font;
}

struct font *font_get(struct xsrv *xsrv, uint32_t id)
{
	return (struct font*)object_get_typed(xsrv, id, xsrv->obj_font);
}
