#ifndef XCB_XCB_H
#define XCB_XCB_H

#include <sys/queue.h>

#include <stdint.h>

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	union
	{
		uint8_t data8[24];
		uint16_t data16[12];
		uint32_t data32[6];
	};
} xcb_generic_reply_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t pad[7];
} xcb_generic_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t error_code;
	uint16_t sequence;
	uint32_t resource_id;
	uint16_t minor_code;
	uint8_t major_code;
	uint8_t pad0;
	uint32_t pad[5];
} xcb_generic_error_t;

typedef struct
{
	uint32_t sequence;
} xcb_void_cookie_t;

typedef struct
{
	void *data;
	int rem;
	int index;
} xcb_generic_iterator_t;

typedef struct xcb_connection_t xcb_connection_t;
typedef struct xcb_extension_t xcb_extension_t;

#include <xcb/xproto.h>

xcb_connection_t *xcb_connect(const char *display_name, int *screenp);
void xcb_disconnect(xcb_connection_t *conn);

int xcb_parse_display(const char *name, char **host, int *display, int *screen);

int xcb_flush(xcb_connection_t *conn);
xcb_generic_event_t *xcb_wait_for_event(xcb_connection_t *conn);
xcb_generic_event_t *xcb_poll_for_event(xcb_connection_t *conn);
xcb_generic_event_t *xcb_peek_event(xcb_connection_t *conn);
int xcb_pending_event(xcb_connection_t *conn);

uint32_t xcb_generate_id(xcb_connection_t *conn);

int xcb_get_file_descriptor(xcb_connection_t *conn);
const xcb_setup_t *xcb_get_setup(xcb_connection_t *conn);
char *xcb_setup_vendor(const xcb_setup_t *setup);
xcb_generic_error_t *xcb_request_check(xcb_connection_t *conn,
                                       xcb_void_cookie_t cookie);

const xcb_query_extension_reply_t *xcb_get_extension_data(xcb_connection_t *conn,
                                                          xcb_extension_t *ext);

int xcb_setup_roots_length(const xcb_setup_t *setup);
xcb_screen_iterator_t xcb_setup_roots_iterator(const xcb_setup_t *setup);
void xcb_screen_next(xcb_screen_iterator_t *it);
xcb_generic_iterator_t xcb_screen_end(xcb_screen_iterator_t it);

int xcb_setup_pixmap_formats_length(const xcb_setup_t *setup);
xcb_format_iterator_t xcb_setup_pixmap_formats_iterator(const xcb_setup_t *setup);
void xcb_format_next(xcb_format_iterator_t *it);
xcb_generic_iterator_t xcb_format_end(xcb_format_iterator_t it);

int xcb_screen_allowed_depths_length(const xcb_screen_t *screen);
xcb_depth_iterator_t xcb_screen_allowed_depths_iterator(const xcb_screen_t *screen);
void xcb_depth_next(xcb_depth_iterator_t *it);
xcb_generic_iterator_t xcb_depth_end(xcb_depth_iterator_t it);

int xcb_depth_visuals_length(const xcb_depth_t *depth);
xcb_visualtype_iterator_t xcb_depth_visuals_iterator(const xcb_depth_t *depth);
void xcb_visualtype_next(xcb_visualtype_iterator_t *it);
xcb_generic_iterator_t xcb_visualtype_end(xcb_visualtype_iterator_t it);

char *xcb_str_name(const xcb_str_t *str);
int xcb_str_name_length(const xcb_str_t *str);
void xcb_str_next(xcb_str_iterator_t *it);
xcb_generic_iterator_t xcb_str_end(xcb_str_iterator_t it);

#endif
