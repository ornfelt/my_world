#include <xcb/xcbext.h>
#include <xcb/int.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

static int conn_init(xcb_connection_t *conn,
                     int8_t endian,
                     uint16_t major,
                     uint16_t minor,
                     const char *protoname,
                     const char *protodata);
static int conn_init_reply(xcb_connection_t *conn);

static inline uint32_t
xcb_pad(uint32_t length)
{
	uint32_t n = length % 4;
	if (!n)
		return 0;
	return 4 - n;
}

xcb_connection_t *
xcb_connect(const char *display_name, int *screenp)
{
	if (!display_name)
	{
		display_name = getenv("DISPLAY");
		if (!display_name)
			return NULL;
	}
	xcb_connection_t *conn = calloc(sizeof(*conn), 1);
	if (!conn)
		return NULL;
	conn->display = strdup(display_name);
	if (!conn->display)
	{
		free(conn);
		return NULL;
	}
	TAILQ_INIT(&conn->answers);
	conn->sequence = 0;
	conn->setup = NULL;
	conn->fd = -1;
	buf_init(&conn->wbuf);
	buf_init(&conn->rbuf);
	if (!buf_reserve(&conn->rbuf, 1024 * 1024))
		goto err;
	if (!buf_reserve(&conn->wbuf, 1024 * 1024))
		goto err;
	char *host;
	int display;
	int screen;
	if (xcb_parse_display(display_name, &host, &display, &screen))
		goto err;
	if (screenp)
		*screenp = screen;
	if (host)
	{
		goto err;
	}
	else
	{
		conn->fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (conn->fd < 0)
			goto err;
		struct sockaddr_un sun;
		sun.sun_family = AF_UNIX;
		snprintf(sun.sun_path, sizeof(sun.sun_path), "/tmp/.X11-unix/X%d", display);
		if (connect(conn->fd, (struct sockaddr*)&sun, sizeof(sun)) == -1)
			goto err;
		int flags = fcntl(conn->fd, F_GETFL, 0);
		if (flags < 0)
			goto err;
		flags |= O_NONBLOCK;
		if (fcntl(conn->fd, F_SETFL, flags) == -1)
			goto err;
	}
	if (!conn_init(conn,
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	               'l',
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	               'B',
#else
#error "unknown endianness"
#endif
	               11, 0, "", ""))
		goto err;
	int ret;
	do
	{
		ret = conn_init_reply(conn);
	} while (ret == 0);
	if (ret == -1)
		goto err;
	return conn;

err:
	xcb_disconnect(conn);
	return NULL;
}

static void
setup_free(xcb_priv_setup_t *setup)
{
	if (!setup)
		return;
	if (setup->roots)
	{
		for (size_t i = 0; i < setup->setup.roots_len; ++i)
		{
			xcb_priv_screen_t *screen = &setup->roots[i];
			if (screen->allowed_depths)
			{
				for (size_t j = 0; j < screen->screen.allowed_depths_len; ++j)
					free(screen->allowed_depths[j].visuals);
				free(screen->allowed_depths);
			}
		}
		free(setup->roots);
	}
	free(setup->pixmap_formats);
	free(setup->vendor);
	free(setup);
}

void
xcb_disconnect(xcb_connection_t *conn)
{
	if (!conn)
		return;
	setup_free(conn->setup);
	free(conn->display);
	close(conn->fd);
	buf_destroy(&conn->rbuf);
	buf_destroy(&conn->wbuf);
	while (!TAILQ_EMPTY(&conn->answers))
	{
		xcb_generic_answer_t *answer = TAILQ_FIRST(&conn->answers);
		TAILQ_REMOVE(&conn->answers, answer, chain);
		free(answer);
	}
	free(conn);
}

int
xcb_parse_display(const char *name,
                  char **host,
                  int *display,
                  int *screenp)
{
	if (!name)
	{
		name = getenv("DISPLAY");
		if (!name)
			return 1;
	}
	char *semi = strrchr(name, ':');
	if (!semi)
		return 1;
	if (semi == name)
	{
		*host = NULL;
	}
	else
	{
		*host = strndup(name, semi - name);
		if (!*host)
			return 1;
	}
	char *endptr;
	errno = 0;
	*display = strtoul(semi + 1, &endptr, 10);
	if (errno || (*endptr != '\0' && *endptr != '.'))
	{
		free(*host);
		return 1;
	}
	if (*endptr)
	{
		long screen = strtoul(endptr + 1, &endptr, 10);
		if (errno || *endptr)
		{
			free(*host);
			return 1;
		}
		if (screenp)
			*screenp = screen;
	}
	else
	{
		if (screenp)
			*screenp = 0;
	}
	return 0;
}

