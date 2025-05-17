#include <xcb/xcbext.h>
#include <xcb/glx.h>
#include <xcb/int.h>

#include <stdlib.h>
#include <string.h>

xcb_extension_t
xcb_glx_id =
{
	"GLX",
	0,
};

xcb_void_cookie_t
xcb_glx_render(xcb_connection_t *conn,
               xcb_glx_context_tag_t tag,
               uint32_t data_len,
               const uint8_t *data)
{
	uint16_t length;

	length = 2 + (data_len + 3) / 4;
	EXT_GET(glx);
	REQ_INIT(glx_render, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_RENDER;
	r->context_tag = tag;
	buf_write(&conn->wbuf, data, data_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_glx_render_checked(xcb_connection_t *conn,
                       xcb_glx_context_tag_t tag,
                       uint32_t data_len,
                       const uint8_t *data)
{
	return checked_request(conn,
	                       xcb_glx_render(conn,
	                                      tag,
	                                      data_len,
	                                      data));
}

xcb_void_cookie_t
xcb_glx_render_large(xcb_connection_t *conn,
                     xcb_glx_context_tag_t tag,
                     uint16_t request_num,
                     uint16_t request_total,
                     uint32_t data_len,
                     const uint8_t *data)
{
	uint16_t length;

	length = 4 + (data_len + 3) / 4;
	EXT_GET(glx);
	REQ_INIT(glx_render_large, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_RENDER_LARGE;
	r->context_tag = tag;
	r->request_num = request_num;
	r->request_total = request_total;
	r->data_len = data_len;
	buf_write(&conn->wbuf, data, data_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_glx_render_large_checked(xcb_connection_t *conn,
                             xcb_glx_context_tag_t tag,
                             uint16_t request_num,
                             uint16_t request_total,
                             uint32_t data_len,
                             const uint8_t *data)
{
	return checked_request(conn,
	                       xcb_glx_render_large(conn,
	                                            tag,
	                                            request_num,
	                                            request_total,
	                                            data_len,
	                                            data));
}

xcb_void_cookie_t
xcb_glx_create_context(xcb_connection_t *conn,
                       xcb_glx_context_t context,
                       xcb_visualid_t visual,
                       uint32_t screen,
                       xcb_glx_context_t share_list,
                       uint8_t is_direct)
{
	uint16_t length = 6;

	EXT_GET(glx);
	REQ_INIT(glx_create_context, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_CREATE_CONTEXT;
	r->context = context;
	r->visual = visual;
	r->screen = screen;
	r->share_list = share_list;
	r->is_direct = is_direct;
	r->pad0[0] = 0;
	r->pad0[1] = 0;
	r->pad0[2] = 0;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_glx_create_context_checked(xcb_connection_t *conn,
                               xcb_glx_context_t context,
                               xcb_visualid_t visual,
                               uint32_t screen,
                               xcb_glx_context_t share_list,
                               uint8_t is_direct)
{
	return checked_request(conn,
		xcb_glx_create_context(conn,
		                       context,
		                       visual,
		                       screen,
		                       share_list,
		                       is_direct));
}

xcb_void_cookie_t
xcb_glx_destroy_context(xcb_connection_t *conn,
                        xcb_glx_context_t context)
{
	uint16_t length = 2;

	EXT_GET(glx);
	REQ_INIT(glx_destroy_context, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_DESTROY_CONTEXT;
	r->context = context;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_glx_destroy_context_checked(xcb_connection_t *conn,
                                xcb_glx_context_t context)
{
	return checked_request(conn,
		xcb_glx_destroy_context(conn,
		                        context));
}

xcb_glx_make_current_cookie_t
xcb_glx_make_current(xcb_connection_t *conn,
                     xcb_glx_drawable_t drawable,
                     xcb_glx_context_t context,
                     xcb_glx_context_tag_t tag)
{
	xcb_glx_make_current_cookie_t cookie;
	uint16_t length = 4;

	EXT_GET(glx);
	REQ_INIT(glx_make_current, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_MAKE_CURRENT;
	r->drawable = drawable;
	r->context = context;
	r->old_context_tag = tag;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_glx_make_current_reply_t *
xcb_glx_make_current_reply(xcb_connection_t *conn,
                           xcb_glx_make_current_cookie_t cookie,
                           xcb_generic_error_t **error)
{
	REPLY_ANSWER(glx_make_current, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->context_tag = answer->reply.data32[0];
	return reply;
}

xcb_glx_is_direct_cookie_t
xcb_glx_is_direct(xcb_connection_t *conn,
                  xcb_glx_context_t context)
{
	xcb_glx_is_direct_cookie_t cookie;
	uint16_t length = 2;

	EXT_GET(glx);
	REQ_INIT(glx_is_direct, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_IS_DIRECT;
	r->context = context;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_glx_is_direct_reply_t *
xcb_glx_is_direct_reply(xcb_connection_t *conn,
                        xcb_glx_is_direct_cookie_t cookie,
                        xcb_generic_error_t **error)
{
	REPLY_ANSWER(glx_is_direct, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->is_direct = answer->reply.data8[0];
	reply->pad1[0] = 0;
	reply->pad1[1] = 0;
	reply->pad1[2] = 0;
	return reply;
}

xcb_glx_query_version_cookie_t
xcb_glx_query_version(xcb_connection_t *conn,
                      uint32_t client_major_version,
                      uint32_t client_minor_version)
{
	xcb_glx_query_version_cookie_t cookie;
	uint16_t length = 3;

	EXT_GET(glx);
	REQ_INIT(glx_query_version, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_QUERY_VERSION;
	r->major_version = client_major_version;
	r->minor_version = client_minor_version;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_glx_query_version_reply_t *
xcb_glx_query_version_reply(xcb_connection_t *conn,
                            xcb_glx_query_version_cookie_t cookie,
                            xcb_generic_error_t **error)
{
	REPLY_ANSWER(glx_query_version, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->major_version = answer->reply.data32[0];
	reply->minor_version = answer->reply.data32[1];
	return reply;
}

xcb_void_cookie_t
xcb_glx_wait_gl(xcb_connection_t *conn,
                xcb_glx_context_tag_t tag)
{
	uint16_t length = 2;

	EXT_GET(glx);
	REQ_INIT(glx_wait_gl, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_WAIT_GL;
	r->context_tag = tag;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_glx_wait_gl_checked(xcb_connection_t *conn,
                        xcb_glx_context_tag_t tag)
{
	return checked_request(conn,
	                       xcb_glx_wait_gl(conn,
	                                       tag));
}

xcb_void_cookie_t
xcb_glx_wait_x(xcb_connection_t *conn,
               xcb_glx_context_tag_t tag)
{
	uint16_t length = 2;

	EXT_GET(glx);
	REQ_INIT(glx_wait_x, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_WAIT_X;
	r->context_tag = tag;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_glx_wait_x_checked(xcb_connection_t *conn,
                       xcb_glx_context_tag_t tag)
{
	return checked_request(conn,
	                       xcb_glx_wait_x(conn,
	                                      tag));
}

xcb_void_cookie_t
xcb_glx_copy_context(xcb_connection_t *conn,
                     xcb_glx_context_t src,
                     xcb_glx_context_t dest,
                     uint32_t mask,
                     xcb_glx_context_tag_t src_context_tag)
{
	uint16_t length = 5;

	EXT_GET(glx);
	REQ_INIT(glx_copy_context, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_COPY_CONTEXT;
	r->src = src;
	r->dest = dest;
	r->mask = mask;
	r->src_context_tag = src_context_tag;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_glx_copy_context_checked(xcb_connection_t *conn,
                             xcb_glx_context_t src,
                             xcb_glx_context_t dest,
                             uint32_t mask,
                             xcb_glx_context_tag_t src_context_tag)
{
	return checked_request(conn,
	                       xcb_glx_copy_context(conn,
	                                            src,
	                                            dest,
	                                            mask,
	                                            src_context_tag));
}

xcb_void_cookie_t
xcb_glx_swap_buffers(xcb_connection_t *conn,
                     xcb_glx_context_tag_t tag,
                     xcb_glx_drawable_t drawable)
{
	uint16_t length = 3;

	EXT_GET(glx);
	REQ_INIT(glx_swap_buffers, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_SWAP_BUFFERS;
	r->context_tag = tag;
	r->drawable = drawable;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_glx_swap_buffers_checked(xcb_connection_t *conn,
                             xcb_glx_context_tag_t tag,
                             xcb_glx_drawable_t drawable)
{
	return checked_request(conn,
	                       xcb_glx_swap_buffers(conn,
	                                            tag,
	                                            drawable));
}

xcb_glx_query_extensions_string_cookie_t
xcb_glx_query_extensions_string(xcb_connection_t *conn,
                                uint32_t screen)
{
	xcb_glx_query_extensions_string_cookie_t cookie;
	uint16_t length = 2;

	EXT_GET(glx);
	REQ_INIT(glx_query_extensions_string, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_QUERY_EXTENSIONS_STRING;
	r->screen = screen;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_glx_query_extensions_string_reply_t *
xcb_glx_query_extensions_string_reply(xcb_connection_t *conn,
                                      xcb_glx_query_extensions_string_cookie_t cookie,
                                      xcb_generic_error_t **error)
{
	REPLY_ANSWER(glx_query_extensions_string, answer->reply.length * 4);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->pad1[0] = 0;
	reply->pad1[1] = 0;
	reply->pad1[2] = 0;
	reply->pad1[3] = 0;
	reply->n = answer->reply.data32[1];
	for (size_t i = 0; i < 16; ++i)
		reply->pad2[i] = 0;
	memcpy(&reply[1], answer->data, answer->reply.length * 4);
	return reply;
}

xcb_glx_query_server_string_cookie_t
xcb_glx_query_server_string(xcb_connection_t *conn,
                            uint32_t screen,
                            uint32_t name)
{
	xcb_glx_query_server_string_cookie_t cookie;
	uint16_t length = 3;

	EXT_GET(glx);
	REQ_INIT(glx_query_server_string, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_QUERY_SERVER_STRING;
	r->screen = screen;
	r->name = name;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_glx_query_server_string_reply_t *
xcb_glx_query_server_string_reply(xcb_connection_t *conn,
                                  xcb_glx_query_server_string_cookie_t cookie,
                                  xcb_generic_error_t **error)
{
	REPLY_ANSWER(glx_query_server_string, answer->reply.length * 4);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->pad1[0] = 0;
	reply->pad1[1] = 0;
	reply->pad1[2] = 0;
	reply->pad1[3] = 0;
	reply->n = answer->reply.data32[1];
	for (size_t i = 0; i < 16; ++i)
		reply->pad2[i] = 0;
	memcpy(&reply[1], answer->data, answer->reply.length * 4);
	return reply;
}

xcb_glx_get_fb_configs_cookie_t
xcb_glx_get_fb_configs(xcb_connection_t *conn,
                       uint32_t screen)
{
	xcb_glx_get_fb_configs_cookie_t cookie;;
	uint16_t length = 2;

	EXT_GET(glx);
	REQ_INIT(glx_get_fb_configs, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_GET_FB_CONFIGS;
	r->screen = screen;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_glx_get_fb_configs_reply_t *
xcb_glx_get_fb_configs_reply(xcb_connection_t *conn,
                             xcb_glx_get_fb_configs_cookie_t cookie,
                             xcb_generic_error_t **error)
{
	REPLY_ANSWER(glx_get_fb_configs, answer->reply.length * 4);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->num_FB_configs = answer->reply.data32[0];
	reply->num_properties = answer->reply.data32[1];
	memcpy(&reply[1], answer->data, answer->reply.length * 4);
	return reply;
}

xcb_glx_make_context_current_cookie_t
xcb_glx_make_context_current(xcb_connection_t *conn,
                             xcb_glx_context_tag_t old_context_tag,
                             xcb_glx_drawable_t drawable,
                             xcb_glx_drawable_t read_drawable,
                             xcb_glx_context_t context)
{
	xcb_glx_make_context_current_cookie_t cookie;
	uint16_t length = 5;

	EXT_GET(glx);
	REQ_INIT(glx_make_context_current, extension->major_opcode, length);
	r->minor_opcode = XCB_GLX_MAKE_CONTEXT_CURRENT;
	r->old_context_tag = old_context_tag;
	r->drawable = drawable;
	r->read_drawable = read_drawable;
	r->context = context;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_glx_make_context_current_reply_t *
xcb_glx_make_context_current_reply(xcb_connection_t *conn,
                                   xcb_glx_make_context_current_cookie_t cookie,
                                   xcb_generic_error_t **error)
{
	REPLY_ANSWER(glx_make_context_current, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->context_tag = answer->reply.data32[0];
	return reply;
}
