#include "xsrv.h"

#include <sys/socket.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#if 0
#define DEBUG_REQ_TIMING
#endif

struct client *
client_new(struct xsrv *xsrv, int fd)
{
	struct client *client = NULL;
	uint32_t id_base;

	id_base = xsrv_allocate_id_base(xsrv);
	if (!id_base)
		goto err;
	client = calloc(sizeof(*client), 1);
	if (!client)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		goto err;
	}
	client->state = CLIENT_SETUP;
	client->fd = -1;
	if (ringbuf_init(&client->rbuf, 1024 * 1024))
	{
		fprintf(stderr, "%s: failed to create client rbuf\n",
		        xsrv->progname);
		goto err;
	}
	if (ringbuf_init(&client->wbuf, 1024 * 1024))
	{
		fprintf(stderr, "%s: failed to create client wbuf\n",
		        xsrv->progname);
		goto err;
	}
	client->fd = fd;
	client->id_base = id_base;
	client->id_mask = 0xFFFFF;
	TAILQ_INIT(&client->objects);
	TAILQ_INIT(&client->events);
	TAILQ_INIT(&client->button_grabs);
	xsrv->stats.client_count++;
	return client;

err:
	if (client)
	{
		ringbuf_destroy(&client->rbuf);
		ringbuf_destroy(&client->wbuf);
		free(client);
	}
	return NULL;
}

void
client_delete(struct xsrv *xsrv, struct client *client)
{
	struct button_grab *button_grab;
	struct window_event *event;
	struct object *object;

	event = TAILQ_FIRST(&client->events);
	while (event)
	{
		TAILQ_REMOVE(&event->window->events, event, window_chain);
		TAILQ_REMOVE(&event->client->events, event, client_chain);
		free(event);
		event = TAILQ_FIRST(&client->events);
	}
	object = TAILQ_FIRST(&client->objects);
	while (object)
	{
		object_destroy(xsrv, object);
		object_free(xsrv, object);
		object = TAILQ_FIRST(&client->objects);
	}
	button_grab = TAILQ_FIRST(&client->button_grabs);
	while (button_grab)
	{
		delete_button_grab(xsrv, button_grab);
		button_grab = TAILQ_FIRST(&client->button_grabs);
	}
	if (xsrv->pointer_grab.client)
		xsrv_ungrab_pointer(xsrv);
	close(client->fd);
	TAILQ_REMOVE(&xsrv->clients, client, chain);
	xsrv_release_id_base(xsrv, client->id_base);
	if (xsrv->grab_client == client)
	{
		xsrv->grab_client = NULL;
		xsrv->grab_client_nb = 0;
	}
	xsrv->stats.client_count--;
	free(client);
}

int
client_recv(struct xsrv *xsrv, struct client *client)
{
	size_t avail;
	ssize_t ret;

	if (!(client->sock_state & CLIENT_SOCK_RECV))
		return 0;
	avail = ringbuf_contiguous_write_size(&client->rbuf);
	ret = recv(client->fd, ringbuf_write_ptr(&client->rbuf), avail, 0);
#if 0
	printf("read %ld bytes\n", (long)ret);
#endif
	if (ret == -1)
	{
		if (errno == EAGAIN)
		{
			client->sock_state &= ~CLIENT_SOCK_RECV;
			return 0;
		}
		if (errno == EINTR)
			return 0;
		fprintf(stderr, "%s: recv: %s\n", xsrv->progname,
		        strerror(errno));
		client->state = CLIENT_CLOSED;
		return 1;
	}
	if (ret == 0)
	{
		client->state = CLIENT_CLOSED;
		return 1;
	}
	ringbuf_advance_write(&client->rbuf, ret);
	return 0;
}

int
client_send(struct xsrv *xsrv, struct client *client)
{
	size_t avail;
	ssize_t ret;

	if (!(client->sock_state & CLIENT_SOCK_SEND))
		return 0;
	avail = ringbuf_contiguous_read_size(&client->wbuf);
	if (!avail)
		return 0;
	ret = send(client->fd, ringbuf_read_ptr(&client->wbuf), avail, 0);
	if (ret == -1)
	{
		if (errno == EAGAIN)
		{
			client->sock_state &= ~CLIENT_SOCK_SEND;
			return 0;
		}
		if (errno == EINTR)
			return 0;
		fprintf(stderr, "%s: send: %s\n", xsrv->progname,
		        strerror(errno));
		client->state = CLIENT_CLOSED;
		return 1;
	}
#if 0
	printf("sent %ld bytes\n", (long)ret);
#endif
	ringbuf_advance_read(&client->wbuf, ret);
	return 0;
}

