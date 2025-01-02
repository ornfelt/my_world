#ifndef XCB_XCBINT_H
#define XCB_XCBINT_H

#include <xcb/xcb.h>

#include "buf.h"

#define REQ_DECL(name) xcb_##name##_request_t *r = buf_grow(&(conn)->wbuf, sizeof(*r))
#define REQ_INIT(name, opcode, length) \
	buf_reserve(&conn->wbuf, conn->wbuf.size + (length) * 4); \
	REQ_DECL(name); \
	r->major_opcode = opcode; \
	r->length = length

#define REPLY_ANSWER(name, add) \
	xcb_generic_answer_t *answer = xcb_find_answer(conn, cookie.sequence); \
	if (answer->response_type == 0) \
	{ \
		*error = xcb_copy_error(&answer->error); \
		return NULL; \
	} \
	xcb_##name##_reply_t *reply = malloc(sizeof(*reply) + (add)); \
	if (!reply) \
		abort(); /* XXX */

#define EXT_GET(name) \
	const xcb_query_extension_reply_t *extension = xcb_get_extension_data(conn, &xcb_##name##_id); \
	if (!extension) \
		abort(); /* XXX */

typedef struct xcb_generic_answer
{
	TAILQ_ENTRY(xcb_generic_answer) chain;
	union
	{
		struct
		{
			uint8_t response_type;
			uint8_t error_code;
			uint16_t sequence;
		};
		xcb_generic_event_t event;
		xcb_generic_error_t error;
		xcb_generic_reply_t reply;
	};
	uint8_t data[];
} xcb_generic_answer_t;

typedef struct
{
	xcb_depth_t depth;
	xcb_visualtype_t *visuals;
} xcb_priv_depth_t;

typedef struct
{
	xcb_screen_t screen;
	xcb_priv_depth_t *allowed_depths;
} xcb_priv_screen_t;

typedef struct
{
	xcb_setup_t setup;
	char *vendor;
	xcb_priv_screen_t *roots;
	xcb_format_t *pixmap_formats;
} xcb_priv_setup_t;

typedef struct
{
	xcb_str_t str;
	char *name;
} xcb_priv_str_t;

typedef struct
{
	xcb_list_extensions_reply_t reply;
	xcb_priv_str_t *names;
} xcb_priv_list_extensions_reply_t;

typedef struct
{
	xcb_list_fonts_reply_t reply;
	xcb_priv_str_t *names;
} xcb_priv_list_fonts_reply_t;

typedef struct
{
	xcb_get_keyboard_mapping_reply_t reply;
	uint32_t keysyms_len;
	xcb_keysym_t keysyms[];
} xcb_priv_get_keyboard_mapping_reply_t;

struct xcb_connection_t
{
	int fd;
	struct buf rbuf;
	struct buf wbuf;
	xcb_priv_setup_t *setup;
	uint32_t res_id;
	uint32_t sequence;
	char *display;
	xcb_query_extension_reply_t **ext_cache;
	uint32_t ext_cache_count;
	TAILQ_HEAD(, xcb_generic_answer) answers;
};

int xcb_recv(xcb_connection_t *conn, int wait);
int xcb_send(xcb_connection_t *conn);

xcb_generic_answer_t *xcb_find_answer(xcb_connection_t *conn,
                                      uint32_t sequence);
xcb_generic_error_t *xcb_copy_error(xcb_generic_error_t *error);
xcb_generic_event_t *xcb_copy_event(xcb_generic_event_t *event);

static inline xcb_void_cookie_t checked_request(xcb_connection_t *conn,
                                                xcb_void_cookie_t cookie)
{
	xcb_get_input_focus(conn);
	return cookie;
}

static inline xcb_void_cookie_t xcb_build_void_cookie(xcb_connection_t *conn)
{
	xcb_void_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

#endif
