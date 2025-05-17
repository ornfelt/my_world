#include "glx/glx.h"

#define GLX_RENDER                  1
#define GLX_RENDER_LARGE            2
#define GLX_CREATE_CONTEXT          3
#define GLX_DESTROY_CONTEXT         4
#define GLX_MAKE_CURRENT            5
#define GLX_IS_DIRECT               6
#define GLX_QUERY_VERSION           7
#define GLX_WAIT_GL                 8
#define GLX_WAIT_X                  9
#define GLX_COPY_CONTEXT            10
#define GLX_SWAP_BUFFERS            11
#define GLX_QUERY_EXTENSIONS_STRING 18
#define GLX_QUERY_SERVER_STRING     19
#define GLX_MAKE_CONTEXT_CURRENT    26

static int
req_glx_query_version(struct xsrv *xsrv,
                      struct client *client,
                      struct request *request)
{
	if (request->length != 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	/* XXX read major / minor */
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU32(client, 1); /* major version */
	CLIENT_WU32(client, 4); /* minor version */
	CLIENT_WRITE(client, NULL, 16);
	return 1;
}

int
glx_req(struct xsrv *xsrv,
        struct client *client,
        struct request *request)
{
	static const req_handler_t glx_handlers[] =
	{
		[GLX_QUERY_VERSION] = req_glx_query_version,
	};
	uint8_t minor_opcode = request->detail;
	if (minor_opcode >= sizeof(glx_handlers) / sizeof(*glx_handlers)
	 || !glx_handlers[minor_opcode])
		return CLIENT_BAD_REQUEST(xsrv, client, request);
	return glx_handlers[minor_opcode](xsrv, client, request);
}
