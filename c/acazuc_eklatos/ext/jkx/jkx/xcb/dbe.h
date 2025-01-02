#ifndef XCB_DBE_H
#define XCB_DBE_H

#include <xcb/xcb.h>

#define XCB_DBE_MAJOR_VERSION 1
#define XCB_DBE_MINOR_VERSION 0

#define XCB_DBE_QUERY_VERSION 0

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	uint16_t major_version;
	uint16_t minor_version;
} xcb_dbe_query_version_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_dbe_query_version_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint16_t major_version;
	uint16_t minor_version;
} xcb_dbe_query_version_reply_t;

xcb_dbe_query_version_cookie_t xcb_dbe_query_version(xcb_connection_t *conn,
                                                     uint16_t major_version,
                                                     uint16_t minor_version);
xcb_dbe_query_version_reply_t *xcb_dbe_query_version_reply(xcb_connection_t *conn,
                                                          xcb_dbe_query_version_cookie_t cookie,
                                                          xcb_generic_error_t **error);

extern xcb_extension_t xcb_dbe_id;

#endif
