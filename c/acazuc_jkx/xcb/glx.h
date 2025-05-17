#ifndef XCB_GLX_H
#define XCB_GLX_H

#include <xcb/xcb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XCB_GLX_MAJOR_VERSION 1
#define XCB_GLX_MINOR_VERSION 4

#define XCB_GLX_RENDER                  1
#define XCB_GLX_RENDER_LARGE            2
#define XCB_GLX_CREATE_CONTEXT          3
#define XCB_GLX_DESTROY_CONTEXT         4
#define XCB_GLX_MAKE_CURRENT            5
#define XCB_GLX_IS_DIRECT               6
#define XCB_GLX_QUERY_VERSION           7
#define XCB_GLX_WAIT_GL                 8
#define XCB_GLX_WAIT_X                  9
#define XCB_GLX_COPY_CONTEXT            10
#define XCB_GLX_SWAP_BUFFERS            11
#define XCB_GLX_QUERY_EXTENSIONS_STRING 18
#define XCB_GLX_QUERY_SERVER_STRING     19
#define XCB_GLX_GET_FB_CONFIGS          21
#define XCB_GLX_MAKE_CONTEXT_CURRENT    26

typedef uint32_t xcb_glx_context_tag_t;
typedef uint32_t xcb_glx_drawable_t;
typedef uint32_t xcb_glx_context_t;

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_tag_t context_tag;
} xcb_glx_render_request_t;

xcb_void_cookie_t xcb_glx_render(xcb_connection_t *conn,
                                 xcb_glx_context_tag_t tag,
                                 uint32_t data_len,
                                 const uint8_t *data);

xcb_void_cookie_t xcb_glx_render_checked(xcb_connection_t *conn,
                                         xcb_glx_context_tag_t tag,
                                         uint32_t data_len,
                                         const uint8_t *data);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_tag_t context_tag;
	uint16_t request_num;
	uint16_t request_total;
	uint32_t data_len;
} xcb_glx_render_large_request_t;

xcb_void_cookie_t xcb_glx_render_large(xcb_connection_t *conn,
                                       xcb_glx_context_tag_t tag,
                                       uint16_t request_num,
                                       uint16_t request_total,
                                       uint32_t data_len,
                                       const uint8_t *data);

xcb_void_cookie_t xcb_glx_render_large_checked(xcb_connection_t *conn,
                                               xcb_glx_context_tag_t tag,
                                               uint16_t request_num,
                                               uint16_t request_total,
                                               uint32_t data_len,
                                               const uint8_t *data);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_t context;
	xcb_visualid_t visual;
	uint32_t screen;
	xcb_glx_context_t share_list;
	uint8_t is_direct;
	uint8_t pad0[3];
} xcb_glx_create_context_request_t;

xcb_void_cookie_t xcb_glx_create_context(xcb_connection_t *conn,
                                         xcb_glx_context_t context,
                                         xcb_visualid_t visual,
                                         uint32_t screen,
                                         xcb_glx_context_t share_list,
                                         uint8_t is_direct);
xcb_void_cookie_t xcb_glx_create_context_checked(xcb_connection_t *conn,
                                                 xcb_glx_context_t context,
                                                 xcb_visualid_t visual,
                                                 uint32_t screen,
                                                 xcb_glx_context_t share_list,
                                                 uint8_t is_direct);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_t context;
} xcb_glx_destroy_context_request_t;

xcb_void_cookie_t xcb_glx_destroy_context(xcb_connection_t *conn,
                                          xcb_glx_context_t context);

xcb_void_cookie_t xcb_glx_destroy_context_checked(xcb_connection_t *conn,
                                                  xcb_glx_context_t context);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_drawable_t drawable;
	xcb_glx_context_t context;
	xcb_glx_context_tag_t old_context_tag;
} xcb_glx_make_current_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_glx_make_current_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	xcb_glx_context_tag_t context_tag;
} xcb_glx_make_current_reply_t;

xcb_glx_make_current_cookie_t xcb_glx_make_current(xcb_connection_t *conn,
                                                   xcb_glx_drawable_t drawable,
                                                   xcb_glx_context_t context,
                                                   xcb_glx_context_tag_t tag);
xcb_glx_make_current_reply_t *xcb_glx_make_current_reply(xcb_connection_t *conn,
                                                         xcb_glx_make_current_cookie_t cookie,
                                                         xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_t context;
} xcb_glx_is_direct_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_glx_is_direct_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint8_t is_direct;
	uint8_t pad1[3];
} xcb_glx_is_direct_reply_t;

xcb_glx_is_direct_cookie_t xcb_glx_is_direct(xcb_connection_t *conn,
                                             xcb_glx_context_t context);
xcb_glx_is_direct_reply_t *xcb_glx_is_direct_reply(xcb_connection_t *conn,
                                                   xcb_glx_is_direct_cookie_t cookie,
                                                   xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	uint32_t major_version;
	uint32_t minor_version;
} xcb_glx_query_version_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_glx_query_version_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint32_t major_version;
	uint32_t minor_version;
} xcb_glx_query_version_reply_t;