uint32_t
xcb_generate_id(xcb_connection_t *conn)
{
	/* XXX implement bitmap ? */
	return conn->res_id++;
}

static void
process_input(xcb_connection_t *conn)
{
	while (buf_remaining(&conn->rbuf) >= 32)
	{
		uint8_t response_type;
		buf_ru8(&conn->rbuf, &response_type);
		switch (response_type)
		{
			case 0:
			{
				xcb_generic_answer_t *answer = malloc(sizeof(*answer));
				if (!answer)
				{
					buf_rrb(&conn->rbuf, 1);
					return;
				}
				answer->error.response_type = response_type;
				buf_read(&conn->rbuf, &answer->error.error_code, 31);
				buf_rrst(&conn->rbuf);
				TAILQ_INSERT_TAIL(&conn->answers, answer, chain);
				break;
			}
			case 1:
			{
				uint8_t pad0;
				uint16_t sequence;
				uint32_t length;
				buf_ru8(&conn->rbuf, &pad0);
				buf_ru16(&conn->rbuf, &sequence);
				buf_ru32(&conn->rbuf, &length);
				if (buf_remaining(&conn->rbuf) < (length + 6) * 4)
				{
					buf_rrb(&conn->rbuf, 8);
					return;
				}
				xcb_generic_answer_t *answer = malloc(sizeof(*answer) + length * 4);
				if (!answer)
				{
					buf_rrb(&conn->rbuf, 1);
					return;
				}
				answer->reply.response_type = response_type;
				answer->reply.pad0 = pad0;
				answer->reply.sequence = sequence;
				answer->reply.length = length;
				buf_read(&conn->rbuf, &answer->reply.data8, 24 + length * 4);
				buf_rrst(&conn->rbuf);
				TAILQ_INSERT_TAIL(&conn->answers, answer, chain);
				break;
			}
			default:
			{
				xcb_generic_answer_t *answer = malloc(sizeof(*answer));
				if (!answer)
				{
					buf_rrb(&conn->rbuf, 1);
					return;
				}
				answer->event.response_type = response_type;
				buf_read(&conn->rbuf, &answer->event.pad0, 31);
				buf_rrst(&conn->rbuf);
				TAILQ_INSERT_TAIL(&conn->answers, answer, chain);
				break;
			}
		}
	}
}

static int
xcb_wait(xcb_connection_t *conn, int wr)
{
	fd_set fds;
	int ret;

	FD_ZERO(&fds);
	FD_SET(conn->fd, &fds);
	while (1)
	{
		ret = select(conn->fd + 1,
		             wr ? NULL : &fds,
		             wr ? &fds : NULL,
		             NULL,
		             NULL);
		if (ret == -1)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (ret)
			break;
	}
	return 1;
}

int
xcb_recv(xcb_connection_t *conn, int wait)
{
	int ret;

	if (wait)
	{
		ret = xcb_flush(conn);
		if (ret <= 0)
			return ret;
		ret = xcb_wait(conn, 0);
		if (ret <= 0)
			return ret;
	}
	ret = buf_recv(&conn->rbuf, conn->fd);
	if (ret < 0)
	{
		if (errno != EAGAIN)
			abort(); /* XXX */
		return ret;
	}
	if (!ret)
		abort(); /* XXX */
	process_input(conn);
	return ret;
}

int
xcb_send(xcb_connection_t *conn)
{
	int ret;

	ret = buf_send(&conn->wbuf, conn->fd);
	if (ret < 0 && errno != EAGAIN)
		abort(); /* XXX */
	return ret;
}

