#include <xcb/xcbext.h>
#include <xcb/render.h>
#include <xcb/int.h>

#include <stdlib.h>
#include <string.h>

xcb_extension_t xcb_render_id =
{
	"RENDER",
	0,
};

xcb_render_query_version_cookie_t xcb_render_query_version(xcb_connection_t *conn,
                                                           uint32_t client_major_version,
                                                           uint32_t client_minor_version)
{
	EXT_GET(render);
	uint16_t length = 3;
	REQ_INIT(render_query_version, extension->major_opcode, length);
	r->minor_opcode = XCB_RENDER_QUERY_VERSION;
	r->client_major_version = client_major_version;
	r->client_minor_version = client_minor_version;
	xcb_render_query_version_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_render_query_version_reply_t *xcb_render_query_version_reply(xcb_connection_t *conn,
                                                                 xcb_render_query_version_cookie_t cookie,
                                                                 xcb_generic_error_t **error)
{
	REPLY_ANSWER(render_query_version, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->major_version = answer->reply.data32[0];
	reply->minor_version = answer->reply.data32[1];
	return reply;
}

xcb_render_query_pict_formats_cookie_t xcb_render_query_pict_formats(xcb_connection_t *conn)
{
	EXT_GET(render);
	uint16_t length = 1;
	REQ_INIT(render_query_pict_formats, extension->major_opcode, length);
	r->minor_opcode = XCB_RENDER_QUERY_PICT_FORMATS;
	xcb_render_query_pict_formats_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_render_query_pict_formats_reply_t *xcb_render_query_pict_formats_reply(xcb_connection_t *conn,
                                                                           xcb_render_query_pict_formats_cookie_t cookie,
                                                                           xcb_generic_error_t **error)
{
	REPLY_ANSWER(priv_render_query_pict_formats, 0);
	reply->reply.response_type = answer->reply.response_type;
	reply->reply.pad0 = answer->reply.pad0;
	reply->reply.sequence = answer->reply.sequence;
	reply->reply.length = answer->reply.length;
	reply->reply.num_formats = answer->reply.data32[0];
	reply->reply.num_screens = answer->reply.data32[1];
	reply->reply.num_depths = answer->reply.data32[2];
	reply->reply.num_visuals = answer->reply.data32[3];
	reply->reply.num_subpixels = answer->reply.data32[4];
	reply->reply.pad1 = answer->reply.data32[5];
	if (reply->reply.length < reply->reply.num_formats * sizeof(*reply->formats) / 4)
		abort(); /* XXX */
	reply->formats = malloc(sizeof(*reply->formats) * reply->reply.num_formats);
	if (!reply->formats)
		abort(); /* XXX */
	memcpy(reply->formats, &answer->data[0], sizeof(*reply->formats) * reply->reply.num_formats);
	return &reply->reply;
}

xcb_render_pictforminfo_t *xcb_render_query_pict_formats_formats(xcb_render_query_pict_formats_reply_t *reply)
{
	xcb_priv_render_query_pict_formats_reply_t *priv = (xcb_priv_render_query_pict_formats_reply_t*)reply;
	return priv->formats;
}

int xcb_render_query_pict_formats_formats_length(xcb_render_query_pict_formats_reply_t *reply)
{
	return reply->num_formats;
}

xcb_void_cookie_t xcb_render_create_picture(xcb_connection_t *conn,
                                            xcb_render_picture_t pid,
                                            xcb_drawable_t drawable,
                                            xcb_render_pictformat_t format,
                                            uint32_t value_mask,
                                            const uint32_t *values)
{
	EXT_GET(render);
	size_t n = 0;
	for (size_t i = 0; i < 13; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	uint16_t length = 5 + n;
	REQ_INIT(render_create_picture, extension->major_opcode, length);
	r->minor_opcode = XCB_RENDER_CREATE_PICTURE;
	r->pid = pid;
	r->drawable = drawable;
	r->format = format;
	r->value_mask = value_mask;
	n = 0;
	for (size_t i = 0; i < 13; ++i)
	{
		if (value_mask & (1 << i))
			buf_write(&conn->wbuf, &values[n++], 4);
	}
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_render_create_picture_checked(xcb_connection_t *conn,
                                                    xcb_render_picture_t pid,
                                                    xcb_drawable_t drawable,
                                                    xcb_render_pictformat_t format,
                                                    uint32_t value_mask,
                                                    const uint32_t *values)
{
	return checked_request(conn,
		xcb_render_create_picture(conn,
		                          pid,
		                          drawable,
		                          format,
		                          value_mask,
		                          values));
}

xcb_void_cookie_t xcb_render_fill_rectangles(xcb_connection_t *conn,
                                             uint8_t op,
                                             xcb_render_picture_t dst,
                                             xcb_render_color_t color,
                                             uint32_t rectangles_len,
                                             const xcb_rectangle_t *rectangles)
{
	EXT_GET(render);
	uint16_t length = 5 + 2 * rectangles_len;
	REQ_INIT(render_fill_rectangles, extension->major_opcode, length);
	r->minor_opcode = XCB_RENDER_FILL_RECTANGLES;
	r->op = op;
	r->pad0[0] = 0;
	r->pad0[1] = 0;
	r->pad0[2] = 0;
	r->dst = dst;
	r->color = color;
	buf_write(&conn->wbuf, rectangles, 8 * rectangles_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_render_fill_rectangles_checked(xcb_connection_t *conn,
                                                     uint8_t op,
                                                     xcb_render_picture_t dst,
                                                     xcb_render_color_t color,
                                                     uint32_t rectangles_len,
                                                     const xcb_rectangle_t *rectangles)
{
	return checked_request(conn,
		xcb_render_fill_rectangles(conn,
		                           op,
		                           dst,
		                           color,
		                           rectangles_len,
		                           rectangles));
}

xcb_void_cookie_t xcb_render_composite(xcb_connection_t *conn,
                                       uint8_t op,
                                       xcb_render_picture_t src,
                                       xcb_render_picture_t mask,
                                       xcb_render_picture_t dst,
                                       int16_t src_x,
                                       int16_t src_y,
                                       int16_t mask_x,
                                       int16_t mask_y,
                                       int16_t dst_x,
                                       int16_t dst_y,
                                       uint16_t width,
                                       uint16_t height)
{
	EXT_GET(render);
	uint16_t length = 9;
	REQ_INIT(render_composite, extension->major_opcode, length);
	r->minor_opcode = XCB_RENDER_COMPOSITE;
	r->op = op;
	r->pad0[0] = 0;
	r->pad0[1] = 0;
	r->pad0[2] = 0;
	r->src = src;
	r->mask = mask;
	r->dst = dst;
	r->src_x = src_x;
	r->src_y = src_y;
	r->mask_x = mask_x;
	r->mask_y = mask_y;
	r->dst_x = dst_x;
	r->dst_y = dst_y;
	r->width = width;
	r->height = height;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_render_composite_checked(xcb_connection_t *conn,
                                               uint8_t op,
                                               xcb_render_picture_t src,
                                               xcb_render_picture_t mask,
                                               xcb_render_picture_t dst,
                                               int16_t src_x,
                                               int16_t src_y,
                                               int16_t mask_x,
                                               int16_t mask_y,
                                               int16_t dst_x,
                                               int16_t dst_y,
                                               uint16_t width,
                                               uint16_t height)
{
	return checked_request(conn,
		xcb_render_composite(conn,
		                     op,
		                     src,
		                     mask,
		                     dst,
		                     src_x,
		                     src_y,
		                     mask_x,
		                     mask_y,
		                     dst_x,
		                     dst_y,
		                     width,
		                     height));
}
