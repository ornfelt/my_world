#include "render/render.h"

#define RENDER_QUERY_VERSION      0
#define RENDER_QUERY_PICT_FORMATS 1
#define RENDER_CREATE_PICTURE     4
#define RENDER_COMPOSITE          8
#define RENDER_FILL_RECTANGLES    26

static int
req_render_query_version(struct xsrv *xsrv,
                         struct client *client,
                         struct request *request)
{
	if (request->length != 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU32(client, 0); /* major version */
	CLIENT_WU32(client, 11); /* minor version */
	CLIENT_WRITE(client, NULL, 16);
	return 1;
}

static int
req_render_query_pict_formats(struct xsrv *xsrv,
                              struct client *client,
                              struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	return 1;
}

static int
req_render_create_picture(struct xsrv *xsrv,
                          struct client *client,
                          struct request *request)
{
	if (request->length < 5)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t picture_id = CLIENT_RU32(client);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t pictfmt_id = CLIENT_RU32(client);
	uint32_t value_mask = CLIENT_RU32(client);
	uint32_t n = 0;
	for (size_t i = 0; i < 13; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	if (request->length != 5 + n)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (!client_has_free_id(xsrv, client, picture_id))
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, picture_id);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, drawable_id);
	struct pictfmt *pictfmt = render_get_format(xsrv, pictfmt_id);
	if (!pictfmt)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, pictfmt_id); /* XXX another error */
	}
	struct picture *picture = picture_new(xsrv, client, picture_id, drawable);
	if (!picture)
	{
		object_free(xsrv, OBJECT(drawable));
		object_free(xsrv, OBJECT(pictfmt));
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	}
	object_add(xsrv, &picture->object);
	TAILQ_INSERT_TAIL(&client->objects, &picture->object, client_chain);
	object_free(xsrv, OBJECT(drawable));
	object_free(xsrv, OBJECT(pictfmt));
	return 1;
}

static int
req_render_composite(struct xsrv *xsrv,
                     struct client *client,
                     struct request *request)
{
	if (request->length != 9)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	return 1;
}

static int
req_render_fill_rectangles(struct xsrv *xsrv,
                           struct client *client,
                           struct request *request)
{
	if (request->length < 7)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	return 1;
}

int
render_req(struct xsrv *xsrv,
           struct client *client,
           struct request *request)
{
	static const req_handler_t render_handlers[] =
	{
		[RENDER_QUERY_VERSION]      = req_render_query_version,
		[RENDER_QUERY_PICT_FORMATS] = req_render_query_pict_formats,
		[RENDER_CREATE_PICTURE]     = req_render_create_picture,
		[RENDER_COMPOSITE]          = req_render_composite,
		[RENDER_FILL_RECTANGLES]    = req_render_fill_rectangles,
	};
	uint8_t minor_opcode = request->detail;
	if (minor_opcode >= sizeof(render_handlers) / sizeof(*render_handlers)
	 || !render_handlers[minor_opcode])
		return CLIENT_BAD_REQUEST(xsrv, client, request);
	return render_handlers[minor_opcode](xsrv, client, request);
}