const xcb_query_extension_reply_t *
xcb_get_extension_data(xcb_connection_t *conn, xcb_extension_t *ext)
{
	if (ext->global_id)
	{
		if ((size_t)ext->global_id > conn->ext_cache_count)
			return NULL;
		return conn->ext_cache[ext->global_id - 1];
	}
	xcb_query_extension_cookie_t cookie;
	cookie = xcb_query_extension(conn, strlen(ext->name), ext->name);
	xcb_generic_error_t *error = NULL;
	xcb_query_extension_reply_t *reply;
	reply = xcb_query_extension_reply(conn, cookie, &error);
	if (error)
	{
		free(error);
		return NULL;
	}
	if (!reply)
		return NULL;
	xcb_query_extension_reply_t **replies = realloc(conn->ext_cache, sizeof(*replies) * (conn->ext_cache_count + 1));
	if (!replies)
	{
		free(reply);
		return NULL;
	}
	replies[conn->ext_cache_count] = reply;
	conn->ext_cache = replies;
	ext->global_id = ++conn->ext_cache_count;
	return reply;
}

int
xcb_flush(xcb_connection_t *conn)
{
	int ret;

	while (buf_remaining(&conn->wbuf))
	{
		ret = xcb_send(conn);
		if (ret == -1)
		{
			if (errno == EAGAIN)
			{
				ret = xcb_wait(conn, 1);
				if (ret < 0)
					return ret;
				continue;
			}
			if (errno == EINTR)
				continue;
			return -1;
		}
	}
	return 1;
}

xcb_generic_event_t *
xcb_copy_event(xcb_generic_event_t *event)
{
	xcb_generic_event_t *dup;

	dup = malloc(sizeof(*dup));
	if (!dup)
		return NULL;
	memcpy(dup, event, sizeof(*dup));
	return dup;
}

xcb_generic_error_t *
xcb_copy_error(xcb_generic_error_t *error)
{
	xcb_generic_error_t *dup;

	dup = malloc(sizeof(*dup));
	if (!dup)
		return NULL;
	memcpy(dup, error, sizeof(*dup));
	return dup;
}

xcb_generic_event_t *
xcb_wait_for_event(xcb_connection_t *conn)
{
	xcb_generic_answer_t *answer;

	while (1)
	{
		TAILQ_FOREACH(answer, &conn->answers, chain)
		{
			xcb_generic_event_t *ret;

			if (answer->response_type == 1)
				continue;
			ret = xcb_copy_event(&answer->event);
			TAILQ_REMOVE(&conn->answers, answer, chain);
			free(answer);
			return ret;
		}
		switch (xcb_recv(conn, 1))
		{
			case 0:
				return NULL;
			case -1:
				if (errno != EAGAIN)
					return NULL;
		}
	}
	return NULL;
}

xcb_generic_event_t *
xcb_poll_for_event(xcb_connection_t *conn)
{
	xcb_generic_answer_t *answer;

	while (1)
	{
		TAILQ_FOREACH(answer, &conn->answers, chain)
		{
			xcb_generic_event_t *ret;

			if (answer->response_type == 1)
				continue;
			ret = xcb_copy_event(&answer->event);
			TAILQ_REMOVE(&conn->answers, answer, chain);
			free(answer);
			return ret;
		}
		if (xcb_recv(conn, 0) <= 0)
			return NULL;
	}
}

xcb_generic_event_t *
xcb_peek_event(xcb_connection_t *conn)
{
	xcb_generic_answer_t *answer;

	while (1)
	{
		TAILQ_FOREACH(answer, &conn->answers, chain)
		{
			if (answer->response_type == 1)
				continue;
			return xcb_copy_event(&answer->event);
		}
		if (xcb_recv(conn, 0) <= 0)
			return NULL;
	}
}

int
xcb_pending_event(xcb_connection_t *conn)
{
	xcb_generic_answer_t *answer;

	TAILQ_FOREACH(answer, &conn->answers, chain)
	{
		if (answer->response_type != 1)
			return 1;
	}
	if (xcb_recv(conn, 0) <= 0)
		return 0;
	TAILQ_FOREACH(answer, &conn->answers, chain)
	{
		if (answer->response_type != 1)
			return 1;
	}
	return 0;
}