xcb_glx_query_version_cookie_t xcb_glx_query_version(xcb_connection_t *conn,
                                                     uint32_t client_major_version,
                                                     uint32_t client_minor_version);
xcb_glx_query_version_reply_t *xcb_glx_query_version_reply(xcb_connection_t *conn,
                                                           xcb_glx_query_version_cookie_t cookie,
                                                           xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_tag_t context_tag;
} xcb_glx_wait_gl_request_t;

xcb_void_cookie_t xcb_glx_wait_gl(xcb_connection_t *conn,
                                  xcb_glx_context_tag_t tag);

xcb_void_cookie_t xcb_glx_wait_gl_checked(xcb_connection_t *conn,
                                          xcb_glx_context_tag_t tag);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_tag_t context_tag;
} xcb_glx_wait_x_request_t;

xcb_void_cookie_t xcb_glx_wait_x(xcb_connection_t *conn,
                                 xcb_glx_context_tag_t tag);

xcb_void_cookie_t xcb_glx_wait_x_checked(xcb_connection_t *conn,
                                         xcb_glx_context_tag_t tag);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_t src;
	xcb_glx_context_t dest;
	uint32_t mask;
	xcb_glx_context_tag_t src_context_tag;
} xcb_glx_copy_context_request_t;

xcb_void_cookie_t xcb_glx_copy_context(xcb_connection_t *conn,
                                       xcb_glx_context_t src,
                                       xcb_glx_context_t dest,
                                       uint32_t mask,
                                       xcb_glx_context_tag_t src_context_tag);
xcb_void_cookie_t xcb_glx_copy_context_checked(xcb_connection_t *conn,
                                               xcb_glx_context_t src,
                                               xcb_glx_context_t dest,
                                               uint32_t mask,
                                               xcb_glx_context_tag_t src_context_tag);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_tag_t context_tag;
	xcb_glx_drawable_t drawable;
} xcb_glx_swap_buffers_request_t;

xcb_void_cookie_t xcb_glx_swap_buffers(xcb_connection_t *conn,
                                       xcb_glx_context_tag_t tag,
                                       xcb_glx_drawable_t drawable);

xcb_void_cookie_t xcb_glx_swap_buffers_checked(xcb_connection_t *conn,
                                               xcb_glx_context_tag_t tag,
                                               xcb_glx_drawable_t drawable);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	uint32_t screen;
} xcb_glx_query_extensions_string_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_glx_query_extensions_string_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint8_t pad1[4];
	uint32_t n;
	uint8_t pad2[16];
} xcb_glx_query_extensions_string_reply_t;

xcb_glx_query_extensions_string_cookie_t xcb_glx_query_extensions_string(xcb_connection_t *conn,
                                                                         uint32_t screen);
xcb_glx_query_extensions_string_reply_t *xcb_glx_query_extensions_string_reply(xcb_connection_t *conn,
                                                                               xcb_glx_query_extensions_string_cookie_t cookie,
                                                                               xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	uint32_t screen;
	uint32_t name;
} xcb_glx_query_server_string_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_glx_query_server_string_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint8_t pad1[4];
	uint32_t n;
	uint8_t pad2[16];
} xcb_glx_query_server_string_reply_t;

xcb_glx_query_server_string_cookie_t xcb_glx_query_server_string(xcb_connection_t *conn,
                                                                 uint32_t screen,
                                                                 uint32_t name);
xcb_glx_query_server_string_reply_t *xcb_glx_query_server_string_reply(xcb_connection_t *conn,
                                                                       xcb_glx_query_server_string_cookie_t cookie,
                                                                       xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	uint32_t screen;
} xcb_glx_get_fb_configs_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_glx_get_fb_configs_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint32_t num_FB_configs;
	uint32_t num_properties;
} xcb_glx_get_fb_configs_reply_t;

xcb_glx_get_fb_configs_cookie_t xcb_glx_get_fb_configs(xcb_connection_t *conn,
                                                       uint32_t screen);
xcb_glx_get_fb_configs_reply_t *xcb_glx_get_fb_configs_reply(xcb_connection_t *conn,
                                                             xcb_glx_get_fb_configs_cookie_t cookie,
                                                             xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_glx_context_tag_t old_context_tag;
	xcb_glx_drawable_t drawable;
	xcb_glx_drawable_t read_drawable;
	xcb_glx_context_t context;
} xcb_glx_make_context_current_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_glx_make_context_current_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	xcb_glx_context_tag_t context_tag;
} xcb_glx_make_context_current_reply_t;

xcb_glx_make_context_current_cookie_t xcb_glx_make_context_current(xcb_connection_t *conn,
                                                                   xcb_glx_context_tag_t old_context_tag,
                                                                   xcb_glx_drawable_t drawable,
                                                                   xcb_glx_drawable_t read_drawable,
                                                                   xcb_glx_context_t context);
xcb_glx_make_context_current_reply_t *xcb_glx_make_context_current_reply(xcb_connection_t *conn,
                                                                         xcb_glx_make_context_current_cookie_t cookie,
                                                                         xcb_generic_error_t **error);

extern xcb_extension_t xcb_glx_id;

#ifdef __cplusplus
}
#endif

#endif
