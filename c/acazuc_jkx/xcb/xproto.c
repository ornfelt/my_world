#include <xcb/int.h>

#include <stdlib.h>
#include <string.h>

static void
write_masked8(xcb_connection_t *conn, uint8_t value)
{
	buf_wu8(&conn->wbuf, value);
	buf_write(&conn->wbuf, NULL, 3);
}

static void
write_masked16(xcb_connection_t *conn, uint16_t value)
{
	buf_wu16(&conn->wbuf, value);
	buf_write(&conn->wbuf, NULL, 2);
}

static void
write_masked32(xcb_connection_t *conn, uint32_t value)
{
	buf_wu32(&conn->wbuf, value);
}

xcb_list_extensions_cookie_t
xcb_list_extensions(xcb_connection_t *conn)
{
	uint16_t length = 1;

	REQ_INIT(list_extensions, XCB_LIST_EXTENSIONS, length);
	r->pad0 = 0;
	xcb_list_extensions_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_list_extensions_reply_t *
xcb_list_extensions_reply(xcb_connection_t *conn,
                          xcb_list_extensions_cookie_t cookie,
                          xcb_generic_error_t **error)
{
	REPLY_ANSWER(priv_list_extensions, 0);
	reply->reply.response_type = answer->reply.response_type;
	reply->reply.names_len = answer->reply.pad0;
	reply->reply.sequence = answer->reply.sequence;
	reply->reply.length = answer->reply.length;
	reply->names = malloc(sizeof(*reply->names) * reply->reply.names_len);
	uint8_t *ptr = answer->data;
	for (size_t i = 0; i < reply->reply.names_len; ++i)
	{
		uint8_t length = *(ptr++);
		xcb_priv_str_t *priv_name = &reply->names[i];
		priv_name->str.name_len = length;
		priv_name->name = (char*)ptr;
		ptr += length;
	}
	return &reply->reply;
}

int
xcb_list_extensions_names_length(const xcb_list_extensions_reply_t *reply)
{
	return reply->names_len;
}

xcb_str_iterator_t
xcb_list_extensions_names_iterator(const xcb_list_extensions_reply_t *reply)
{
	xcb_priv_list_extensions_reply_t *priv_reply = (xcb_priv_list_extensions_reply_t*)reply;
	xcb_str_iterator_t it;

	it.data = &priv_reply->names->str;
	it.rem = reply->names_len;
	it.index = 0;
	return it;
}

xcb_query_extension_cookie_t
xcb_query_extension(xcb_connection_t *conn,
                    uint16_t name_len,
                    const char *name)
{
	xcb_query_extension_cookie_t cookie;
	uint16_t length = 2 + (name_len + 3) / 4;

	REQ_INIT(query_extension, XCB_QUERY_EXTENSION, length);
	r->pad0 = 0;
	r->name_len = name_len;
	r->pad1 = 0;
	buf_write(&conn->wbuf, name, name_len);
	buf_wpad(&conn->wbuf);
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_query_extension_reply_t *
xcb_query_extension_reply(xcb_connection_t *conn,
                          xcb_query_extension_cookie_t cookie,
                          xcb_generic_error_t **error)
{
	REPLY_ANSWER(query_extension, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = 0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->present = answer->reply.data8[0];
	reply->major_opcode = answer->reply.data8[1];
	reply->first_event = answer->reply.data8[2];
	reply->first_error = answer->reply.data8[3];
	return reply;
}

xcb_void_cookie_t
xcb_create_window(xcb_connection_t *conn,
                  uint8_t depth,
                  xcb_window_t wid,
                  xcb_window_t parent,
                  int16_t x,
                  int16_t y,
                  uint16_t width,
                  uint16_t height,
                  uint16_t border_width,
                  uint16_t _class,
                  xcb_visualid_t visual,
                  uint32_t value_mask,
                  const uint32_t *value_list)
{
	uint16_t length;
	size_t n = 0;

	for (size_t i = 0; i < 15; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	length = 8 + n;
	REQ_INIT(create_window, XCB_CREATE_WINDOW, length);
	r->depth = depth;
	r->wid = wid;
	r->parent = parent;
	r->x = x;
	r->y = y;
	r->width = width;
	r->height = height;
	r->border_width = border_width;
	r->_class = _class;
	r->visual = visual;
	r->value_mask = value_mask;
	n = 0;
	for (size_t i = 0; i < 15; ++i)
	{
		if (value_mask & (1 << i))
			buf_write(&conn->wbuf, &value_list[n++], 4);
	}
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t
xcb_create_window_checked(xcb_connection_t *conn,
                          uint8_t depth,
                          xcb_window_t wid,
                          xcb_window_t parent,
                          int16_t x,
                          int16_t y,
                          uint16_t width,
                          uint16_t height,
                          uint16_t border_width,
                          uint16_t _class,
                          xcb_visualid_t visual,
                          uint32_t value_mask,
                          const uint32_t *value_list)
{
	return checked_request(conn,
		xcb_create_window(conn,
		                  depth,
		                  wid,
		                  parent,
		                  x,
		                  y,
		                  width,
		                  height,
		                  border_width,
		                  _class,
		                  visual,
		                  value_mask,
		                  value_list));
}

xcb_void_cookie_t
xcb_destroy_window(xcb_connection_t *conn,
                   xcb_window_t window)
{
	uint16_t length = 2;

	REQ_INIT(destroy_window, XCB_DESTROY_WINDOW, length);
	r->pad0 = 0;
	r->window = window;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_destroy_window_checked(xcb_connection_t *conn,
                                             xcb_window_t window)
{
	return checked_request(conn, xcb_destroy_window(conn, window));
}

xcb_void_cookie_t xcb_destroy_subwindows(xcb_connection_t *conn,
                                         xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(destroy_subwindows, XCB_DESTROY_SUBWINDOWS, length);
	r->pad0 = 0;
	r->window = window;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_destroy_subwindows_checked(xcb_connection_t *conn,
                                                 xcb_window_t window)
{
	return checked_request(conn, xcb_destroy_subwindows(conn, window));
}

xcb_void_cookie_t xcb_map_window(xcb_connection_t *conn, 
                                 xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(map_window, XCB_MAP_WINDOW, length);
	r->pad0 = 0;
	r->window = window;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_map_window_checked(xcb_connection_t *conn,
                                         xcb_window_t window)
{
	return checked_request(conn, xcb_map_window(conn, window));
}

xcb_void_cookie_t xcb_unmap_window(xcb_connection_t *conn,
                                   xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(unmap_window, XCB_UNMAP_WINDOW, length);
	r->pad0 = 0;
	r->window = window;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_unmap_window_checked(xcb_connection_t *conn,
                                           xcb_window_t window)
{
	return checked_request(conn, xcb_unmap_window(conn, window));
}

xcb_void_cookie_t xcb_map_subwindows(xcb_connection_t *conn,
                                     xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(map_subwindows, XCB_MAP_SUBWINDOWS, length);
	r->pad0 = 0;
	r->window = window;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_map_subwindows_checked(xcb_connection_t *conn,
                                             xcb_window_t window)
{
	return checked_request(conn, xcb_map_subwindows(conn, window));
}

xcb_void_cookie_t xcb_unmap_subwindows(xcb_connection_t *conn,
                                       xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(unmap_subwindows, XCB_UNMAP_SUBWINDOWS, length);
	r->pad0 = 0;
	r->window = window;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_unmap_subwindows_checked(xcb_connection_t *conn,
                                               xcb_window_t window)
{
	return checked_request(conn, xcb_unmap_subwindows(conn, window));
}

xcb_intern_atom_cookie_t xcb_intern_atom(xcb_connection_t *conn,
                                         uint8_t only_if_exists,
                                         uint16_t name_len,
                                         const char *name)
{
	uint16_t length = 2 + (name_len + 3) / 4;
	REQ_INIT(intern_atom, XCB_INTERN_ATOM, length);
	r->only_if_exists = only_if_exists;
	r->name_len = name_len;
	r->pad0[0] = 0;
	r->pad0[1] = 0;
	buf_write(&conn->wbuf, name, name_len);
	buf_wpad(&conn->wbuf);
	xcb_intern_atom_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_intern_atom_reply_t *xcb_intern_atom_reply(xcb_connection_t *conn,
                                               xcb_intern_atom_cookie_t cookie,
                                               xcb_generic_error_t **error)
{
	REPLY_ANSWER(intern_atom, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->atom = answer->reply.data32[0];
	return reply;
}

xcb_get_atom_name_cookie_t xcb_get_atom_name(xcb_connection_t *conn,
                                             xcb_atom_t atom)
{
	uint16_t length = 2;
	REQ_INIT(get_atom_name, XCB_GET_ATOM_NAME, length);
	r->pad0 = 0;
	r->atom = atom;
	xcb_get_atom_name_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_atom_name_reply_t *xcb_get_atom_name_reply(xcb_connection_t *conn,
                                                   xcb_get_atom_name_cookie_t cookie,
                                                   xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_atom_name, answer->reply.length * 4);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->name_len = answer->reply.data16[0];
	memcpy(&reply[1], answer->data, answer->reply.length * 4);
	return reply;
}

char *xcb_get_atom_name_name(const xcb_get_atom_name_reply_t *reply)
{
	return (char*)&reply[1];
}

xcb_void_cookie_t xcb_change_property(xcb_connection_t *conn,
                                      xcb_prop_mode_t mode,
                                      xcb_window_t window,
                                      xcb_atom_t property,
                                      xcb_atom_t type,
                                      uint8_t format,
                                      uint32_t data_len,
                                      const void *data)
{
	uint32_t data_bytes = data_len * (format / 8);
	uint16_t length = 6 + (data_bytes + 3) / 4;
	REQ_INIT(change_property, XCB_CHANGE_PROPERTY, length);
	r->mode = mode;
	r->window = window;
	r->property = property;
	r->type = type;
	r->format = format;
	r->pad0[0] = 0;
	r->pad0[1] = 0;
	r->pad0[2] = 0;
	r->data_len = data_len;
	buf_write(&conn->wbuf, data, data_bytes);
	buf_wpad(&conn->wbuf);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_change_property_checked(xcb_connection_t *conn,
                                              xcb_prop_mode_t mode,
                                              xcb_window_t window,
                                              xcb_atom_t property,
                                              xcb_atom_t type,
                                              uint8_t format,
                                              uint32_t data_length,
                                              const void *data)
{
	return checked_request(conn,
		xcb_change_property(conn,
		                    mode,
		                    window,
		                    property,
		                    type,
		                    format,
		                    data_length,
		                    data));
}

xcb_void_cookie_t xcb_configure_window(xcb_connection_t *conn,
                                              xcb_window_t window,
                                              uint16_t value_mask,
                                              uint32_t *value_list)
{
	size_t n = 0;
	for (size_t i = 0; i < 7; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	uint16_t length = 3 + n;
	REQ_INIT(configure_window, XCB_CONFIGURE_WINDOW, length);
	r->pad0 = 0;
	r->window = window;
	r->value_mask = value_mask;
	r->pad1 = 0;
	n = 0;
	if (value_mask & XCB_CONFIG_WINDOW_X)
		write_masked16(conn, value_list[n++]);
	if (value_mask & XCB_CONFIG_WINDOW_Y)
		write_masked16(conn, value_list[n++]);
	if (value_mask & XCB_CONFIG_WINDOW_WIDTH)
		write_masked16(conn, value_list[n++]);
	if (value_mask & XCB_CONFIG_WINDOW_HEIGHT)
		write_masked16(conn, value_list[n++]);
	if (value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH)
		write_masked16(conn, value_list[n++]);
	if (value_mask & XCB_CONFIG_WINDOW_SIBLING)
		write_masked32(conn, value_list[n++]);
	if (value_mask & XCB_CONFIG_WINDOW_STACK_MODE)
		write_masked8(conn, value_list[n++]);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_configure_window_checked(xcb_connection_t *conn,
                                               xcb_window_t window,
                                               uint16_t value_mask,
                                               uint32_t *value_list)
{
	return checked_request(conn,
		xcb_configure_window(conn,
		                     window,
		                     value_mask,
		                     value_list));
}

xcb_void_cookie_t xcb_circulate_window(xcb_connection_t *conn,
                                       uint8_t direction,
                                       xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(circulate_window, XCB_CIRCULATE_WINDOW, length);
	r->direction = direction;
	r->window = window;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_circulate_window_checked(xcb_connection_t *conn,
                                               uint8_t direction,
                                               xcb_window_t window)
{
	return checked_request(conn,
		xcb_circulate_window(conn,
		                     direction,
		                     window));
}

xcb_void_cookie_t xcb_change_window_attributes(xcb_connection_t *conn,
                                               xcb_window_t window,
                                               uint32_t value_mask,
                                               uint32_t *values)
{
	size_t n = 0;
	for (size_t i = 0; i < 15; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	uint16_t length = 3 + n;
	REQ_INIT(change_window_attributes, XCB_CHANGE_WINDOW_ATTRIBUTES, length);
	r->pad0 = 0;
	r->window = window;
	r->value_mask = value_mask;
	n = 0;
	for (size_t i = 0; i < 15; ++i)
	{
		if (value_mask & (1 << i))
			buf_wu32(&conn->wbuf, values[n]);
	}
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_change_window_attributes_checked(xcb_connection_t *conn,
                                                       xcb_window_t window,
                                                       uint32_t value_mask,
                                                       uint32_t *values)
{
	return checked_request(conn,
		xcb_change_window_attributes(conn,
		                             window,
		                             value_mask,
		                             values));
}

xcb_get_window_attributes_cookie_t xcb_get_window_attributes(xcb_connection_t *conn,
                                                             xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(get_window_attributes, XCB_GET_WINDOW_ATTRIBUTES, length);
	r->pad0 = 0;
	r->window = window;
	xcb_get_window_attributes_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_window_attributes_reply_t *xcb_get_window_attributes_reply(xcb_connection_t *conn,
                                                                   xcb_get_window_attributes_cookie_t cookie,
                                                                   xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_window_attributes, 0);
	reply->response_type = answer->reply.response_type;
	reply->backing_store = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->visual = answer->reply.data32[0];
	reply->_class = answer->reply.data16[2];
	reply->bit_gravity = answer->reply.data8[6];
	reply->win_gravity = answer->reply.data8[7];
	reply->backing_planes = answer->reply.data32[2];
	reply->backing_pixel = answer->reply.data32[3];
	reply->save_under = answer->reply.data8[16];
	reply->map_is_installed = answer->reply.data8[17];
	reply->map_state = answer->reply.data8[18];
	reply->override_redirect = answer->reply.data8[19];
	reply->colormap = answer->reply.data32[5];
	reply->all_event_masks = *(uint32_t*)&answer->data[0];
	reply->your_event_mask = *(uint32_t*)&answer->data[4];
	reply->do_not_propagate_mask = *(uint32_t*)&answer->data[8];
	return reply;
}

xcb_get_input_focus_cookie_t xcb_get_input_focus(xcb_connection_t *conn)
{
	uint16_t length = 1;
	REQ_INIT(get_input_focus, XCB_GET_INPUT_FOCUS, length);
	r->pad0 = 0;
	xcb_get_input_focus_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_input_focus_reply_t *xcb_get_input_focus_reply(xcb_connection_t *conn,
                                                       xcb_get_input_focus_cookie_t cookie,
                                                       xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_input_focus, 0);
	reply->response_type = answer->reply.response_type;
	reply->revert_to = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->focus = answer->reply.data32[0];
	return reply;
}

xcb_void_cookie_t xcb_set_input_focus(xcb_connection_t *conn,
                                      uint8_t revert_to,
                                      xcb_window_t focus,
                                      xcb_timestamp_t time)
{
	uint16_t length = 3;
	REQ_INIT(set_input_focus, XCB_SET_INPUT_FOCUS, length);
	r->revert_to = revert_to;
	r->focus = focus;
	r->time = time;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_set_input_focus_checked(xcb_connection_t *conn,
                                              uint8_t revert_to,
                                              xcb_window_t focus,
                                              xcb_timestamp_t time)
{
	return checked_request(conn,
		xcb_set_input_focus(conn,
		                    revert_to,
		                    focus,
		                    time));
}

xcb_void_cookie_t xcb_create_gc(xcb_connection_t *conn,
                                xcb_gcontext_t cid,
                                xcb_drawable_t drawable,
                                uint32_t value_mask,
                                const uint32_t *values)
{
	size_t n = 0;
	for (size_t i = 0; i < 23; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	uint16_t length = 4 + n;
	REQ_INIT(create_gc, XCB_CREATE_GC, length);
	r->pad0 = 0;
	r->cid = cid;
	r->drawable = drawable;
	r->value_mask = value_mask;
	n = 0;
	if (value_mask & XCB_GC_FUNCTION)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_PLANE_MASK)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_FOREGROUND)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_BACKGROUND)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_LINE_WIDTH)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_LINE_STYLE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_CAP_STYLE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_JOIN_STYLE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_FILL_STYLE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_FILL_RULE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_TILE)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_STIPPLE)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_TILE_STIPPLE_ORIGIN_X)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_TILE_STIPPLE_ORIGIN_Y)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_FONT)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_SUBWINDOW_MODE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_GRAPHICS_EXPOSURES)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_CLIP_ORIGIN_X)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_CLIP_ORIGIN_Y)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_CLIP_MASK)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_DASH_OFFSET)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_DASH_LIST)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_ARC_MODE)
		write_masked8(conn, values[n++]);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_create_gc_checked(xcb_connection_t *conn,
                                        xcb_gcontext_t cid,
                                        xcb_drawable_t drawable,
                                        uint32_t value_mask,
                                        const uint32_t *values)
{
	return checked_request(conn,
		xcb_create_gc(conn,
		              cid,
		              drawable,
		              value_mask,
		              values));
}

xcb_void_cookie_t xcb_copy_gc(xcb_connection_t *conn, 
                              xcb_gcontext_t src_gc,
                              xcb_gcontext_t dst_gc,
                              uint32_t value_mask)
{
	uint16_t length = 4;
	REQ_INIT(copy_gc, XCB_COPY_GC, length);
	r->pad0 = 0;
	r->src_gc = src_gc;
	r->dst_gc = dst_gc;
	r->value_mask = value_mask;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_copy_gc_checked(xcb_connection_t *conn,
                                      xcb_gcontext_t src_gc,
                                      xcb_gcontext_t dst_gc,
                                      uint32_t value_mask)
{
	return checked_request(conn,
		xcb_copy_gc(conn,
		            src_gc,
		            dst_gc,
		            value_mask));
}

xcb_void_cookie_t xcb_free_gc(xcb_connection_t *conn, xcb_gcontext_t gc)
{
	uint16_t length = 2;
	REQ_INIT(free_gc, XCB_FREE_GC, length);
	r->pad0 = 0;
	r->gc = gc;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_free_gc_checked(xcb_connection_t *conn,
                                      xcb_gcontext_t gc)
{
	return checked_request(conn, xcb_free_gc(conn, gc));
}

xcb_void_cookie_t xcb_poly_point(xcb_connection_t *conn,
                                 uint8_t coordinate_mode,
                                 xcb_drawable_t drawable,
                                 xcb_gcontext_t gc,
                                 uint32_t points_len,
                                 const xcb_point_t *points)
{
	uint16_t length = 3 + points_len;
	REQ_INIT(poly_point, XCB_POLY_POINT, length);
	r->coordinate_mode = coordinate_mode;
	r->drawable = drawable;
	r->gc = gc;
	buf_write(&conn->wbuf, points, 4 * points_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_poly_point_checked(xcb_connection_t *conn,
                                         uint8_t coordinate_mode,
                                         xcb_drawable_t drawable,
                                         xcb_gcontext_t gc,
                                         uint32_t points_len,
                                         const xcb_point_t *points)
{
	return checked_request(conn,
		xcb_poly_point(conn,
		               coordinate_mode,
		               drawable,
		               gc,
		               points_len,
		               points));
}

xcb_void_cookie_t xcb_poly_line(xcb_connection_t *conn,
                                uint8_t coordinate_mode,
                                xcb_drawable_t drawable,
                                xcb_gcontext_t gc,
                                uint32_t points_len,
                                const xcb_point_t *points)
{
	uint16_t length = 3 + points_len;
	REQ_INIT(poly_line, XCB_POLY_LINE, length);
	r->coordinate_mode = coordinate_mode;
	r->drawable = drawable;
	r->gc = gc;
	buf_write(&conn->wbuf, points, 4 * points_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_poly_line_checked(xcb_connection_t *conn,
                                        uint8_t coordinate_mode,
                                        xcb_drawable_t drawable,
                                        xcb_gcontext_t gc,
                                        uint32_t points_len,
                                        const xcb_point_t *points)
{
	return checked_request(conn,
		xcb_poly_line(conn,
		              coordinate_mode,
		              drawable,
		              gc,
		              points_len,
		              points));
}

xcb_void_cookie_t xcb_poly_segment(xcb_connection_t *conn,
                                   xcb_drawable_t drawable,
                                   xcb_gcontext_t gc,
                                   uint32_t segments_len,
                                   const xcb_segment_t *segments)
{
	uint16_t length = 3 + 2 * segments_len;
	REQ_INIT(poly_segment, XCB_POLY_SEGMENT, length);
	r->pad0 = 0;
	r->drawable = drawable;
	r->gc = gc;
	buf_write(&conn->wbuf, segments, 8 * segments_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_poly_segment_checked(xcb_connection_t *conn,
                                           xcb_drawable_t drawable,
                                           xcb_gcontext_t gc,
                                           uint32_t segments_len,
                                           const xcb_segment_t *segments)
{
	return checked_request(conn,
		xcb_poly_segment(conn,
		                 drawable,
		                 gc,
		                 segments_len,
		                 segments));
}

xcb_void_cookie_t xcb_poly_rectangle(xcb_connection_t *conn,
                                     xcb_drawable_t drawable,
                                     xcb_gcontext_t gc,
                                     uint32_t rectangles_len,
                                     const xcb_rectangle_t *rectangles)
{
	uint16_t length = 3 + 2 * rectangles_len;
	REQ_INIT(poly_rectangle, XCB_POLY_RECTANGLE, length);
	r->pad0 = 0;
	r->drawable = drawable;
	r->gc = gc;
	buf_write(&conn->wbuf, rectangles, 8 * rectangles_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_poly_rectangle_checked(xcb_connection_t *conn,
                                             xcb_drawable_t drawable,
                                             xcb_gcontext_t gc,
                                             uint32_t rectangles_len,
                                             const xcb_rectangle_t *rectangles)
{
	return checked_request(conn,
		xcb_poly_rectangle(conn,
		                   drawable,
		                   gc,
		                   rectangles_len,
		                   rectangles));
}

xcb_void_cookie_t xcb_poly_arc(xcb_connection_t *conn,
                               xcb_drawable_t drawable, xcb_gcontext_t gc,
                               uint32_t arcs_len,
                               const xcb_arc_t *arcs)
{
	uint16_t length = 3 + 3 * arcs_len;
	REQ_INIT(poly_arc, XCB_POLY_ARC, length);
	r->pad0 = 0;
	r->drawable = drawable;
	r->gc = gc;
	buf_write(&conn->wbuf, arcs, 12 * arcs_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_poly_arc_checked(xcb_connection_t *conn,
                                       xcb_drawable_t drawable,
                                       xcb_gcontext_t gc,
                                       uint32_t arcs_len,
                                       const xcb_arc_t *arcs)
{
	return checked_request(conn,
		xcb_poly_arc(conn,
		             drawable,
		             gc,
		             arcs_len,
		             arcs));
}

xcb_void_cookie_t xcb_fill_poly(xcb_connection_t *conn,
                                xcb_drawable_t drawable,
                                xcb_gcontext_t gc,
                                uint8_t shape,
                                uint8_t coordinate_mode,
                                uint32_t points_len,
                                const xcb_point_t *points)
{
	uint16_t length = 4 + points_len;
	REQ_INIT(fill_poly, XCB_FILL_POLY, length);
	r->pad0 = 0;
	r->drawable = drawable;
	r->gc = gc;
	r->shape = shape;
	r->coordinate_mode = coordinate_mode;
	r->pad1 = 0;
	buf_write(&conn->wbuf, points, 4 * points_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_fill_poly_checked(xcb_connection_t *conn,
                                        xcb_drawable_t drawable,
                                        xcb_gcontext_t gc,
                                        uint8_t shape,
                                        uint8_t coordinate_mode,
                                        uint32_t points_len,
                                        const xcb_point_t *points)
{
	return checked_request(conn,
		xcb_fill_poly(conn,
		              drawable,
		              gc,
		              shape,
		              coordinate_mode,
		              points_len,
		              points));
}

xcb_void_cookie_t xcb_poly_fill_rectangle(xcb_connection_t *conn,
                                          xcb_drawable_t drawable,
                                          xcb_gcontext_t gc,
                                          uint32_t rectangles_len,
                                          const xcb_rectangle_t *rectangles)
{
	uint16_t length = 3 + 2 * rectangles_len;
	REQ_INIT(poly_fill_rectangle, XCB_POLY_FILL_RECTANGLE, length);
	r->pad0 = 0;
	r->drawable = drawable;
	r->gc = gc;
	buf_write(&conn->wbuf, rectangles, 8 * rectangles_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_poly_fill_rectangle_checked(xcb_connection_t *conn,
                                                  xcb_drawable_t drawable,
                                                  xcb_gcontext_t gc,
                                                  uint32_t rectangles_len,
                                                  const xcb_rectangle_t *rectangles)
{
	return checked_request(conn,
		xcb_poly_fill_rectangle(conn,
		                        drawable,
		                        gc,
		                        rectangles_len,
		                        rectangles));
}

xcb_void_cookie_t xcb_poly_fill_arc(xcb_connection_t *conn,
                                    xcb_drawable_t drawable,
                                    xcb_gcontext_t gc,
                                    uint32_t arcs_len,
                                    const xcb_arc_t *arcs)
{
	uint16_t length = 3 + 3 * arcs_len;
	REQ_INIT(poly_fill_arc, XCB_POLY_FILL_ARC, length);
	r->pad0 = 0;
	r->drawable = drawable;
	r->gc = gc;
	buf_write(&conn->wbuf, arcs, 12 * arcs_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_poly_fill_arc_checked(xcb_connection_t *conn,
                                            xcb_drawable_t drawable,
                                            xcb_gcontext_t gc,
                                            uint32_t arcs_len,
                                            const xcb_arc_t *arcs)
{
	return checked_request(conn,
		xcb_poly_fill_arc(conn,
		                  drawable,
		                  gc,
		                  arcs_len,
		                  arcs));
}

xcb_void_cookie_t xcb_open_font(xcb_connection_t *conn,
                                xcb_font_t fid,
                                uint16_t name_len,
                                const char *name)
{
	uint16_t length = 3 + (name_len + 3) / 4;
	REQ_INIT(open_font, XCB_OPEN_FONT, length);
	r->pad0 = 0;
	r->fid = fid;
	r->name_len = name_len;
	r->pad1 = 0;
	buf_write(&conn->wbuf, name, name_len);
	buf_wpad(&conn->wbuf);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_open_font_checked(xcb_connection_t *conn,
                                        xcb_font_t fid,
                                        uint16_t name_len,
                                        const char *name)
{
	return checked_request(conn,
		xcb_open_font(conn,
		              fid,
		              name_len,
		              name));
}

xcb_void_cookie_t xcb_close_font(xcb_connection_t *conn,
                                        xcb_font_t font)
{
	uint16_t length = 2;
	REQ_INIT(close_font, XCB_CLOSE_FONT, length);
	r->pad0 = 0;
	r->font = font;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_close_font_checked(xcb_connection_t *conn,
                                                xcb_font_t font)
{
	return checked_request(conn,
		xcb_close_font(conn,
		               font));
}

xcb_void_cookie_t xcb_image_text_8(xcb_connection_t *conn,
                                   xcb_drawable_t drawable,
                                   xcb_gcontext_t gc,
                                   int16_t x,
                                   int16_t y,
                                   const char *string)
{
	uint16_t n = strlen(string);
	uint16_t length = 4 + (n + 3) / 4;
	REQ_INIT(image_text_8, XCB_IMAGE_TEXT_8, length);
	r->n = n;
	r->drawable = drawable;
	r->gc = gc;
	r->x = x;
	r->y = y;
	buf_write(&conn->wbuf, string, n);
	buf_wpad(&conn->wbuf);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_image_text_8_checked(xcb_connection_t *conn,
                                           xcb_drawable_t drawable,
                                           xcb_gcontext_t gc,
                                           int16_t x,
                                           int16_t y,
                                           const char *string)
{
	return checked_request(conn,
		xcb_image_text_8(conn,
		                 drawable,
		                 gc,
		                 x,
		                 y,
		                 string));
}

xcb_void_cookie_t xcb_create_glyph_cursor(xcb_connection_t *conn,
                                          xcb_cursor_t cid,
                                          xcb_font_t source_font,
                                          xcb_font_t mask_font,
                                          uint16_t source_char,
                                          uint16_t mask_char,
                                          uint16_t fore_red,
                                          uint16_t fore_green,
                                          uint16_t fore_blue,
                                          uint16_t back_red,
                                          uint16_t back_green,
                                          uint16_t back_blue)
{
	uint16_t length = 8;
	REQ_INIT(create_glyph_cursor, XCB_CREATE_GLYPH_CURSOR, length);
	r->pad0 = 0;
	r->cid = cid;
	r->source_font = source_font;
	r->mask_font = mask_font;
	r->source_char = source_char;
	r->mask_char = mask_char;
	r->fore_red = fore_red;
	r->fore_green = fore_green;
	r->fore_blue = fore_blue;
	r->back_red = back_red;
	r->back_green = back_green;
	r->back_blue = back_blue;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_create_glyph_cursor_checked(xcb_connection_t *conn,
                                                  xcb_cursor_t cid,
                                                  xcb_font_t source_font,
                                                  xcb_font_t mask_font,
                                                  uint16_t source_char,
                                                  uint16_t mask_char,
                                                  uint16_t fore_red,
                                                  uint16_t fore_green,
                                                  uint16_t fore_blue,
                                                  uint16_t back_red,
                                                  uint16_t back_green,
                                                  uint16_t back_blue)
{
	return checked_request(conn,
		xcb_create_glyph_cursor(conn,
		                        cid,
		                        source_font,
		                        mask_font,
		                        source_char,
		                        mask_char,
		                        fore_red,
		                        fore_green,
		                        fore_blue,
		                        back_red,
		                        back_green,
		                        back_blue));
}

xcb_void_cookie_t xcb_create_cursor(xcb_connection_t *conn,
                                    xcb_cursor_t cid,
                                    xcb_pixmap_t source,
                                    xcb_pixmap_t mask,
                                    uint16_t fore_red,
                                    uint16_t fore_green,
                                    uint16_t fore_blue,
                                    uint16_t back_red,
                                    uint16_t back_green,
                                    uint16_t back_blue,
                                    uint16_t x,
                                    uint16_t y)
{
	uint16_t length = 8;
	REQ_INIT(create_cursor, XCB_CREATE_CURSOR, length);
	r->pad0 = 0;
	r->cid = cid;
	r->source = source;
	r->mask = mask;
	r->fore_red = fore_red;
	r->fore_green = fore_green;
	r->fore_blue = fore_blue;
	r->back_red = back_red;
	r->back_green = back_green;
	r->back_blue = back_blue;
	r->x = x;
	r->y = y;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_create_cursor_checked(xcb_connection_t *conn,
                                            xcb_cursor_t cid,
                                            xcb_pixmap_t source,
                                            xcb_pixmap_t mask,
                                            uint16_t fore_red,
                                            uint16_t fore_green,
                                            uint16_t fore_blue,
                                            uint16_t back_red,
                                            uint16_t back_green,
                                            uint16_t back_blue,
                                            uint16_t x,
                                            uint16_t y)
{
	return checked_request(conn,
		xcb_create_cursor(conn,
		                  cid,
		                  source,
		                  mask,
		                  fore_red,
		                  fore_green,
		                  fore_blue,
		                  back_red,
		                  back_green,
		                  back_blue,
		                  x,
		                  y));
}

xcb_void_cookie_t xcb_recolor_cursor(xcb_connection_t *conn,
                                     xcb_cursor_t cursor,
                                     uint16_t fore_red,
                                     uint16_t fore_green,
                                     uint16_t fore_blue,
                                     uint16_t back_red,
                                     uint16_t back_green,
                                     uint16_t back_blue)
{
	uint16_t length = 5;
	REQ_INIT(recolor_cursor, XCB_RECOLOR_CURSOR, length);
	r->pad0 = 0;
	r->cursor = cursor;
	r->fore_red = fore_red;
	r->fore_green = fore_green;
	r->fore_blue = fore_blue;
	r->back_red = back_red;
	r->back_green = back_green;
	r->back_blue = back_blue;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_recolor_cursor_checked(xcb_connection_t *conn,
                                             xcb_cursor_t cursor,
                                             uint16_t fore_red,
                                             uint16_t fore_green,
                                             uint16_t fore_blue,
                                             uint16_t back_red,
                                             uint16_t back_green,
                                             uint16_t back_blue)
{
	return checked_request(conn,
		xcb_recolor_cursor(conn,
		                   cursor,
		                   fore_red,
		                   fore_green,
		                   fore_blue,
		                   back_red,
		                   back_green,
		                   back_blue));
}

xcb_void_cookie_t xcb_free_cursor(xcb_connection_t *conn,
                                  xcb_cursor_t cursor)
{
	uint16_t length = 2;
	REQ_INIT(free_cursor, XCB_FREE_CURSOR, length);
	r->pad0 = 0;
	r->cursor = cursor;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_free_cursor_checked(xcb_connection_t *conn,
                                          xcb_cursor_t cursor)
{
	return checked_request(conn,
		xcb_free_cursor(conn,
		                cursor));
}

xcb_void_cookie_t xcb_create_pixmap(xcb_connection_t *conn,
                                    uint8_t depth,
                                    xcb_pixmap_t pid,
                                    xcb_drawable_t drawable,
                                    uint16_t width,
                                    uint16_t height)
{
	uint16_t length = 4;
	REQ_INIT(create_pixmap, XCB_CREATE_PIXMAP, length);
	r->depth = depth;
	r->pid = pid;
	r->drawable = drawable;
	r->width = width;
	r->height = height;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_create_pixmap_checked(xcb_connection_t *conn,
                                            uint8_t depth,
                                            xcb_pixmap_t pid,
                                            xcb_drawable_t drawable,
                                            uint16_t width,
                                            uint16_t height)
{
	return checked_request(conn,
		xcb_create_pixmap(conn,
		                  depth,
		                  pid,
		                  drawable,
		                  width,
		                  height));
}

xcb_void_cookie_t xcb_free_pixmap(xcb_connection_t *conn,
                                  xcb_pixmap_t pixmap)
{
	uint16_t length = 2;
	REQ_INIT(free_pixmap, XCB_FREE_PIXMAP, length);
	r->pad0 = 0;
	r->pixmap = pixmap;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_free_pixmap_checked(xcb_connection_t *conn,
                                          xcb_pixmap_t pixmap)
{
	return checked_request(conn,
		xcb_free_pixmap(conn,
		                pixmap));
}

xcb_void_cookie_t xcb_put_image(xcb_connection_t *conn,
                                uint8_t format,
                                xcb_drawable_t drawable,
                                xcb_gcontext_t gc,
                                uint16_t width,
                                uint16_t height,
                                int16_t dst_x,
                                int16_t dst_y,
                                uint8_t left_pad,
                                uint8_t depth,
                                uint32_t data_len,
                                const uint8_t *data)
{
	uint16_t length = 6 + (data_len + 3) / 4;
	REQ_INIT(put_image, XCB_PUT_IMAGE, length);
	r->format = format;
	r->drawable = drawable;
	r->gc = gc;
	r->width = width;
	r->height = height;
	r->dst_x = dst_x;
	r->dst_y = dst_y;
	r->left_pad = left_pad;
	r->depth = depth;
	r->pad0 = 0;
	buf_write(&conn->wbuf, data, data_len);
	buf_wpad(&conn->wbuf);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_put_image_checked(xcb_connection_t *conn,
                                        uint8_t format,
                                        xcb_drawable_t drawable,
                                        xcb_gcontext_t gc,
                                        uint16_t width,
                                        uint16_t height,
                                        int16_t dst_x,
                                        int16_t dst_y,
                                        uint8_t left_pad,
                                        uint8_t depth,
                                        uint32_t data_len,
                                        const uint8_t *data)
{
	return checked_request(conn,
		xcb_put_image(conn,
		              format,
		              drawable,
		              gc,
		              width,
		              height,
		              dst_x,
		              dst_y,
		              left_pad,
		              depth,
		              data_len,
		              data));
}

xcb_get_image_cookie_t xcb_get_image(xcb_connection_t *conn,
                                     uint8_t format,
                                     xcb_drawable_t drawable,
                                     int16_t x,
                                     int16_t y,
                                     uint16_t width,
                                     uint16_t height,
                                     uint32_t plane_mask)
{
	uint16_t length = 5;
	REQ_INIT(get_image, XCB_GET_IMAGE, length);
	r->format = format;
	r->drawable = drawable;
	r->x = x;
	r->y = y;
	r->width = width;
	r->height = height;
	r->plane_mask = plane_mask;
	xcb_get_image_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_image_reply_t *xcb_get_image_reply(xcb_connection_t *conn,
                                           xcb_get_image_cookie_t cookie,
                                           xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_image, answer->reply.length * 4);
	reply->response_type = answer->reply.response_type;
	reply->depth = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->visual = answer->reply.data32[0];
	memcpy(&reply[1], answer->data, answer->reply.length * 4);
	return reply;
}

uint8_t *xcb_get_image_data(const xcb_get_image_reply_t *reply)
{
	return (uint8_t*)&reply[1];
}

xcb_void_cookie_t xcb_copy_area(xcb_connection_t *conn,
                                xcb_drawable_t src_drawable,
                                xcb_drawable_t dst_drawable,
                                xcb_gcontext_t gc,
                                int16_t src_x,
                                int16_t src_y,
                                int16_t dst_x,
                                int16_t dst_y,
                                uint16_t width,
                                uint16_t height)
{
	uint16_t length = 7;
	REQ_INIT(copy_area, XCB_COPY_AREA, length);
	r->pad0 = 0;
	r->src_drawable = src_drawable;
	r->dst_drawable = dst_drawable;
	r->gc = gc;
	r->src_x = src_x;
	r->src_y = src_y;
	r->dst_x = dst_x;
	r->dst_y = dst_y;
	r->width = width;
	r->height = height;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_copy_area_checked(xcb_connection_t *conn,
                                        xcb_drawable_t src_drawable,
                                        xcb_drawable_t dst_drawable,
                                        xcb_gcontext_t gc,
                                        int16_t src_x,
                                        int16_t src_y,
                                        int16_t dst_x,
                                        int16_t dst_y,
                                        uint16_t width,
                                        uint16_t height)
{
	return checked_request(conn,
		xcb_copy_area(conn,
		              src_drawable,
		              dst_drawable,
		              gc,
		              src_x,
		              src_y,
		              dst_x,
		              dst_y,
		              width,
		              height));
}

xcb_void_cookie_t xcb_clear_area(xcb_connection_t *conn,
                                 uint8_t exposures,
                                 xcb_window_t window,
                                 int16_t x,
                                 int16_t y,
                                 uint16_t width,
                                 uint16_t height)
{
	uint16_t length = 4;
	REQ_INIT(clear_area, XCB_CLEAR_AREA, length);
	r->exposures = exposures;
	r->window = window;
	r->x = x;
	r->y = y;
	r->width = width;
	r->height = height;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_clear_area_checked(xcb_connection_t *conn,
                                         uint8_t exposures,
                                         xcb_window_t window,
                                         int16_t x,
                                         int16_t y,
                                         uint16_t width,
                                         uint16_t height)
{
	return checked_request(conn,
		xcb_clear_area(conn,
		               exposures,
		               window,
		               x,
		               y,
		               width,
		               height));
}

xcb_list_fonts_cookie_t xcb_list_fonts(xcb_connection_t *conn,
                                       uint16_t max_names,
                                       uint16_t pattern_len,
                                       const char *pattern)
{
	uint16_t length = 2 + (pattern_len + 3) / 4;
	REQ_INIT(list_fonts, XCB_LIST_FONTS, length);
	r->pad0 = 0;
	r->max_names = max_names;
	r->pattern_len = pattern_len;
	buf_write(&conn->wbuf, pattern, pattern_len);
	buf_wpad(&conn->wbuf);
	xcb_list_fonts_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_list_fonts_reply_t *xcb_list_fonts_reply(xcb_connection_t *conn,
                                             xcb_list_fonts_cookie_t cookie,
                                             xcb_generic_error_t **error)
{
	REPLY_ANSWER(priv_list_fonts, 0);
	reply->reply.response_type = answer->reply.response_type;
	reply->reply.pad0 = answer->reply.pad0;
	reply->reply.sequence = answer->reply.sequence;
	reply->reply.length = answer->reply.length;
	reply->reply.names_len = answer->reply.data16[0];
	reply->names = malloc(sizeof(*reply->names) * reply->reply.names_len);
	if (!reply->names)
		abort(); /* XXX */
	uint8_t *ptr = answer->data;
	for (size_t i = 0; i < reply->reply.names_len; ++i)
	{
		uint8_t length = *(ptr++);
		xcb_priv_str_t *priv_name = &reply->names[i];
		priv_name->str.name_len = length;
		priv_name->name = (char*)ptr;
		ptr += length;
	}
	return &reply->reply;
}

int xcb_list_fonts_names_length(const xcb_list_fonts_reply_t *reply)
{
	return reply->names_len;
}

xcb_str_iterator_t xcb_list_fonts_names_iterator(const xcb_list_fonts_reply_t *reply)
{
	xcb_priv_list_fonts_reply_t *priv_reply = (xcb_priv_list_fonts_reply_t*)reply;
	xcb_str_iterator_t it;
	it.data = &priv_reply->names->str;
	it.rem = reply->names_len;
	it.index = 0;
	return it;
}

xcb_void_cookie_t xcb_change_keyboard_mapping(xcb_connection_t *conn,
                                              uint8_t keycode_count,
                                              xcb_keycode_t first_keycode,
                                              uint8_t keysyms_per_keycode,
                                              const xcb_keysym_t *keysyms)
{
	uint16_t length = 2 + keycode_count * keysyms_per_keycode;
	REQ_INIT(change_keyboard_mapping, XCB_CHANGE_KEYBOARD_MAPPING, length);
	r->keycode_count = keycode_count;
	r->first_keycode = first_keycode;
	r->keysyms_per_keycode = keysyms_per_keycode;
	r->pad0 = 0;
	buf_write(&conn->wbuf, keysyms, keycode_count * keysyms_per_keycode);
	buf_wpad(&conn->wbuf);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_change_keyboard_mapping_checked(xcb_connection_t *conn,
                                                      uint8_t keycode_count,
                                                      xcb_keycode_t first_keycode,
                                                      uint8_t keysyms_per_keycode,
                                                      const xcb_keysym_t *keysyms)
{
	return checked_request(conn,
		xcb_change_keyboard_mapping(conn,
		                            keycode_count,
		                            first_keycode,
		                            keysyms_per_keycode,
		                            keysyms));
}

xcb_get_keyboard_mapping_cookie_t xcb_get_keyboard_mapping(xcb_connection_t *conn,
                                                           xcb_keycode_t first_keycode,
                                                           uint8_t count)
{
	uint16_t length = 2;
	REQ_INIT(get_keyboard_mapping, XCB_GET_KEYBOARD_MAPPING, length);
	r->pad0 = 0;
	r->first_keycode = first_keycode;
	r->count = count;
	r->pad1 = 0;
	xcb_get_keyboard_mapping_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	cookie.count = count;
	return cookie;
}

xcb_get_keyboard_mapping_reply_t *xcb_get_keyboard_mapping_reply(xcb_connection_t *conn,
                                                                 xcb_get_keyboard_mapping_cookie_t cookie,
                                                                 xcb_generic_error_t **error)
{
	REPLY_ANSWER(priv_get_keyboard_mapping, sizeof(*reply->keysyms)
	                                      * answer->reply.pad0
	                                      * cookie.count);
	reply->reply.response_type = answer->reply.response_type;
	reply->reply.keysyms_per_keycode = answer->reply.pad0;
	reply->reply.sequence = answer->reply.sequence;
	reply->reply.length = answer->reply.length;
	reply->keysyms_len = reply->reply.keysyms_per_keycode * cookie.count;
	for (size_t i = 0; i < reply->keysyms_len; ++i)
		reply->keysyms[i] = ((uint32_t*)answer->data)[i];
	return &reply->reply;
}

int xcb_get_keyboard_mapping_keysyms_length(const xcb_get_keyboard_mapping_reply_t *reply)
{
	xcb_priv_get_keyboard_mapping_reply_t *priv_reply = (xcb_priv_get_keyboard_mapping_reply_t*)reply;
	return priv_reply->keysyms_len;
}

xcb_keysym_t *xcb_get_keyboard_mapping_keysyms(const xcb_get_keyboard_mapping_reply_t *reply)
{
	xcb_priv_get_keyboard_mapping_reply_t *priv_reply = (xcb_priv_get_keyboard_mapping_reply_t*)reply;
	return priv_reply->keysyms;
}

xcb_void_cookie_t xcb_change_gc(xcb_connection_t *conn,
                                xcb_gcontext_t gc,
                                uint32_t value_mask,
                                const uint32_t *values)
{
	size_t n = 0;
	for (size_t i = 0; i < 23; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	uint16_t length = 3 + n;
	REQ_INIT(change_gc, XCB_CHANGE_GC, length);
	r->pad0 = 0;
	r->gc = gc;
	r->value_mask = value_mask;
	n = 0;
	if (value_mask & XCB_GC_FUNCTION)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_PLANE_MASK)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_FOREGROUND)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_BACKGROUND)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_LINE_WIDTH)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_LINE_STYLE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_CAP_STYLE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_JOIN_STYLE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_FILL_STYLE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_FILL_RULE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_TILE)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_STIPPLE)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_TILE_STIPPLE_ORIGIN_X)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_TILE_STIPPLE_ORIGIN_Y)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_FONT)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_SUBWINDOW_MODE)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_GRAPHICS_EXPOSURES)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_CLIP_ORIGIN_X)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_CLIP_ORIGIN_Y)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_CLIP_MASK)
		write_masked32(conn, values[n++]);
	if (value_mask & XCB_GC_DASH_OFFSET)
		write_masked16(conn, values[n++]);
	if (value_mask & XCB_GC_DASH_LIST)
		write_masked8(conn, values[n++]);
	if (value_mask & XCB_GC_ARC_MODE)
		write_masked8(conn, values[n++]);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_change_gc_checked(xcb_connection_t *conn,
                                        xcb_gcontext_t gc,
                                        uint32_t value_mask,
                                        const uint32_t *values)
{
	return checked_request(conn,
		xcb_change_gc(conn,
		              gc,
		              value_mask,
		              values));
}

xcb_void_cookie_t xcb_bell(xcb_connection_t *conn,
                           int8_t percent)
{
	uint16_t length = 1;
	REQ_INIT(bell, XCB_BELL, length);
	r->percent = percent;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_bell_checked(xcb_connection_t *conn,
                                   int8_t percent)
{
	return checked_request(conn,
		xcb_bell(conn,
		         percent));
}

xcb_query_pointer_cookie_t xcb_query_pointer(xcb_connection_t *conn,
                                             xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(query_pointer, XCB_QUERY_POINTER, length);
	r->pad0 = 0;
	r->window = window;
	xcb_query_pointer_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_query_pointer_reply_t *xcb_query_pointer_reply(xcb_connection_t *conn,
                                                   xcb_query_pointer_cookie_t cookie,
                                                   xcb_generic_error_t **error)
{
	REPLY_ANSWER(query_pointer, 0);
	reply->response_type = answer->reply.response_type;
	reply->same_screen = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->root = answer->reply.data32[0];
	reply->child = answer->reply.data32[1];
	reply->root_x = (int16_t)answer->reply.data16[4];
	reply->root_y = (int16_t)answer->reply.data16[5];
	reply->win_x = (int16_t)answer->reply.data16[6];
	reply->win_y = (int16_t)answer->reply.data16[7];
	reply->mask = answer->reply.data16[8];
	return reply;
}

xcb_void_cookie_t xcb_warp_pointer(xcb_connection_t *conn,
                                   xcb_window_t src_window,
                                   xcb_window_t dst_window,
                                   int16_t src_x,
                                   int16_t src_y,
                                   uint16_t src_width,
                                   uint16_t src_height,
                                   int16_t dst_x,
                                   int16_t dst_y)
{
	uint16_t length = 6;
	REQ_INIT(warp_pointer, XCB_WARP_POINTER, length);
	r->pad0 = 0;
	r->src_window = src_window;
	r->dst_window = dst_window;
	r->src_x = src_x;
	r->src_y = src_y;
	r->src_width = src_width;
	r->src_height = src_height;
	r->dst_x = dst_x;
	r->dst_y = dst_y;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_warp_pointer_checked(xcb_connection_t *conn,
                                           xcb_window_t src_window,
                                           xcb_window_t dst_window,
                                           int16_t src_x,
                                           int16_t src_y,
                                           uint16_t src_width,
                                           uint16_t src_height,
                                           int16_t dst_x,
                                           int16_t dst_y)
{
	return checked_request(conn,
		xcb_warp_pointer(conn,
		                 src_window,
		                 dst_window,
		                 src_x,
		                 src_y,
		                 src_width,
		                 src_height,
		                 dst_x,
		                 dst_y));
}

xcb_void_cookie_t xcb_grab_server(xcb_connection_t *conn)
{
	uint16_t length = 1;
	REQ_INIT(grab_server, XCB_GRAB_SERVER, length);
	r->pad0 = 0;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_grab_server_checked(xcb_connection_t *conn)
{
	return checked_request(conn,
		xcb_grab_server(conn));
}

xcb_void_cookie_t xcb_ungrab_server(xcb_connection_t *conn)
{
	uint16_t length = 1;
	REQ_INIT(ungrab_server, XCB_UNGRAB_SERVER, length);
	r->pad0 = 0;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_ungrab_server_checked(xcb_connection_t *conn)
{
	return checked_request(conn,
		xcb_ungrab_server(conn));
}

xcb_set_modifier_mapping_cookie_t xcb_set_modifier_mapping(xcb_connection_t *conn,
                                                           uint8_t keycodes_per_modifier,
                                                           const xcb_keycode_t *keycodes)
{
	uint16_t length = 1 + 2 * keycodes_per_modifier;
	REQ_INIT(set_modifier_mapping, XCB_SET_MODIFIER_MAPPING, length);
	r->keycodes_per_modifier = keycodes_per_modifier;
	buf_write(&conn->wbuf, keycodes, 8 * keycodes_per_modifier);
	xcb_set_modifier_mapping_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_set_modifier_mapping_reply_t *xcb_set_modifier_mapping_reply(xcb_connection_t *conn,
                                                                 xcb_set_modifier_mapping_cookie_t cookie,
                                                                 xcb_generic_error_t **error)
{
	REPLY_ANSWER(set_modifier_mapping, 0);
	reply->response_type = answer->reply.response_type;
	reply->status = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	return reply;
}

xcb_get_modifier_mapping_cookie_t xcb_get_modifier_mapping(xcb_connection_t *conn)
{
	uint16_t length = 1;
	REQ_INIT(get_modifier_mapping, XCB_GET_MODIFIER_MAPPING, length);
	r->pad0 = 0;
	xcb_get_modifier_mapping_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_modifier_mapping_reply_t *xcb_get_modifier_mapping_reply(xcb_connection_t *conn,
                                                                 xcb_get_modifier_mapping_cookie_t cookie,
                                                                 xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_modifier_mapping, 8 * answer->reply.pad0);
	reply->response_type = answer->reply.response_type;
	reply->keycodes_per_modifier = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	memcpy(&reply[1], answer->data, 8 * reply->keycodes_per_modifier);
	return reply;
}

xcb_keycode_t *xcb_get_modifier_mapping_keycodes(xcb_get_modifier_mapping_reply_t *reply)
{
	return (xcb_keycode_t*)&reply[1];
}

xcb_void_cookie_t xcb_no_operation(xcb_connection_t *conn)
{
	uint16_t length = 1;
	REQ_INIT(no_operation, XCB_NO_OPERATION, length);
	r->pad0 = 0;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_no_operation_checked(xcb_connection_t *conn)
{
	return checked_request(conn,
		xcb_no_operation(conn));
}

xcb_void_cookie_t xcb_reparent_window(xcb_connection_t *conn,
                                      xcb_window_t window,
                                      xcb_window_t parent,
                                      int16_t x,
                                      int16_t y)
{
	uint16_t length = 4;
	REQ_INIT(reparent_window, XCB_REPARENT_WINDOW, length);
	r->pad0 = 0;
	r->window = window;
	r->parent = parent;
	r->x = x;
	r->y = y;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_reparent_window_checked(xcb_connection_t *conn,
                                              xcb_window_t window,
                                              xcb_window_t parent,
                                              int16_t x,
                                              int16_t y)
{
	return checked_request(conn,
		xcb_reparent_window(conn,
		                    window,
		                    parent,
		                    x,
		                    y));
}

xcb_get_geometry_cookie_t xcb_get_geometry(xcb_connection_t *conn,
                                           xcb_drawable_t drawable)
{
	uint16_t length = 2;
	REQ_INIT(get_geometry, XCB_GET_GEOMETRY, length);
	r->pad0 = 0;
	r->drawable = drawable;
	xcb_get_geometry_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_geometry_reply_t *xcb_get_geometry_reply(xcb_connection_t *conn,
                                                 xcb_get_geometry_cookie_t cookie,
                                                 xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_geometry, 0);
	reply->response_type = answer->reply.response_type;
	reply->depth = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->root = answer->reply.data32[0];
	reply->x = (int16_t)answer->reply.data16[2];
	reply->y = (int16_t)answer->reply.data16[3];
	reply->width = answer->reply.data16[4];
	reply->height = answer->reply.data16[5];
	reply->border_width = answer->reply.data16[6];
	return reply;
}

xcb_query_tree_cookie_t xcb_query_tree(xcb_connection_t *conn,
                                       xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(query_tree, XCB_QUERY_TREE, length);
	r->pad0 = 0;
	r->window = window;
	xcb_query_tree_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_query_tree_reply_t *xcb_query_tree_reply(xcb_connection_t *conn,
                                             xcb_query_tree_cookie_t cookie,
                                             xcb_generic_error_t **error)
{
	REPLY_ANSWER(query_tree, 4 * answer->reply.data16[4]);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->root = answer->reply.data32[0];
	reply->parent = answer->reply.data32[1];
	reply->children_len = answer->reply.data16[4];
	memcpy(&reply[1], answer->data, 4 * reply->children_len);
	return reply;
}

xcb_window_t *xcb_query_tree_children(const xcb_query_tree_reply_t *reply)
{
	return (xcb_window_t*)(&reply[1]);
}

xcb_get_property_cookie_t xcb_get_property(xcb_connection_t *conn,
                                           uint8_t _delete,
                                           xcb_window_t window,
                                           xcb_atom_t property,
                                           xcb_atom_t type,
                                           uint32_t long_offset,
                                           uint32_t long_length)
{
	uint16_t length = 6;
	REQ_INIT(get_property, XCB_GET_PROPERTY, length);
	r->_delete = _delete;
	r->window = window;
	r->property = property;
	r->type = type;
	r->long_offset = long_offset;
	r->long_length = long_length;
	xcb_get_property_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_property_reply_t *xcb_get_property_reply(xcb_connection_t *conn,
                                                 xcb_get_property_cookie_t cookie,
                                                 xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_property, answer->reply.length * 4);
	reply->response_type = answer->reply.response_type;
	reply->format = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->type = answer->reply.data32[0];
	reply->bytes_after = answer->reply.data32[1];
	reply->value_len = answer->reply.data32[2];
	memcpy(&reply[1], answer->data, answer->reply.length * 4);
	return reply;
}

void *xcb_get_property_value(const xcb_get_property_reply_t *reply)
{
	return (void*)&reply[1];
}

xcb_list_properties_cookie_t xcb_list_properties(xcb_connection_t *conn,
                                                 xcb_window_t window)
{
	uint16_t length = 2;
	REQ_INIT(list_properties, XCB_LIST_PROPERTIES, length);
	r->pad0 = 0;
	r->window = window;
	xcb_list_properties_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_list_properties_reply_t *xcb_list_properties_reply(xcb_connection_t *conn,
                                                       xcb_list_properties_cookie_t cookie,
                                                       xcb_generic_error_t **error)
{
	REPLY_ANSWER(list_properties, answer->reply.length * 4);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->atoms_len = answer->reply.data16[0];
	memcpy(&reply[1], answer->data, answer->reply.length * 4);
	return reply;
}

xcb_atom_t *xcb_list_properties_atoms(const xcb_list_properties_reply_t *reply)
{
	return (xcb_atom_t*)&reply[1];
}

xcb_void_cookie_t xcb_grab_button(xcb_connection_t *conn,
                                  uint8_t owner_events,
                                  xcb_window_t grab_window,
                                  uint16_t event_mask,
                                  uint8_t pointer_mode,
                                  uint8_t keyboard_mode,
                                  xcb_window_t confine_to,
                                  xcb_cursor_t cursor,
                                  xcb_button_t button,
                                  uint16_t modifiers)
{
	uint16_t length = 6;
	REQ_INIT(grab_button, XCB_GRAB_BUTTON, length);
	r->owner_events = owner_events;
	r->grab_window = grab_window;
	r->event_mask = event_mask;
	r->pointer_mode = pointer_mode;
	r->keyboard_mode = keyboard_mode;
	r->confine_to = confine_to;
	r->cursor = cursor;
	r->button = button;
	r->pad0 = 0;
	r->modifiers = modifiers;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_grab_button_checked(xcb_connection_t *conn,
                                          uint8_t owner_events,
                                          xcb_window_t grab_window,
                                          uint16_t event_mask,
                                          uint8_t pointer_mode,
                                          uint8_t keyboard_mode,
                                          xcb_window_t confine_to,
                                          xcb_cursor_t cursor,
                                          xcb_button_t button,
                                          uint16_t modifiers)
{
	return checked_request(conn,
		xcb_grab_button(conn,
		                owner_events,
		                grab_window,
		                event_mask,
		                pointer_mode,
		                keyboard_mode,
		                confine_to,
		                cursor,
		                button,
		                modifiers));
}

xcb_void_cookie_t xcb_ungrab_button(xcb_connection_t *conn,
                                    xcb_button_t button,
                                    xcb_window_t grab_window,
                                    uint16_t modifiers)
{
	uint16_t length = 3;
	REQ_INIT(ungrab_button, XCB_UNGRAB_BUTTON, length);
	r->button = button;
	r->grab_window = grab_window;
	r->modifiers = modifiers;
	r->pad0 = 0;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_ungrab_button_checked(xcb_connection_t *conn,
                                            xcb_button_t button,
                                            xcb_window_t grab_window,
                                            uint16_t modifiers)
{
	return checked_request(conn,
		xcb_ungrab_button(conn,
		                  button,
		                  grab_window,
		                  modifiers));
}

xcb_void_cookie_t xcb_delete_property(xcb_connection_t *conn,
                                      xcb_window_t window,
                                      xcb_property_t property)
{
	uint16_t length = 3;
	REQ_INIT(delete_property, XCB_DELETE_PROPERTY, length);
	r->pad0 = 0;
	r->window = window;
	r->property = property;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_delete_property_checked(xcb_connection_t *conn,
                                              xcb_window_t window,
                                              xcb_property_t property)
{
	return checked_request(conn,
		xcb_delete_property(conn,
		                    window,
		                    property));
}

xcb_grab_pointer_cookie_t xcb_grab_pointer(xcb_connection_t *conn,
                                           uint8_t owner_events,
                                           xcb_window_t grab_window,
                                           uint16_t event_mask,
                                           uint8_t pointer_mode,
                                           uint8_t keyboard_mode,
                                           xcb_window_t confine_to,
                                           xcb_cursor_t cursor,
                                           xcb_timestamp_t time)
{
	uint16_t length = 6;
	REQ_INIT(grab_pointer, XCB_GRAB_POINTER, length);
	r->owner_events = owner_events;
	r->grab_window = grab_window;
	r->event_mask = event_mask;
	r->pointer_mode = pointer_mode;
	r->keyboard_mode = keyboard_mode;
	r->confine_to = confine_to;
	r->cursor = cursor;
	r->time = time;
	xcb_grab_pointer_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_grab_pointer_reply_t *xcb_grab_pointer_reply(xcb_connection_t *conn,
                                                 xcb_grab_pointer_cookie_t cookie,
                                                 xcb_generic_error_t **error)
{
	REPLY_ANSWER(grab_pointer, 0);
	reply->response_type = answer->reply.response_type;
	reply->status = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	return reply;
}

xcb_void_cookie_t xcb_ungrab_pointer(xcb_connection_t *conn,
                                     xcb_timestamp_t time)
{
	uint16_t length = 2;
	REQ_INIT(ungrab_pointer, XCB_UNGRAB_POINTER, length);
	r->pad0 = 0;
	r->time = time;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_ungrab_pointer_checked(xcb_connection_t *conn,
                                             xcb_timestamp_t time)
{
	return checked_request(conn,
		xcb_ungrab_pointer(conn,
		                   time));
}

xcb_set_pointer_mapping_cookie_t xcb_set_pointer_mapping(xcb_connection_t *conn,
                                                         uint8_t map_len,
                                                         const uint8_t *map)
{
	uint16_t length = 1 + (map_len + 3) / 4;
	REQ_INIT(set_pointer_mapping, XCB_SET_POINTER_MAPPING, length);
	r->map_len = map_len;
	buf_write(&conn->wbuf, map, map_len);
	buf_wpad(&conn->wbuf);
	xcb_set_pointer_mapping_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_set_pointer_mapping_reply_t *xcb_set_pointer_mapping_reply(xcb_connection_t *conn,
                                                               xcb_set_pointer_mapping_cookie_t cookie,
                                                               xcb_generic_error_t **error)
{
	REPLY_ANSWER(set_pointer_mapping, 0);
	reply->response_type = answer->reply.response_type;
	reply->status = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	return reply;
}

xcb_get_pointer_mapping_cookie_t xcb_get_pointer_mapping(xcb_connection_t *conn)
{
	uint16_t length = 1;
	REQ_INIT(get_pointer_mapping, XCB_GET_POINTER_MAPPING, length);
	r->pad0 = 0;
	xcb_get_pointer_mapping_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_pointer_mapping_reply_t *xcb_get_pointer_mapping_reply(xcb_connection_t *conn,
                                                               xcb_get_pointer_mapping_cookie_t cookie,
                                                               xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_pointer_mapping, answer->reply.pad0);
	reply->response_type = answer->reply.response_type;
	reply->map_len = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	memcpy(&reply[1], answer->data, answer->reply.pad0);
	return reply;
}

uint8_t *xcb_get_pointer_mapping_map(const xcb_get_pointer_mapping_reply_t *reply)
{
	return (uint8_t*)&reply[1];
}

xcb_void_cookie_t xcb_change_pointer_control(xcb_connection_t *conn,
                                             int16_t acceleration_numerator,
                                             int16_t acceleration_denominator,
                                             int16_t threshold,
                                             uint8_t do_acceleration,
                                             uint8_t do_threshold)
{
	uint16_t length = 3;
	REQ_INIT(change_pointer_control, XCB_CHANGE_POINTER_CONTROL, length);
	r->acceleration_numerator = acceleration_numerator;
	r->acceleration_denominator = acceleration_denominator;
	r->threshold = threshold;
	r->do_acceleration = do_acceleration;
	r->do_threshold = do_threshold;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_change_pointer_control_checked(xcb_connection_t *conn,
                                                     int16_t acceleration_numerator,
                                                     int16_t acceleration_denominator,
                                                     int16_t threshold,
                                                     uint8_t do_acceleration,
                                                     uint8_t do_threshold)
{
	return checked_request(conn,
		xcb_change_pointer_control(conn,
		                           acceleration_numerator,
		                           acceleration_denominator,
		                           threshold,
		                           do_acceleration,
		                           do_threshold));
}

xcb_get_pointer_control_cookie_t xcb_get_pointer_control(xcb_connection_t *conn)
{
	uint16_t length = 1;
	REQ_INIT(get_pointer_control, XCB_GET_POINTER_CONTROL, length);
	r->pad0 = 0;
	xcb_get_pointer_control_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_pointer_control_reply_t *xcb_get_pointer_control_reply(xcb_connection_t *conn,
                                                               xcb_get_pointer_control_cookie_t cookie,
                                                               xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_pointer_control, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->acceleration_numerator = (int16_t)answer->reply.data16[0];
	reply->acceleration_denominator = (int16_t)answer->reply.data16[1];
	reply->threshold = (int16_t)answer->reply.data16[2];
	return reply;
}

xcb_query_best_size_cookie_t xcb_query_best_size(xcb_connection_t *conn,
                                                 uint8_t class,
                                                 xcb_drawable_t drawable,
                                                 uint16_t width,
                                                 uint16_t height)
{
	uint16_t length = 3;
	REQ_INIT(query_best_size, XCB_QUERY_BEST_SIZE, length);
	r->class = class,
	r->drawable = drawable,
	r->width = width;
	r->height = height;
	xcb_query_best_size_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_query_best_size_reply_t *xcb_query_best_size_reply(xcb_connection_t *conn,
                                                       xcb_query_best_size_cookie_t cookie,
                                                       xcb_generic_error_t **error)
{
	REPLY_ANSWER(query_best_size, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->width = answer->reply.data16[0];
	reply->height = answer->reply.data16[1];
	return reply;
}

xcb_void_cookie_t xcb_change_keyboard_control(xcb_connection_t *conn,
                                              uint32_t value_mask,
                                              const void *values)
{
	uint16_t length = 2;
	for (size_t i = 0; i < 8; ++i)
	{
		if (value_mask & (1 << i))
			length++;
	}
	REQ_INIT(change_keyboard_control, XCB_CHANGE_KEYBOARD_CONTROL, length);
	r->pad0 = 0;
	r->value_mask = value_mask;
	size_t n = 0;
	for (size_t i = 0; i < 8; ++i)
	{
		if (value_mask & (1 << i))
			buf_write(&conn->wbuf, &((uint32_t*)values)[n++], 4);
	}
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_change_keyboard_control_checked(xcb_connection_t *conn,
                                                      uint32_t value_mask,
                                                      const void *values)
{
	return checked_request(conn,
		xcb_change_keyboard_control(conn,
		                            value_mask,
		                            values));
}

xcb_get_keyboard_control_cookie_t xcb_get_keyboard_control(xcb_connection_t *conn)
{
	uint16_t length = 1;
	REQ_INIT(get_keyboard_control, XCB_GET_KEYBOARD_CONTROL, length);
	r->pad0 = 0;
	xcb_get_keyboard_control_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_get_keyboard_control_reply_t *xcb_get_keyboard_control_reply(xcb_connection_t *conn,
                                                                 xcb_get_keyboard_control_cookie_t cookie,
                                                                 xcb_generic_error_t **error)
{
	REPLY_ANSWER(get_keyboard_control, 0);
	reply->response_type = answer->reply.response_type;
	reply->global_auto_repeat = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->led_mask = answer->reply.data32[0];
	reply->key_click_percent = answer->reply.data8[4];
	reply->bell_percent = answer->reply.data8[5];
	reply->bell_pitch = answer->reply.data16[3];
	reply->bell_duration = answer->reply.data16[4];
	for (uint8_t i = 0; i < 12; ++i)
		reply->auto_repeats[i] = answer->reply.data8[12 + i];
	for (uint8_t i = 12; i < 32; ++i)
		reply->auto_repeats[i] = answer->data[i - 12];
	return reply;
}

xcb_void_cookie_t xcb_set_dashes(xcb_connection_t *conn,
                                 xcb_gcontext_t gc,
                                 uint16_t dash_offset,
                                 uint16_t dashes_len,
                                 const uint8_t *dashes)
{
	uint16_t length = 3 + (dashes_len + 3) / 4;
	REQ_INIT(set_dashes, XCB_SET_DASHES, length);
	r->pad0 = 0;
	r->gc = gc;
	r->dash_offset = dash_offset;
	r->dashes_len = dashes_len;
	buf_write(&conn->wbuf, dashes, dashes_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_set_dashes_checked(xcb_connection_t *conn,
                                         xcb_gcontext_t gc,
                                         uint16_t dash_offset,
                                         uint16_t dashes_len,
                                         const uint8_t *dashes)
{
	return checked_request(conn,
		xcb_set_dashes(conn,
		               gc,
		               dash_offset,
		               dashes_len,
		               dashes));
}

xcb_void_cookie_t xcb_set_clip_rectangles(xcb_connection_t *conn,
                                          uint8_t ordering,
                                          xcb_gcontext_t gc,
                                          int16_t clip_x_origin,
                                          int16_t clip_y_origin,
                                          uint16_t rectangles_len,
                                          const xcb_rectangle_t *rectangles)
{
	uint16_t length = 3 + 2 * rectangles_len;
	REQ_INIT(set_clip_rectangles, XCB_SET_CLIP_RECTANGLES, length);
	r->ordering = ordering;
	r->gc = gc;
	r->clip_x_origin = clip_x_origin;
	r->clip_y_origin = clip_y_origin;
	buf_write(&conn->wbuf, rectangles, 8 * rectangles_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_set_clip_rectangles_checked(xcb_connection_t *conn,
                                                  uint8_t ordering,
                                                  xcb_gcontext_t gc,
                                                  int16_t clip_x_origin,
                                                  int16_t clip_y_origin,
                                                  uint16_t rectangles_len,
                                                  const xcb_rectangle_t *rectangles)
{
	return checked_request(conn,
		xcb_set_clip_rectangles(conn,
		                        ordering,
		                        gc,
		                        clip_x_origin,
		                        clip_y_origin,
		                        rectangles_len,
		                        rectangles));
}

xcb_void_cookie_t xcb_rotate_properties(xcb_connection_t *conn,
                                        xcb_window_t window,
                                        uint16_t atoms_len,
                                        int16_t delta,
                                        const xcb_atom_t *atoms)
{
	uint16_t length = 3 + atoms_len;
	REQ_INIT(rotate_properties, XCB_ROTATE_PROPERTIES, length);
	r->pad0 = 0;
	r->window = window;
	r->atoms_len = atoms_len;
	r->delta = delta;
	buf_write(&conn->wbuf, atoms, 4 * atoms_len);
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_rotate_properties_checked(xcb_connection_t *conn,
                                                xcb_window_t window,
                                                uint16_t atoms_len,
                                                int16_t delta,
                                                const xcb_atom_t *atoms)
{
	return checked_request(conn,
		xcb_rotate_properties(conn,
		                      window,
		                      atoms_len,
		                      delta,
		                      atoms));
}