xcb_generic_answer_t *
xcb_find_answer(xcb_connection_t *conn,
                                             uint32_t sequence)
{
	xcb_generic_answer_t *answer;
	uint16_t searched = sequence & 0xFFFF;

	while (1)
	{
		TAILQ_FOREACH(answer, &conn->answers, chain)
		{
			if (answer->response_type < 2
			 && answer->sequence == searched)
			{
				TAILQ_REMOVE(&conn->answers, answer, chain);
				return answer;
			}
		}
		switch (xcb_recv(conn, 1))
		{
			case 0:
				return NULL;
			case -1:
				if (errno != EAGAIN)
					return NULL;
		}
	}
}

static int
conn_init(xcb_connection_t *conn,
          int8_t endian,
          uint16_t major,
          uint16_t minor,
          const char *protoname,
          const char *protodata)
{
	size_t protonamelen = strlen(protoname);
	size_t protodatalen = strlen(protodata);

	return buf_wi8(&conn->wbuf, endian)
	    && buf_wi8(&conn->wbuf, 0)
	    && buf_wu16(&conn->wbuf, major)
	    && buf_wu16(&conn->wbuf, minor)
	    && buf_wu16(&conn->wbuf, protonamelen)
	    && buf_wu16(&conn->wbuf, protodatalen)
	    && buf_wu16(&conn->wbuf, 0)
	    && buf_wstr(&conn->wbuf, protoname, protonamelen)
	    && buf_wpad(&conn->wbuf)
	    && buf_wstr(&conn->wbuf, protodata, protodatalen)
	    && buf_wpad(&conn->wbuf);
}

static int
conn_init_reply(xcb_connection_t *conn)
{
	switch (xcb_recv(conn, 1))
	{
		case 0:
			return -1;
		case -1:
			if (errno == EAGAIN)
				return 0;
			return -1;
	}
	uint8_t status;
	uint8_t pad0;
	uint16_t major;
	uint16_t minor;
	uint16_t length;
	xcb_priv_setup_t *setup = NULL;
	if (!buf_ru8(&conn->rbuf, &status))
		return 0;
	switch (status)
	{
		case 0:
			return -1;
		case 2:
			return -1;
		default:
			return -1;
		case 1:
			break;
	}
	if (buf_remaining(&conn->rbuf) < 7)
	{
		buf_rrb(&conn->rbuf, 1);
		return 0;
	}
	buf_ru8(&conn->rbuf, &pad0);
	buf_ru16(&conn->rbuf, &major);
	buf_ru16(&conn->rbuf, &minor);
	buf_ru16(&conn->rbuf, &length);
	if (length * 4 > buf_remaining(&conn->rbuf))
	{
		buf_rrb(&conn->rbuf, 8);
		return 0;
	}
	setup = calloc(sizeof(*setup), 1);
	if (!setup)
	{
		buf_rrb(&conn->rbuf, 8);
		return -1;
	}
	setup->setup.status = status;
	setup->setup.pad0 = pad0;
	setup->setup.protocol_major_version = major;
	setup->setup.protocol_minor_version = minor;
	setup->setup.length = length;
	if (!buf_read(&conn->rbuf, &setup->setup.release_number,
	              sizeof(setup->setup) - 8))
		goto err;
	setup->vendor = malloc(setup->setup.vendor_len + 1);
	if (!setup->vendor)
		goto err;
	if (!buf_read(&conn->rbuf, setup->vendor, setup->setup.vendor_len))
		goto err;
	setup->vendor[setup->setup.vendor_len] = 0;
	if (!buf_read(&conn->rbuf, NULL, xcb_pad(setup->setup.vendor_len)))
		goto err;
	setup->pixmap_formats = malloc(sizeof(*setup->pixmap_formats)
	                             * setup->setup.pixmap_formats_len);
	if (!setup->pixmap_formats)
		goto err;
	for (size_t i = 0; i < setup->setup.pixmap_formats_len; ++i)
	{
		xcb_format_t *format = &setup->pixmap_formats[i];
		if (!buf_read(&conn->rbuf, format, sizeof(*format)))
			goto err;
	}
	setup->roots = calloc(sizeof(*setup->roots) * setup->setup.roots_len, 1);
	if (!setup->roots)
		goto err;
	for (size_t i = 0; i < setup->setup.roots_len; ++i)
	{
		xcb_priv_screen_t *screen = &setup->roots[i];
		if (!buf_read(&conn->rbuf, &screen->screen,
		              sizeof(screen->screen)))
			goto err;
		if (!screen->screen.allowed_depths_len)
		{
			screen->allowed_depths = NULL;
			continue;
		}
		screen->allowed_depths = calloc(sizeof(*screen->allowed_depths)
		                              * screen->screen.allowed_depths_len, 1);
		if (!screen->allowed_depths)
			goto err;
		for (size_t j = 0; j < screen->screen.allowed_depths_len; ++j)
		{
			xcb_priv_depth_t *depth = &screen->allowed_depths[j];
			if (!buf_read(&conn->rbuf, &depth->depth,
			              sizeof(depth->depth)))
				goto err;
			if (!depth->depth.visuals_len)
			{
				depth->visuals = NULL;
				continue;
			}
			depth->visuals = malloc(sizeof(*depth->visuals)
			                      * depth->depth.visuals_len);
			if (!depth->visuals)
				goto err;
			for (size_t k = 0; k < depth->depth.visuals_len; ++k)
			{
				xcb_visualtype_t *visualtype = &depth->visuals[k];
				if (!buf_read(&conn->rbuf, visualtype,
				              sizeof(*visualtype)))
					goto err;
			}
		}
	}
	conn->setup = setup;
	conn->res_id = conn->setup->setup.resource_id_base;
	return 1;

err:
	setup_free(setup);
	return -1;
}

