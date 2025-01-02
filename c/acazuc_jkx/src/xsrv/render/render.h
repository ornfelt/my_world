#ifndef RENDER_RENDER_H
#define RENDER_RENDER_H

#include "xsrv.h"

struct glyphset
{
	struct object object;
};

struct picture
{
	struct object object;
	struct drawable *drawable;
};

struct pictfmt
{
	struct object object;
};

int setup_render(struct xsrv *xsrv);

int render_req(struct xsrv *xsrv, struct client *client,
               struct request *request);

struct glyphset *glyphset_get(struct xsrv *xsrv, uint32_t id);

void picture_register(struct xsrv *xsrv);
struct picture *picture_new(struct xsrv *xsrv, struct client *client,
                            uint32_t id, struct drawable *drawable);
struct picture *picture_get(struct xsrv *xsrv, uint32_t id);

void glyphset_register(struct xsrv *xsrv);
struct glyphset *glyphset_new(struct xsrv *xsrv, struct client *client,
                              uint32_t id);
struct glyphset *glyphset_get(struct xsrv *xsrv, uint32_t id);

struct pictfmt *render_get_format(struct xsrv *xsrv, uint32_t id);

#endif