static int
process_setup(struct xsrv *xsrv, struct client *client)
{
	(void)xsrv;
	if (ringbuf_read_size(&client->rbuf) < 12)
		return 0;
	client->init_data.byte_order = CLIENT_RU8(client);
	if (client->init_data.byte_order == 'B')
	{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		client->rbuf.bitswap = 1;
		client->wbuf.bitswap = 1;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		client->rbuf.bitswap = 0;
		client->wbuf.bitswap = 0;
#else
#error "unknown endianness"
#endif
	}
	else if (client->init_data.byte_order == 'l')
	{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		client->rbuf.bitswap = 0;
		client->wbuf.bitswap = 0;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		client->rbuf.bitswap = 1;
		client->wbuf.bitswap = 1;
#else
#error "unknown endianness"
#endif
	}
	else
	{
		client->state = CLIENT_CLOSED;
		return -1;
	}
	ringbuf_advance_read(&client->rbuf, 1);
	client->init_data.major_version = CLIENT_RU16(client);
	client->init_data.minor_version = CLIENT_RU16(client);
	client->init_data.auth_proto_name_len = CLIENT_RU16(client);
	client->init_data.auth_proto_data_len = CLIENT_RU16(client);
	ringbuf_advance_read(&client->rbuf, 2);
	client->state = CLIENT_CONNECTING;
	return 1;
}

static int
process_connecting(struct xsrv *xsrv, struct client *client)
{
	if (ringbuf_read_size(&client->rbuf) < client->init_data.auth_proto_name_len
	                                     + client->init_data.auth_proto_data_len)
		return 0;
	if (client->init_data.auth_proto_name_len)
	{
		/* XXX */
		ringbuf_advance_read(&client->rbuf, client->init_data.auth_proto_name_len);
		CLIENT_RPAD(client, client->init_data.auth_proto_name_len);
	}
	if (client->init_data.auth_proto_data_len)
	{
		/* XXX */
		ringbuf_advance_read(&client->rbuf, client->init_data.auth_proto_data_len);
		CLIENT_RPAD(client, client->init_data.auth_proto_data_len);
	}
	uint16_t vendor_len = strlen(VENDOR);
	uint16_t screens_len = 10 * xsrv->screens_count;
	for (size_t i = 0; i < xsrv->screens_count; ++i)
	{
		struct screen *screen = xsrv->screens[i];
		screens_len += 2 * screen->depths_count;
		for (size_t j = 0; j < screen->depths_count; ++j)
		{
			struct depth *depth = screen->depths[j];
			screens_len += 6 * depth->visuals_count;
		}
	}
	uint16_t length = 8 + xsrv->formats_count * 2 + (vendor_len + 3) / 4 + screens_len;
	CLIENT_WU8(client, 1); /* OK */
	CLIENT_WU8(client, 0); /* pad */
	CLIENT_WU16(client, VERSION_MAJOR);
	CLIENT_WU16(client, VERSION_MINOR);
	CLIENT_WU16(client, length);
	CLIENT_WU32(client, RELEASE_NUMBER);
	CLIENT_WU32(client, client->id_base);
	CLIENT_WU32(client, client->id_mask);
	CLIENT_WU32(client, 256); /* motion buffer size */
	CLIENT_WU16(client, vendor_len);
	CLIENT_WU16(client, 65535); /* max request length */
	CLIENT_WU8(client, xsrv->screens_count);
	CLIENT_WU8(client, xsrv->formats_count);
	CLIENT_WU8(client, LSBFirst);
	CLIENT_WU8(client, LSBFirst);
	CLIENT_WU8(client, 32); /* bitmap unit */
	CLIENT_WU8(client, 32); /* bitmap pad */
	CLIENT_WU8(client, xsrv->keyboard.min_keycode);
	CLIENT_WU8(client, xsrv->keyboard.max_keycode);
	CLIENT_WU32(client, 0); /* pad */
	CLIENT_WRITE(client, VENDOR, vendor_len);
	CLIENT_WPAD(client, vendor_len);
	for (size_t i = 0; i < xsrv->formats_count; ++i)
	{
		struct format *format = xsrv->formats[i];
		CLIENT_WU8(client, format->depth);
		CLIENT_WU8(client, format->bpp);
		CLIENT_WU8(client, format->scanline_pad);
		CLIENT_WRITE(client, NULL, 5);
	}
	for (size_t i = 0; i < xsrv->screens_count; ++i)
	{
		struct screen *screen = xsrv->screens[i];
		CLIENT_WU32(client, screen->root->object.id);
		CLIENT_WU32(client, screen->colormap->object.id);
		CLIENT_WU32(client, screen->white_pixel);
		CLIENT_WU32(client, screen->black_pixel);
		CLIENT_WU32(client, screen->input_mask);
		CLIENT_WU16(client, screen->width);
		CLIENT_WU16(client, screen->height);
		CLIENT_WU16(client, screen->mm_width);
		CLIENT_WU16(client, screen->mm_height);
		CLIENT_WU16(client, screen->min_maps);
		CLIENT_WU16(client, screen->max_maps);
		CLIENT_WU32(client, screen->visual->id);
		CLIENT_WU8(client, screen->backing_stores);
		CLIENT_WU8(client, screen->save_unders);
		CLIENT_WU8(client, screen->root_depth);
		CLIENT_WU8(client, screen->depths_count);
		for (size_t j = 0; j < screen->depths_count; ++j)
		{
			struct depth *depth = screen->depths[j];
			CLIENT_WU8(client, depth->depth);
			CLIENT_WRITE(client, NULL, 1);
			CLIENT_WU16(client, depth->visuals_count);
			CLIENT_WRITE(client, NULL, 4);
			for (size_t k = 0; k < depth->visuals_count; ++k)
			{
				struct visual *visual = depth->visuals[k];
				CLIENT_WU32(client, visual->id);
				CLIENT_WU8(client, visual->class);
				CLIENT_WU8(client, visual->bpp);
				CLIENT_WU16(client, visual->colormap_entries);
				CLIENT_WU32(client, visual->red_mask);
				CLIENT_WU32(client, visual->green_mask);
				CLIENT_WU32(client, visual->blue_mask);
				CLIENT_WRITE(client, NULL, 4);
			}
		}
	}
	client->state = CLIENT_CONNECTED;
	return 1;
}