int
xcb_get_file_descriptor(xcb_connection_t *conn)
{
	return conn->fd;
}

const xcb_setup_t *
xcb_get_setup(xcb_connection_t *conn)
{
	return &conn->setup->setup;
}

char *
xcb_setup_vendor(const xcb_setup_t *setup)
{
	const xcb_priv_setup_t *priv_setup = (const xcb_priv_setup_t*)setup;
	return priv_setup->vendor;
}

xcb_generic_error_t *
xcb_request_check(xcb_connection_t *conn, xcb_void_cookie_t cookie)
{
	xcb_generic_answer_t *answer;
	uint16_t searched = cookie.sequence & 0xFFFF;

	while (1)
	{
		TAILQ_FOREACH(answer, &conn->answers, chain)
		{
			if (answer->sequence == ((searched + 1) & 0xFFFF)
			 && answer->response_type == 1)
			{
				TAILQ_REMOVE(&conn->answers, answer, chain);
				free(answer);
				return NULL;
			}
			/* XXX will cause problems at the moment of overflow */
			if (answer->sequence >= ((searched + 1) & 0xFFFF))
				return NULL;
			if (answer->sequence < searched)
				continue;
			if (answer->response_type)
				continue;
			xcb_generic_error_t *ret = xcb_copy_error(&answer->error);
			if (!ret)
				abort(); /* XXX */
			TAILQ_REMOVE(&conn->answers, answer, chain);
			free(answer);
			return ret;
		}
		switch (xcb_recv(conn, 1))
		{
			case 0:
				return NULL;
			case -1:
				if (errno != EAGAIN)
					return NULL;
		}
	}
}

int
xcb_setup_roots_length(const xcb_setup_t *setup)
{
	const xcb_priv_setup_t *priv_setup = (const xcb_priv_setup_t*)setup;
	return priv_setup->setup.roots_len;
}

xcb_screen_iterator_t
xcb_setup_roots_iterator(const xcb_setup_t *setup)
{
	const xcb_priv_setup_t *priv_setup = (const xcb_priv_setup_t*)setup;
	xcb_screen_iterator_t it;

	it.data = &priv_setup->roots[0].screen;
	it.rem = priv_setup->setup.roots_len;
	it.index = 0;
	return it;
}

void
xcb_screen_next(xcb_screen_iterator_t *it)
{
	it->data = (xcb_screen_t*)((xcb_priv_screen_t*)it->data + 1);
	it->rem--;
	it->index += sizeof(*it->data);
}

xcb_generic_iterator_t
xcb_screen_end(xcb_screen_iterator_t it)
{
	xcb_generic_iterator_t git;

	git.data = (xcb_screen_t*)((xcb_priv_screen_t*)it.data + it.rem);
	git.rem = 0;
	git.index = it.index + it.rem * sizeof(*it.data);
	return git;
}

