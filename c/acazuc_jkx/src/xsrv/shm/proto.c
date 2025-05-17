#include "shm/shm.h"

#include <sys/shm.h>

#include <string.h>
#include <unistd.h>

#define SHM_QUERY_VERSION 0
#define SHM_ATTACH        1
#define SHM_DETACH        2
#define SHM_PUT_IMAGE     3

static int
req_shm_query_version(struct xsrv *xsrv,
                      struct client *client,
                      struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0); /* shared pixmaps */
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU16(client, 1); /* major version */
	CLIENT_WU16(client, 2); /* minor version */
	CLIENT_WU16(client, getuid());
	CLIENT_WU16(client, getgid());
	CLIENT_WU8(client, 0); /* pixmap format */
	CLIENT_WRITE(client, NULL, 15);
	return 1;
}

static int
req_shm_attach(struct xsrv *xsrv,
               struct client *client,
               struct request *request)
{
	if (request->length != 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t shmseg_id = CLIENT_RU32(client);
	uint32_t shmid = CLIENT_RU32(client);
	uint8_t read_only = CLIENT_RU8(client);
	CLIENT_READ(client, NULL, 3);
	if (!client_has_free_id(xsrv, client, shmseg_id))
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, shmseg_id);
	void *ptr = shmat(shmid, NULL, read_only ? SHM_RDONLY : 0);
	if (ptr == (void*)-1)
		return CLIENT_BAD_MATCH(xsrv, client, request);
	struct shmseg *shmseg = shmseg_new(xsrv, client, shmseg_id, ptr);
	if (!shmseg)
	{
		shmdt(ptr);
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	}
	object_add(xsrv, &shmseg->object);
	TAILQ_INSERT_TAIL(&client->objects, &shmseg->object, client_chain);
	return 1;
}

static int
req_shm_detach(struct xsrv *xsrv,
               struct client *client,
               struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t shmseg_id = CLIENT_RU32(client);
	struct shmseg *shmseg = shmseg_get(xsrv, shmseg_id);
	if (!shmseg)
		return CLIENT_BAD_VALUE(xsrv, client, request, shmseg_id); /* XXX specific error */
	object_destroy(xsrv, OBJECT(shmseg));
	object_free(xsrv, OBJECT(shmseg));
	return 1;
}

static void
shm_put(struct shmseg *shmseg,
        struct drawable *drawable,
        int16_t x,
        int16_t y,
        uint16_t width,
        uint16_t height,
        uint16_t src_x,
        uint16_t src_y,
        uint16_t total_width,
        uint16_t total_height)
{
	if (x < 0)
	{
		if (width <= -x)
			return;
		width += x;
		x = 0;
	}
	if (x >= drawable->width)
		return;
	if (x + width > drawable->width)
		width = drawable->width - x;
	if (y < 0)
	{
		if (height <= -y)
			return;
		height += y;
		y = 0;
	}
	if (y >= drawable->height)
		return;
	if (y + height > drawable->height)
		height = drawable->height - y;
	if (src_x >= total_width)
		return;
	if (src_x + width > total_width)
		width = total_width - src_x;
	if (src_y >= total_height)
		return;
	if (src_y + height > total_height)
		height = total_height - src_y;
	uint8_t *dst = (uint8_t*)drawable->data;
	dst += y * drawable->pitch;
	dst += x * drawable->format->bpp / 8;
	uint32_t src_pitch = total_width * drawable->format->bpp / 8;
	uint8_t *src = (uint8_t*)shmseg->ptr;
	src += src_y * src_pitch;
	src += src_x * drawable->format->bpp / 8;
	size_t bytes = width * drawable->format->bpp / 8;
	for (int32_t yy = 0; yy < height; ++yy)
	{
		memcpy(dst, src, bytes);
		src += src_pitch;
		dst += drawable->pitch;
	}
}

static int
req_shm_put_image(struct xsrv *xsrv,
                  struct client *client,
                  struct request *request)
{
	if (request->length != 10)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gcontext_id = CLIENT_RU32(client);
	uint16_t total_width = CLIENT_RU16(client);
	uint16_t total_height = CLIENT_RU16(client);
	uint16_t src_x = CLIENT_RU16(client);
	uint16_t src_y = CLIENT_RU16(client);
	uint16_t src_width = CLIENT_RU16(client);
	uint16_t src_height = CLIENT_RU16(client);
	int16_t dst_x = CLIENT_RI16(client);
	int16_t dst_y = CLIENT_RI16(client);
	uint8_t depthv = CLIENT_RU8(client);
	uint8_t format = CLIENT_RU8(client);
	uint8_t send_event = CLIENT_RU8(client);
	(void)send_event; /* XXX */
	CLIENT_READ(client, NULL, 1);
	uint32_t shmseg_id = CLIENT_RU32(client);
	uint32_t offset = CLIENT_RU32(client);
	(void)offset; /* XXX */
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	}
	struct shmseg *shmseg = shmseg_get(xsrv, shmseg_id);
	if (!shmseg)
	{
		object_free(xsrv, OBJECT(gcontext));
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_VALUE(xsrv, client, request, shmseg_id); /* XXX specific error */
	}
	int ret;
	switch (format)
	{
		case XYBitmap:
			ret = CLIENT_BAD_IMPLEM(xsrv, client, request);
			break;
		case XYPixmap:
			ret = CLIENT_BAD_IMPLEM(xsrv, client, request);
			break;
		case ZPixmap:
			if (depthv != drawable->format->depth)
			{
				ret = CLIENT_BAD_MATCH(xsrv, client, request);
				break;
			}
			shm_put(shmseg, drawable, dst_x, dst_y,
			        src_width, src_height, src_x, src_y,
			        total_width, total_height);
			ret = 1;
			break;
		default:
			ret = CLIENT_BAD_VALUE(xsrv, client, request,
			                       format);
			break;
	}
	test_fb_redraw(xsrv, drawable, dst_x, dst_y,
	               dst_x + src_width, dst_y + src_height);
	object_free(xsrv, OBJECT(gcontext));
	object_free(xsrv, OBJECT(drawable));
	object_free(xsrv, OBJECT(shmseg));
	return ret;
}

int
shm_req(struct xsrv *xsrv,
        struct client *client,
        struct request *request)
{
	static const req_handler_t shm_handlers[] =
	{
		[SHM_QUERY_VERSION] = req_shm_query_version,
		[SHM_ATTACH]        = req_shm_attach,
		[SHM_DETACH]        = req_shm_detach,
		[SHM_PUT_IMAGE]     = req_shm_put_image,
	};
	uint8_t minor_opcode = request->detail;
	if (minor_opcode >= sizeof(shm_handlers) / sizeof(*shm_handlers)
	 || !shm_handlers[minor_opcode])
		return CLIENT_BAD_REQUEST(xsrv, client, request);
	return shm_handlers[minor_opcode](xsrv, client, request);
}