static int
process_connected(struct xsrv *xsrv, struct client *client)
{
	int ret;

	if (!client->has_req)
	{
		if (ringbuf_read_size(&client->rbuf) < 4)
			return 0;
		client->request.opcode = CLIENT_RU8(client);
		client->request.detail = CLIENT_RU8(client);
		client->request.length = CLIENT_RU16(client);
		client->request.sequence = ++client->sequence;
		if (!client->request.length)
			return CLIENT_BAD_LENGTH(xsrv, client, &client->request);
		client->has_req = 1;
	}
	if (ringbuf_read_size(&client->rbuf) < (client->request.length - 1u) * 4)
		return 0;
	size_t next_pos = ringbuf_next_read_pos(&client->rbuf,
	                                        (client->request.length - 1u) * 4);
	client->has_req = 0;
#ifdef DEBUG_REQ_TIMING
	uint64_t s = nanotime();
	printf("handling opcode %u\n", client->request.opcode);
#endif
	req_handler_t handler = xsrv->req_handlers[client->request.opcode];
	if (handler)
	{
		ret = handler(xsrv, client, &client->request);
	}
	else
	{
		fprintf(stderr, "unknown opcode: %u\n", client->request.opcode);
		ret = CLIENT_BAD_REQUEST(xsrv, client, &client->request);
	}
#ifdef DEBUG_REQ_TIMING
	uint64_t e = nanotime();
	printf("handled in %" PRIu64 " us\n", (e - s) / 1000);
#endif
#if 0
	if (client->rbuf.read_pos != next_pos)
		fprintf(stderr, "not fully read\n");
#endif
	client->rbuf.read_pos = next_pos;
	return ret;
}

static int
process_closing(struct xsrv *xsrv, struct client *client)
{
	(void)xsrv;
	/* XXX add timeout */
	if (!ringbuf_read_size(&client->wbuf))
		client->state = CLIENT_CLOSED;
	return 0;
}

static int
process_closed(struct xsrv *xsrv, struct client *client)
{
	/* XXX */
	(void)xsrv;
	(void)client;
	return -1;
}

static int
client_process(struct xsrv *xsrv, struct client *client)
{
	switch (client->state)
	{
		case CLIENT_SETUP:
			return process_setup(xsrv, client);
		case CLIENT_CONNECTING:
			return process_connecting(xsrv, client);
		case CLIENT_CONNECTED:
			return process_connected(xsrv, client);
		case CLIENT_CLOSING:
			return process_closing(xsrv, client);
		case CLIENT_CLOSED:
			return process_closed(xsrv, client);
		default:
			fprintf(stderr, "unknown client state: %d\n", client->state);
			abort();
			return -1;
	}
}

int
client_run(struct xsrv *xsrv, struct client *client)
{
	int ret;

	if (client_recv(xsrv, client))
		return 1;
	do
	{
		ret = client_process(xsrv, client);
		if (ret == -1)
			return 1;
	} while (ret == 1);
	if (client_send(xsrv, client))
		return 1;
	return 0;
}

int
client_has_free_id(struct xsrv *xsrv, struct client *client, uint32_t id)
{
	struct object *object;

	if (id < client->id_base
	 || id >= client->id_base + client->id_mask)
		return 0;
	object = object_get(xsrv, id);
	if (object)
	{
		object_free(xsrv, object);
		return 0;
	}
	return 1;
}