int
xcb_setup_pixmap_formats_length(const xcb_setup_t *setup)
{
	const xcb_priv_setup_t *priv_setup = (const xcb_priv_setup_t*)setup;
	return priv_setup->setup.pixmap_formats_len;
}

xcb_format_iterator_t
xcb_setup_pixmap_formats_iterator(const xcb_setup_t *setup)
{
	const xcb_priv_setup_t *priv_setup = (const xcb_priv_setup_t*)setup;
	xcb_format_iterator_t it;

	it.data = priv_setup->pixmap_formats;
	it.rem = priv_setup->setup.pixmap_formats_len;
	it.index = 0;
	return it;
}

void
xcb_format_next(xcb_format_iterator_t *it)
{
	it->data++;
	it->rem--;
	it->index += sizeof(*it->data);
}

xcb_generic_iterator_t
xcb_format_end(xcb_format_iterator_t it)
{
	xcb_generic_iterator_t git;

	git.data = it.data + it.rem;
	git.rem = 0;
	git.index = it.index + it.rem * sizeof(*it.data);
	return git;
}

int
xcb_screen_allowed_depths_length(const xcb_screen_t *screen)
{
	const xcb_priv_screen_t *priv_screen = (const xcb_priv_screen_t*)screen;
	return priv_screen->screen.allowed_depths_len;
}

xcb_depth_iterator_t
xcb_screen_allowed_depths_iterator(const xcb_screen_t *screen)
{
	const xcb_priv_screen_t *priv_screen = (const xcb_priv_screen_t*)screen;
	xcb_depth_iterator_t it;

	it.data = &priv_screen->allowed_depths[0].depth;
	it.rem = priv_screen->screen.allowed_depths_len;
	it.index = 0;
	return it;
}

void
xcb_depth_next(xcb_depth_iterator_t *it)
{
	it->data = (xcb_depth_t*)((xcb_priv_depth_t*)it->data + 1);
	it->rem--;
	it->index += sizeof(*it->data);
}

xcb_generic_iterator_t
xcb_depth_end(xcb_depth_iterator_t it)
{
	xcb_generic_iterator_t git;

	git.data = (xcb_depth_t*)((xcb_priv_depth_t*)it.data + it.rem);
	git.rem = 0;
	git.index = it.index + it.rem * sizeof(*it.data);
	return git;
}

int
xcb_depth_visuals_length(const xcb_depth_t *depth)
{
	const xcb_priv_depth_t *priv_depth = (const xcb_priv_depth_t*)depth;
	return priv_depth->depth.visuals_len;
}

xcb_visualtype_iterator_t
xcb_depth_visuals_iterator(const xcb_depth_t *depth)
{
	const xcb_priv_depth_t *priv_depth = (const xcb_priv_depth_t*)depth;
	xcb_visualtype_iterator_t it;

	it.data = priv_depth->visuals;
	it.rem = priv_depth->depth.visuals_len;
	it.index = 0;
	return it;
}

void
xcb_visualtype_next(xcb_visualtype_iterator_t *it)
{
	it->data++;
	it->rem--;
	it->index += sizeof(*it->data);
}

xcb_generic_iterator_t
xcb_visualtype_end(xcb_visualtype_iterator_t it)
{
	xcb_generic_iterator_t git;

	git.data = &it.data[it.rem];
	git.rem = 0;
	git.index = it.index + it.rem * sizeof(*it.data);
	return git;
}

char *
xcb_str_name(const xcb_str_t *str)
{
	xcb_priv_str_t *priv_str = (xcb_priv_str_t*)str;
	return priv_str->name;
}

int
xcb_str_name_length(const xcb_str_t *str)
{
	return str->name_len;
}

void
xcb_str_next(xcb_str_iterator_t *it)
{
	it->data = (xcb_str_t*)((xcb_priv_str_t*)it->data + 1);
	it->rem--;
	it->index += sizeof(*it->data);
}

xcb_generic_iterator_t
xcb_str_end(xcb_str_iterator_t it)
{
	xcb_generic_iterator_t git;

	git.data = (xcb_str_t*)((xcb_priv_str_t*)it.data + it.rem);
	git.rem = 0;
	git.index = it.index + it.rem * sizeof(*it.data);
	return git;
}
