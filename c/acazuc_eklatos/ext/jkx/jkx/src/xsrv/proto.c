#include "xsrv.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CREATE_WINDOW            1
#define CHANGE_WINDOW_ATTRIBUTES 2
#define GET_WINDOW_ATTRIBUTES    3
#define DESTROY_WINDOW           4
#define DESTROY_SUBWINDOWS       5
#define REPARENT_WINDOW          7
#define MAP_WINDOW               8
#define MAP_SUBWINDOWS           9
#define UNMAP_WINDOW             10
#define UNMAP_SUBWINDOWS         11
#define CONFIGURE_WINDOW         12
#define GET_GEOMETRY             14
#define QUERY_TREE               15
#define INTERN_ATOM              16
#define GET_ATOM_NAME            17
#define CHANGE_PROPERTY          18
#define DELETE_PROPERTY          19
#define GET_PROPERTY             20
#define LIST_PROPERTIES          21
#define GRAB_POINTER             26
#define UNGRAB_POINTER           27
#define GRAB_BUTTON              28
#define UNGRAB_BUTTON            29
#define GRAB_SERVER              36
#define UNGRAB_SERVER            37
#define QUERY_POINTER            38
#define WARP_POINTER             41
#define SET_INPUT_FOCUS          42
#define GET_INPUT_FOCUS          43
#define OPEN_FONT                45
#define CLOSE_FONT               46
#define LIST_FONTS               49
#define CREATE_PIXMAP            53
#define FREE_PIXMAP              54
#define CREATE_GC                55
#define CHANGE_GC                56
#define COPY_GC                  57
#define FREE_GC                  60
#define CLEAR_AREA               61
#define COPY_AREA                62
#define POLY_POINT               64
#define POLY_LINE                65
#define POLY_SEGMENT             66
#define POLY_RECTANGLE           67
#define POLY_ARC                 68
#define FILL_POLY                69
#define POLY_FILL_RECTANGLE      70
#define POLY_FILL_ARC            71
#define PUT_IMAGE                72
#define GET_IMAGE                73
#define IMAGE_TEXT8              76
#define CREATE_CURSOR            93
#define CREATE_GLYPH_CURSOR      94
#define FREE_CURSOR              95
#define RECOLOR_CURSOR           96
#define QUERY_BEST_SIZE          97
#define QUERY_EXTENSION          98
#define LIST_EXTENSIONS          99
#define CHANGE_KEYBOARD_MAPPING  100
#define GET_KEYBOARD_MAPPING     101
#define CHANGE_KEYBOARD_CONTROL  102
#define GET_KEYBOARD_CONTROL     103
#define BELL                     104
#define CHANGE_POINTER_CONTROL   105
#define GET_POINTER_CONTROL      106
#define SET_POINTER_MAPPING      116
#define GET_POINTER_MAPPING      117
#define SET_MODIFIER_MAPPING     118
#define GET_MODIFIER_MAPPING     119
#define NO_OPERATION             127

#define OBJECT_ID(obj) ((obj) ? (obj)->object.id : None)

#define VM_RU8(mask, value, def) \
do \
{ \
	if (value_mask & (mask)) \
	{ \
		value = CLIENT_RU8(client); \
		CLIENT_READ(client, NULL, 3); \
	} \
	else \
	{ \
		value = def; \
	} \
} while (0)

#define VM_RI16(mask, value, def) \
do \
{ \
	if (value_mask & (mask)) \
	{ \
		value = CLIENT_RI16(client); \
		CLIENT_READ(client, NULL, 2); \
	} \
	else \
	{ \
		value = def; \
	} \
} while (0)

#define VM_RU16(mask, value, def) \
do \
{ \
	if (value_mask & (mask)) \
	{ \
		value = CLIENT_RU16(client); \
		CLIENT_READ(client, NULL, 2); \
	} \
	else \
	{ \
		value = def; \
	} \
} while (0)

#define VM_RU32(mask, value, def) \
do \
{ \
	if (value_mask & (mask)) \
		value = CLIENT_RU32(client); \
	else \
		value = def; \
} while (0)

#define TEST_MIN_MAX(x, y) \
do \
{ \
	if ((x) < min_x) \
		min_x = (x); \
	if ((x) > max_x) \
		max_x = (x); \
	if ((y) < min_y) \
		min_y = (y); \
	if ((y) > max_y) \
		max_y = (y); \
} while (0)

int client_error(struct xsrv *xsrv, struct client *client,
                 struct request *request, uint8_t error_code,
                 uint32_t xid)
{
	(void)xsrv;
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 0);
	CLIENT_WU8(client, error_code);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, xid);
	CLIENT_WU16(client, 0);
	CLIENT_WU8(client, request->opcode);
	CLIENT_WRITE(client, NULL, 21);
	xsrv->need_process = 1;
	return 1;
}

int ev_key_press(struct xsrv *xsrv, struct client *client, uint8_t detail,
                 uint32_t time, struct window *root, struct window *event,
                 struct window *child, int16_t root_x, int16_t root_y,
                 int16_t event_x, int16_t event_y, uint16_t state,
                 uint8_t same_screen)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, KeyPress);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(root));
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(child));
	CLIENT_WI16(client, root_x);
	CLIENT_WI16(client, root_y);
	CLIENT_WI16(client, event_x);
	CLIENT_WI16(client, event_y);
	CLIENT_WU16(client, state);
	CLIENT_WU8(client, same_screen);
	CLIENT_WRITE(client, NULL, 1);
	xsrv->need_process = 1;
	return 1;
}

int ev_key_release(struct xsrv *xsrv, struct client *client, uint8_t detail,
                   uint32_t time, struct window *root, struct window *event,
                   struct window *child, int16_t root_x, int16_t root_y,
                   int16_t event_x, int16_t event_y, uint16_t state,
                   uint8_t same_screen)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, KeyRelease);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(root));
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(child));
	CLIENT_WI16(client, root_x);
	CLIENT_WI16(client, root_y);
	CLIENT_WI16(client, event_x);
	CLIENT_WI16(client, event_y);
	CLIENT_WU16(client, state);
	CLIENT_WU8(client, same_screen);
	CLIENT_WRITE(client, NULL, 1);
	xsrv->need_process = 1;
	return 1;
}

int ev_button_press(struct xsrv *xsrv, struct client *client, uint8_t detail,
                    uint32_t time, struct window *root, struct window *event,
                    struct window *child, int16_t root_x, int16_t root_y,
                    int16_t event_x, int16_t event_y, uint16_t state,
                    uint8_t same_screen)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, ButtonPress);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(root));
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(child));
	CLIENT_WI16(client, root_x);
	CLIENT_WI16(client, root_y);
	CLIENT_WI16(client, event_x);
	CLIENT_WI16(client, event_y);
	CLIENT_WU16(client, state);
	CLIENT_WU8(client, same_screen);
	CLIENT_WRITE(client, NULL, 1);
	xsrv->need_process = 1;
	return 1;
}

int ev_button_release(struct xsrv *xsrv, struct client *client, uint8_t detail,
                      uint32_t time, struct window *root, struct window *event,
                      struct window *child, int16_t root_x, int16_t root_y,
                      int16_t event_x, int16_t event_y, uint16_t state,
                      uint8_t same_screen)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, ButtonRelease);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(root));
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(child));
	CLIENT_WI16(client, root_x);
	CLIENT_WI16(client, root_y);
	CLIENT_WI16(client, event_x);
	CLIENT_WI16(client, event_y);
	CLIENT_WU16(client, state);
	CLIENT_WU8(client, same_screen);
	CLIENT_WRITE(client, NULL, 1);
	xsrv->need_process = 1;
	return 1;
}

int ev_motion_notify(struct xsrv *xsrv, struct client *client, uint8_t detail,
                     uint32_t time, struct window *root, struct window *event,
                     struct window *child, int16_t root_x, int16_t root_y,
                     int16_t event_x, int16_t event_y, uint16_t state,
                     uint8_t same_screen)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, MotionNotify);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(root));
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(child));
	CLIENT_WI16(client, root_x);
	CLIENT_WI16(client, root_y);
	CLIENT_WI16(client, event_x);
	CLIENT_WI16(client, event_y);
	CLIENT_WU16(client, state);
	CLIENT_WU8(client, same_screen);
	CLIENT_WRITE(client, NULL, 1);
	xsrv->need_process = 1;
	return 1;
}

int ev_enter_notify(struct xsrv *xsrv, struct client *client, uint8_t detail,
                    uint32_t time, struct window *root, struct window *event,
                    struct window *child, int16_t root_x, int16_t root_y,
                    int16_t event_x, int16_t event_y, uint16_t state,
                    uint8_t same_screen)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, EnterNotify);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(root));
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(child));
	CLIENT_WI16(client, root_x);
	CLIENT_WI16(client, root_y);
	CLIENT_WI16(client, event_x);
	CLIENT_WI16(client, event_y);
	CLIENT_WU16(client, state);
	CLIENT_WU8(client, same_screen);
	CLIENT_WRITE(client, NULL, 1);
	xsrv->need_process = 1;
	return 1;
}

int ev_leave_notify(struct xsrv *xsrv, struct client *client, uint8_t detail,
                    uint32_t time, struct window *root, struct window *event,
                    struct window *child, int16_t root_x, int16_t root_y,
                    int16_t event_x, int16_t event_y, uint16_t state,
                    uint8_t same_screen)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, LeaveNotify);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(root));
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(child));
	CLIENT_WI16(client, root_x);
	CLIENT_WI16(client, root_y);
	CLIENT_WI16(client, event_x);
	CLIENT_WI16(client, event_y);
	CLIENT_WU16(client, state);
	CLIENT_WU8(client, same_screen);
	CLIENT_WRITE(client, NULL, 1);
	xsrv->need_process = 1;
	return 1;
}

int ev_focus_in(struct xsrv *xsrv, struct client *client,
                uint8_t detail, struct window *window, uint8_t mode)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, FocusIn);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU8(client, mode);
	CLIENT_WRITE(client, NULL, 23);
	xsrv->need_process = 1;
	return 1;
}

int ev_focus_out(struct xsrv *xsrv, struct client *client,
                 uint8_t detail, struct window *window, uint8_t mode)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, FocusOut);
	CLIENT_WU8(client, detail);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU8(client, mode);
	CLIENT_WRITE(client, NULL, 23);
	xsrv->need_process = 1;
	return 1;
}

int ev_keymap_notify(struct xsrv *xsrv, struct client *client, uint8_t *keys)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, KeymapNotify);
	CLIENT_WRITE(client, keys, 31);
	xsrv->need_process = 1;
	return 1;
}

int ev_expose(struct xsrv *xsrv, struct client *client,
              struct window *window, int16_t x, int16_t y,
              uint16_t width, uint16_t height, uint16_t count)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, Expose);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU16(client, x);
	CLIENT_WU16(client, y);
	CLIENT_WU16(client, width);
	CLIENT_WU16(client, height);
	CLIENT_WU16(client, count);
	CLIENT_WRITE(client, NULL, 14);
	xsrv->need_process = 1;
	return 1;
}

int ev_graphics_exposure(struct xsrv *xsrv, struct client *client,
                         struct drawable *drawable, uint16_t x, uint16_t y,
                         uint16_t width, uint16_t height,
                         uint16_t minor_opcode, uint16_t count,
                         uint8_t major_opcode)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, GraphicsExpose);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(drawable));
	CLIENT_WU16(client, x);
	CLIENT_WU16(client, y);
	CLIENT_WU16(client, width);
	CLIENT_WU16(client, height);
	CLIENT_WU16(client, minor_opcode);
	CLIENT_WU16(client, count);
	CLIENT_WU8(client, major_opcode);
	CLIENT_WRITE(client, NULL, 11);
	xsrv->need_process = 1;
	return 1;
}

int ev_no_exposure(struct xsrv *xsrv, struct client *client,
                   struct drawable *drawable, uint16_t minor_opcode,
                   uint8_t major_opcode)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, NoExpose);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(drawable));
	CLIENT_WU16(client, minor_opcode);
	CLIENT_WU8(client, major_opcode);
	CLIENT_WRITE(client, NULL, 21);
	xsrv->need_process = 1;
	return 1;
}

int ev_visibility_notify(struct xsrv *xsrv, struct client *client,
                         struct window *window, uint8_t state)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, VisibilityNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU8(client, state);
	CLIENT_WRITE(client, NULL, 23);
	xsrv->need_process = 1;
	return 1;
}

int ev_create_notify(struct xsrv *xsrv, struct client *client,
                     struct window *parent, struct window *window,
                     int16_t x, int16_t y, uint16_t width, uint16_t height,
                     uint16_t border_width, uint8_t override_redirect)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, CreateNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(parent));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WI16(client, x);
	CLIENT_WI16(client, y);
	CLIENT_WU16(client, width);
	CLIENT_WU16(client, height);
	CLIENT_WU16(client, border_width);
	CLIENT_WU8(client, override_redirect);
	CLIENT_WRITE(client, NULL, 9);
	xsrv->need_process = 1;
	return 1;
}

int ev_destroy_notify(struct xsrv *xsrv, struct client *client,
                      struct window *event, struct window *window)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, DestroyNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WRITE(client, NULL, 20);
	xsrv->need_process = 1;
	return 1;
}

int ev_unmap_notify(struct xsrv *xsrv, struct client *client,
                    struct window *event, struct window *window,
                    uint8_t from_configure)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, UnmapNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU8(client, from_configure);
	CLIENT_WRITE(client, NULL, 19);
	xsrv->need_process = 1;
	return 1;
}

int ev_map_notify(struct xsrv *xsrv, struct client *client,
                  struct window *event, struct window *window,
                  uint8_t override_redirect)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, MapNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU8(client, override_redirect);
	CLIENT_WRITE(client, NULL, 19);
	xsrv->need_process = 1;
	return 1;
}

int ev_map_request(struct xsrv *xsrv, struct client *client,
                   struct window *parent, struct window *window)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, MapRequest);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(parent));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WRITE(client, NULL, 20);
	xsrv->need_process = 1;
	return 1;
}

int ev_reparent_notify(struct xsrv *xsrv, struct client *client,
                       struct window *event, struct window *window,
                       struct window *parent, int16_t x, int16_t y,
                       uint8_t override_redirect)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, ReparentNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU32(client, OBJECT_ID(parent));
	CLIENT_WI16(client, x);
	CLIENT_WI16(client, y);
	CLIENT_WU8(client, override_redirect);
	CLIENT_WRITE(client, NULL, 11);
	xsrv->need_process = 1;
	return 1;
}

int ev_configure_notify(struct xsrv *xsrv, struct client *client,
                        struct window *event, struct window *window,
                        struct window *above_sibling, int16_t x, int16_t y,
                        uint16_t width, uint16_t height, uint16_t border_width,
                        uint8_t override_redirect)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, ConfigureNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU32(client, OBJECT_ID(above_sibling));
	CLIENT_WI16(client, x);
	CLIENT_WI16(client, y);
	CLIENT_WU16(client, width);
	CLIENT_WU16(client, height);
	CLIENT_WU16(client, border_width);
	CLIENT_WU8(client, override_redirect);
	CLIENT_WRITE(client, NULL, 5);
	xsrv->need_process = 1;
	return 1;
}

int ev_configure_request(struct xsrv *xsrv, struct client *client,
                         uint8_t stack_mode, struct window *parent,
                         struct window *window, struct window *sibling,
                         int16_t x, int16_t y, uint16_t width, uint16_t height,
                         uint16_t border_width, uint16_t value_mask)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, ConfigureRequest);
	CLIENT_WU8(client, stack_mode);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(parent));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU32(client, OBJECT_ID(sibling));
	CLIENT_WI16(client, x);
	CLIENT_WI16(client, y);
	CLIENT_WU16(client, width);
	CLIENT_WU16(client, height);
	CLIENT_WU16(client, border_width);
	CLIENT_WU16(client, value_mask);
	CLIENT_WRITE(client, NULL, 4);
	xsrv->need_process = 1;
	return 1;
}

int ev_gravity_notify(struct xsrv *xsrv, struct client *client,
                      struct window *event, struct window *window,
                      int16_t x, int16_t y)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, GravityNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WI16(client, x);
	CLIENT_WI16(client, y);
	CLIENT_WRITE(client, NULL, 16);
	xsrv->need_process = 1;
	return 1;
}

int ev_resize_request(struct xsrv *xsrv, struct client *client,
                      struct window *window, uint16_t width, uint16_t height)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, ResizeRequest);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU16(client, width);
	CLIENT_WU16(client, height);
	CLIENT_WRITE(client, NULL, 20);
	xsrv->need_process = 1;
	return 1;
}

int ev_circulate_notify(struct xsrv *xsrv, struct client *client,
                        struct window *event, struct window *window,
                        uint8_t place)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, CirculateNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(event));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU32(client, 0);
	CLIENT_WU8(client, place);
	CLIENT_WRITE(client, NULL, 15);
	xsrv->need_process = 1;
	return 1;
}

int ev_circulate_request(struct xsrv *xsrv, struct client *client,
                         struct window *parent, struct window *window,
                         uint8_t place)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, CirculateRequest);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(parent));
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU32(client, 0);
	CLIENT_WU8(client, place);
	CLIENT_WRITE(client, NULL, 15);
	xsrv->need_process = 1;
	return 1;
}

int ev_property_notify(struct xsrv *xsrv, struct client *client,
                       struct window *window, struct atom *atom, uint32_t time,
                       uint8_t state)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, PropertyNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU32(client, atom->id);
	CLIENT_WU32(client, time);
	CLIENT_WU8(client, state);
	CLIENT_WRITE(client, NULL, 15);
	xsrv->need_process = 1;
	return 1;
}

int ev_selection_clear(struct xsrv *xsrv, struct client *client, uint32_t time,
                       struct window *owner, struct atom *selection)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, SelectionClear);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(owner));
	CLIENT_WU32(client, selection->id);
	CLIENT_WRITE(client, NULL, 16);
	xsrv->need_process = 1;
	return 1;
}

int ev_selection_request(struct xsrv *xsrv, struct client *client,
                         uint32_t time, struct window *owner,
                         struct window *requestor, struct atom *selection,
                         struct atom *target, struct atom *property)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, SelectionRequest);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(owner));
	CLIENT_WU32(client, OBJECT_ID(requestor));
	CLIENT_WU32(client, selection->id);
	CLIENT_WU32(client, target->id);
	CLIENT_WU32(client, property ? property->id : None);
	CLIENT_WRITE(client, NULL, 4);
	xsrv->need_process = 1;
	return 1;
}

int ev_selection_notify(struct xsrv *xsrv, struct client *client,
                        uint32_t time, struct window *requestor,
                        struct atom *selection, struct atom *target,
                        struct atom *property)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, SelectionNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, time);
	CLIENT_WU32(client, OBJECT_ID(requestor));
	CLIENT_WU32(client, selection->id);
	CLIENT_WU32(client, target->id);
	CLIENT_WU32(client, property ? property->id : None);
	CLIENT_WRITE(client, NULL, 8);
	xsrv->need_process = 1;
	return 1;
}

int ev_colormap_notify(struct xsrv *xsrv, struct client *client,
                       struct window *window, struct colormap *colormap,
                       uint8_t new, uint8_t state)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, ColormapNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU32(client, OBJECT_ID(colormap));
	CLIENT_WU8(client, new);
	CLIENT_WU8(client, state);
	CLIENT_WRITE(client, NULL, 18);
	xsrv->need_process = 1;
	return 1;
}

int ev_client_message(struct xsrv *xsrv, struct client *client,
                      uint8_t format, struct window *window, struct atom *type,
                      uint8_t data_len, uint8_t *data)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, ClientMessage);
	CLIENT_WU8(client, format);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, OBJECT_ID(window));
	CLIENT_WU32(client, type->id);
	CLIENT_WRITE(client, data, data_len * (format / 8));
	xsrv->need_process = 1;
	return 1;
}

int ev_mapping_notify(struct xsrv *xsrv, struct client *client,
                      uint8_t request, uint8_t first_keycode, uint8_t count)
{
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, MappingNotify);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU8(client, request);
	CLIENT_WU8(client, first_keycode);
	CLIENT_WU8(client, count);
	CLIENT_WRITE(client, NULL, 25);
	xsrv->need_process = 1;
	return 1;
}

static int parse_window_attributes(struct xsrv *xsrv,
                                   struct client *client,
                                   struct request *request,
                                   struct window_attributes *attributes,
                                   uint32_t value_mask,
                                   struct window *parent,
                                   uint8_t depthv)
{
	uint32_t background_pixmap;
	uint32_t border_pixmap;
	uint32_t colormap;
	uint32_t cursor;
	attributes->background_pixmap = NULL;
	attributes->border_pixmap = NULL;
	attributes->colormap = NULL;
	attributes->cursor = NULL;
	VM_RU32(CWBackPixmap, background_pixmap, 0);
	VM_RU32(CWBackPixel, attributes->background_pixel, 0);
	VM_RU32(CWBorderPixmap, border_pixmap, 0);
	VM_RU32(CWBorderPixel, attributes->border_pixel, 0);
	VM_RU8(CWBitGravity, attributes->bit_gravity, ForgetGravity);
	VM_RU8(CWWinGravity, attributes->win_gravity, NorthWestGravity);
	VM_RU8(CWBackingStore, attributes->backing_store, NotUseful);
	VM_RU32(CWBackingPlanes, attributes->backing_planes, -1);
	VM_RU32(CWBackingPixel, attributes->backing_pixel, 0);
	VM_RU8(CWOverrideRedirect, attributes->override_redirect, 0);
	VM_RU8(CWSaveUnder, attributes->save_under, 0);
	VM_RU32(CWEventMask, attributes->event_mask, 0);
	VM_RU32(CWDontPropagate, attributes->do_not_propagate_mask, 0);
	VM_RU32(CWColormap, colormap, 0);
	VM_RU32(CWCursor, cursor, 0);
	if (attributes->bit_gravity > StaticGravity)
	{
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        attributes->bit_gravity);
	}
	if (attributes->win_gravity > StaticGravity)
	{
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        attributes->win_gravity);
	}
	if (attributes->backing_store > Always)
	{
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        attributes->backing_store);
	}
	if (background_pixmap)
	{
		attributes->background_pixmap = pixmap_get(xsrv, background_pixmap);
		if (!attributes->background_pixmap)
		{
			return CLIENT_BAD_PIXMAP(xsrv, client, request,
			                         background_pixmap);
		}
		if (attributes->background_pixmap->drawable.root != parent->drawable.root
		 || attributes->background_pixmap->drawable.format->depth != depthv)
		{
			return CLIENT_BAD_MATCH(xsrv, client, request);
		}
	}
	if (border_pixmap)
	{
		attributes->border_pixmap = pixmap_get(xsrv, border_pixmap);
		if (!attributes->border_pixmap)
		{
			return CLIENT_BAD_PIXMAP(xsrv, client, request,
			                         border_pixmap);
		}
		if (attributes->border_pixmap->drawable.root != parent->drawable.root
		 || attributes->border_pixmap->drawable.format->depth != depthv)
		{
			return CLIENT_BAD_MATCH(xsrv, client, request);
		}
	}
	if (colormap)
	{
		attributes->colormap = colormap_get(xsrv, colormap);
		if (!attributes->colormap)
		{
			return CLIENT_BAD_COLORMAP(xsrv, client, request,
			                           colormap);
		}
		/* XXX test for root & visual */
	}
	if (cursor)
	{
		attributes->cursor = cursor_get(xsrv, cursor);
		if (!attributes->cursor)
		{
			return CLIENT_BAD_CURSOR(xsrv, client, request, cursor);
		}
	}
	return 0;
}

static int req_create_window(struct xsrv *xsrv, struct client *client,
                             struct request *request)
{
	if (request->length < 8)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t depthv = request->detail;
	uint32_t wid = CLIENT_RU32(client);
	uint32_t parent_id = CLIENT_RU32(client);
	int16_t x = CLIENT_RU16(client);
	int16_t y = CLIENT_RI16(client);
	uint16_t width = CLIENT_RU16(client);
	uint16_t height = CLIENT_RU16(client);
	uint16_t border_width = CLIENT_RU16(client);
	uint16_t class = CLIENT_RU16(client);
	uint32_t visualid = CLIENT_RU32(client);
	uint32_t value_mask = CLIENT_RU32(client);
	uint32_t n = 0;
	for (size_t i = 0; i < 15; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	if (request->length != 8 + n)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (!client_has_free_id(xsrv, client, wid))
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, wid);
	struct window *parent = window_get(xsrv, parent_id);
	if (!parent)
		return CLIENT_BAD_WINDOW(xsrv, client, request, parent_id);
	if (!depthv)
	{
		depthv = parent->drawable.format->depth;
	}
	else if (depthv != parent->drawable.format->depth)
	{
		object_free(xsrv, OBJECT(parent));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	if (class > InputOnly)
	{
		object_free(xsrv, OBJECT(parent));
		return CLIENT_BAD_VALUE(xsrv, client, request, class);
	}
	if (class == CopyFromParent)
		class = parent->class;
	struct visual *visual;
	if (visualid)
	{
		visual = NULL;
		for (size_t i = 0; i < parent->screen->depths_count && !visual; ++i)
		{
			struct depth *depth = parent->screen->depths[i];
			if (depth->depth != depthv)
				continue;
			for (size_t j = 0; j < depth->visuals_count; ++j)
			{
				if (depth->visuals[j]->id == visualid)
				{
					visual = depth->visuals[j];
					break;
				}
			}
		}
		if (!visual)
		{
			object_free(xsrv, OBJECT(parent));
			return CLIENT_BAD_MATCH(xsrv, client, request);
		}
	}
	else
	{
		visual = parent->visual;
	}
	const struct format *format = xsrv_get_format(xsrv, depthv);
	if (!format)
	{
		object_free(xsrv, OBJECT(parent));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	struct window_attributes attributes;
	int ret = parse_window_attributes(xsrv, client, request, &attributes,
	                                  value_mask, parent, depthv);
	if (ret)
		goto err;
	struct window_event *event;
	if ((value_mask & CWEventMask) && attributes.event_mask)
	{
		event = malloc(sizeof(*event));
		if (!event)
		{
			object_free(xsrv, OBJECT(parent));
			return CLIENT_BAD_ALLOC(xsrv, client, request);
		}
		event->mask = attributes.event_mask;
	}
	else
	{
		event = NULL;
	}
	struct window *window = window_new(xsrv, client, wid, parent, x, y,
	                                   width, height, format, border_width,
	                                   visual, class, &attributes);
	if (!window)
	{
		free(event);
		ret = CLIENT_BAD_ALLOC(xsrv, client, request);
		goto err;
	}
	if (event)
	{
		event->client = client;
		event->window = window;
		TAILQ_INSERT_TAIL(&window->events, event, window_chain);
		TAILQ_INSERT_TAIL(&client->events, event, client_chain);
	}
	object_add(xsrv, &window->object);
	TAILQ_INSERT_TAIL(&client->objects, &window->object, client_chain);
	window_create_notify(xsrv, window);
	return 1;

err:
	object_free(xsrv, OBJECT(attributes.background_pixmap));
	object_free(xsrv, OBJECT(attributes.border_pixmap));
	object_free(xsrv, OBJECT(attributes.colormap));
	object_free(xsrv, OBJECT(attributes.cursor));
	object_free(xsrv, OBJECT(parent));
	return ret;
}

static int req_change_window_attributes(struct xsrv *xsrv,
                                        struct client *client,
                                        struct request *request)
{
	if (request->length < 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	uint32_t value_mask = CLIENT_RU32(client);
	uint32_t n = 0;
	for (size_t i = 0; i < 15; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	if (request->length != 3 + n)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	if (!window->parent && (value_mask & ~(CWEventMask | CWCursor)))
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	struct window_attributes attributes;
	int ret = parse_window_attributes(xsrv, client, request, &attributes,
	                                  value_mask, window->parent,
	                                  window->drawable.format->depth);
	if (ret)
		goto err;
	if (value_mask & CWEventMask)
	{
		struct window_event *event;
		TAILQ_FOREACH(event, &window->events, window_chain)
		{
			/* XXX still check SubstructureRedirectMask | ResizeRedirectMask | ButtonPressMask (but ignoring self client) */
			if (event->client != client)
				continue;
			if (attributes.event_mask)
			{
				event->mask = attributes.event_mask;
				break;
			}
			TAILQ_REMOVE(&event->window->events, event,
			             window_chain);
			TAILQ_REMOVE(&event->client->events, event,
			             client_chain);
			free(event);
			event = NULL;
			break;
		}
		if (!event && attributes.event_mask)
		{
			if (attributes.event_mask & (SubstructureRedirectMask | ResizeRedirectMask | ButtonPressMask))
			{
				uint32_t total_mask = 0;
				TAILQ_FOREACH(event, &window->events, window_chain)
					total_mask |= event->mask;
				if (total_mask & attributes.event_mask & (SubstructureRedirectMask | ResizeRedirectMask | ButtonPressMask))
				{
					object_free(xsrv, OBJECT(window));
					return CLIENT_BAD_ACCESS(xsrv, client, request);
				}
			}
			event = malloc(sizeof(*event));
			if (!event)
			{
				object_free(xsrv, OBJECT(window));
				return CLIENT_BAD_ALLOC(xsrv, client, request);
			}
			event->mask = attributes.event_mask;
			event->client = client;
			event->window = window;
			TAILQ_INSERT_TAIL(&window->events, event, window_chain);
			TAILQ_INSERT_TAIL(&client->events, event, client_chain);
		}
	}
	if (value_mask & CWCursor)
		window_set_cursor(xsrv, window, attributes.cursor);
	if (value_mask & CWBorderPixel)
		window->attributes.border_pixel = attributes.border_pixel;
	/* XXX other attributes */
	object_free(xsrv, OBJECT(window));
	return 1;

err:
	object_free(xsrv, OBJECT(attributes.background_pixmap));
	object_free(xsrv, OBJECT(attributes.border_pixmap));
	object_free(xsrv, OBJECT(attributes.colormap));
	object_free(xsrv, OBJECT(attributes.cursor));
	object_free(xsrv, OBJECT(window));
	return ret;
}

static int req_get_window_attributes(struct xsrv *xsrv, struct client *client,
                                     struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	uint16_t length = 3;
	if (ringbuf_write_size(&client->wbuf) < 32u + length * 4u)
	{
		object_free(xsrv, OBJECT(window));
		return -1;
	}
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, window->attributes.backing_store);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, length);
	CLIENT_WU32(client, window->visual->id);
	CLIENT_WU16(client, window->class);
	CLIENT_WU8(client, window->attributes.bit_gravity);
	CLIENT_WU8(client, window->attributes.win_gravity);
	CLIENT_WU32(client, window->attributes.backing_planes);
	CLIENT_WU32(client, window->attributes.backing_pixel);
	CLIENT_WU8(client, window->attributes.save_under);
	CLIENT_WU8(client, 0); /* XXX */
	CLIENT_WU8(client, window_visible(window) ? IsViewable : ((window->flags & WINDOW_MAPPED) ? IsUnviewable : IsUnmapped));
	CLIENT_WU8(client, window->attributes.override_redirect);
	CLIENT_WU32(client, OBJECT_ID(window->attributes.colormap));
	uint32_t your_mask = 0;
	uint32_t total_mask = 0;
	struct window_event *event;
	TAILQ_FOREACH(event, &window->events, window_chain)
	{
		if (event->client == client)
			your_mask = event->mask;
		total_mask |= event->mask;
	}
	CLIENT_WU32(client, total_mask);
	CLIENT_WU32(client, your_mask);
	CLIENT_WU32(client, window->attributes.do_not_propagate_mask);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_destroy_window(struct xsrv *xsrv, struct client *client,
                              struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	if (!window->parent)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	object_destroy(xsrv, OBJECT(window));
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_destroy_subwindows(struct xsrv *xsrv, struct client *client,
                                  struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	struct window *child = TAILQ_FIRST(&window->children);
	while (child)
	{
		object_destroy(xsrv, OBJECT(child));
		child = TAILQ_FIRST(&window->children);
	}
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_reparent_window(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	if (request->length != 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	uint32_t parent_id = CLIENT_RU32(client);
	int16_t x = CLIENT_RI16(client);
	int16_t y = CLIENT_RI16(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	if (!window->parent)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	struct window *parent = window_get(xsrv, parent_id);
	if (!parent)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_WINDOW(xsrv, client, request, parent_id);
	}
	if (parent->tree_depth > window->tree_depth)
	{
		struct window *it = parent;
		while (it->tree_depth > window->tree_depth)
			it = it->parent;
		if (it == window)
		{
			object_free(xsrv, OBJECT(window));
			object_free(xsrv, OBJECT(parent));
			return CLIENT_BAD_MATCH(xsrv, client, request);
		}
	}
	window_reparent(xsrv, client, window, parent, x, y);
	object_free(xsrv, OBJECT(window));
	object_free(xsrv, OBJECT(parent));
	return 1;
}

static int req_map_window(struct xsrv *xsrv, struct client *client,
                          struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	window_map(xsrv, client, window);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_map_subwindows(struct xsrv *xsrv, struct client *client,
                              struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	struct window *child;
	TAILQ_FOREACH(child, &window->children, chain)
		window_map(xsrv, client, child);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_unmap_window(struct xsrv *xsrv, struct client *client,
                            struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	if (!window->parent)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	window_unmap(xsrv, window);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_unmap_subwindows(struct xsrv *xsrv, struct client *client,
                                struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	struct window *child;
	TAILQ_FOREACH_REVERSE(child, &window->children, window_head, chain)
		window_unmap(xsrv, child);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_configure_window(struct xsrv *xsrv, struct client *client,
                                struct request *request)
{
	if (request->length < 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	uint16_t value_mask = CLIENT_RU16(client);
	CLIENT_READ(client, NULL, 2);
	size_t n = 0;
	for (size_t i = 0; i < 7; ++i)
	{
		if (value_mask & (1 << i))
			n++;
	}
	if (request->length != 3 + n)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	if (!window->parent)
	{
		object_free(xsrv, OBJECT(window));
		return 1;
	}
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t border_width;
	uint32_t sibling_id;
	uint8_t stack_mode;
	VM_RI16(CWX, x, 0);
	VM_RI16(CWY, y, 0);
	VM_RU16(CWWidth, width, window->drawable.width);
	VM_RU16(CWHeight, height, window->drawable.height);
	VM_RU16(CWBorderWidth, border_width, 0);
	VM_RU32(CWSibling, sibling_id, 0);
	VM_RU8(CWStackMode, stack_mode, 0);
	struct window *sibling;
	if (value_mask & CWSibling)
	{
		sibling = window_get(xsrv, sibling_id);
		if (!sibling)
		{
			object_free(xsrv, OBJECT(window));
			return CLIENT_BAD_WINDOW(xsrv, client, request,
			                         sibling_id);
		}
	}
	else
	{
		sibling = NULL;
	}
	if (!window->attributes.override_redirect
	 && window_configure_request(xsrv, client, window, value_mask, x, y,
	                             width, height, border_width, sibling,
	                             stack_mode))
	{
		object_free(xsrv, OBJECT(sibling));
		object_free(xsrv, OBJECT(window));
		return 1;
	}
	if ((value_mask & (CWStackMode | CWSibling)) == CWSibling)
	{
		object_free(xsrv, OBJECT(sibling));
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	if ((value_mask & (CWWidth | CWHeight))
	 && window_resize_request(xsrv, client, window, width, height))
		value_mask &= ~(CWWidth | CWHeight);
	if ((value_mask & CWStackMode) && stack_mode > Opposite)
	{
		object_free(xsrv, OBJECT(sibling));
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_VALUE(xsrv, client, request, stack_mode);
	}
	if (value_mask & (CWX | CWY | CWWidth | CWHeight | CWBorderWidth))
	{
		uint32_t old_x;
		uint32_t old_y;
		uint32_t old_width;
		uint32_t old_height;
		window_get_full_rect(xsrv, window, &old_x, &old_y,
		                     &old_width, &old_height);
		if (value_mask & CWX)
			window->x = x;
		if (value_mask & CWY)
			window->y = y;
		if (value_mask & (CWWidth | CWHeight))
			window_resize(xsrv, window, width, height);
		if (value_mask & CWBorderWidth)
			window->border_width = border_width;
		uint32_t new_x;
		uint32_t new_y;
		uint32_t new_width;
		uint32_t new_height;
		window_get_full_rect(xsrv, window, &new_x, &new_y,
		                     &new_width, &new_height);
		if (old_x < new_x)
			framebuffer_redraw(xsrv, old_x, old_y, new_x - old_x, old_height);
		else if (old_x > new_x)
			framebuffer_redraw(xsrv, new_x + new_width, old_y, (old_x + old_width) - (new_x + new_width), old_height);
		else if (old_width > new_width)
			framebuffer_redraw(xsrv, new_x + new_width, old_y, old_width - new_width, old_height);
		if (old_y < new_y)
			framebuffer_redraw(xsrv, old_x, old_y, old_width, new_y - old_y);
		else if (old_y > new_y)
			framebuffer_redraw(xsrv, old_x, new_y + new_height, old_width, (old_y + old_height) - (new_y + new_height));
		else if (old_height > new_height)
			framebuffer_redraw(xsrv, old_x, old_y + new_height, old_width, old_height - new_height);
		framebuffer_redraw(xsrv, new_x, new_y, new_width, new_height);
	}
	if (value_mask & CWStackMode)
	{
		switch (stack_mode)
		{
			case Above:
				if (!TAILQ_PREV(window, window_head, chain))
					break;
				TAILQ_REMOVE(&window->parent->children,
				             window, chain);
				TAILQ_INSERT_HEAD(&window->parent->children,
				                  window, chain);
				window_redraw(xsrv, window);
				break;
			case Below:
				if (!TAILQ_NEXT(window, chain))
					break;
				TAILQ_REMOVE(&window->parent->children,
				             window, chain);
				TAILQ_INSERT_TAIL(&window->parent->children,
				                  window, chain);
				window_redraw(xsrv, window);
				break;
			case TopIf:
			{
				/* XXX */
				break;
			}
			case BottomIf:
			{
				/* XXX */
				break;
			}
			default: /* XXX */
				break;
		}
	}
	window_configure_notify(xsrv, window);
	object_free(xsrv, OBJECT(sibling));
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_get_geometry(struct xsrv *xsrv, struct client *client,
                            struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t drawable_id = CLIENT_RU32(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	if (ringbuf_write_size(&client->wbuf) < 32)
	{
		object_free(xsrv, OBJECT(drawable));
		return -1;
	}
	struct window *window;
	if (drawable->object.type == xsrv->obj_window)
		window = (struct window*)drawable;
	else
		window = NULL;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, drawable->format->depth);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU32(client, OBJECT_ID(drawable->root));
	CLIENT_WI16(client, window ? window->x : 0);
	CLIENT_WI16(client, window ? window->y : 0);
	CLIENT_WU16(client, drawable->width);
	CLIENT_WU16(client, drawable->height);
	CLIENT_WU16(client, window ? window->border_width : 0);
	CLIENT_WRITE(client, NULL, 10);
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_query_tree(struct xsrv *xsrv, struct client *client,
                          struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	size_t nchildren = 0;
	struct window *child;
	TAILQ_FOREACH(child, &window->children, chain) /* XXX O(1) */
		nchildren++;
	if (ringbuf_write_size(&client->wbuf) < 32 + nchildren)
	{
		object_free(xsrv, OBJECT(window));
		return -1;
	}
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, nchildren);
	CLIENT_WU32(client, OBJECT_ID(window->drawable.root));
	CLIENT_WU32(client, OBJECT_ID(window->parent));
	CLIENT_WU16(client, nchildren);
	CLIENT_WRITE(client, NULL, 14);
	TAILQ_FOREACH(child, &window->children, chain)
		CLIENT_WU32(client, OBJECT_ID(child));
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_intern_atom(struct xsrv *xsrv, struct client *client,
                           struct request *request)
{
	if (request->length < 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t only_if_exists = request->detail;
	uint16_t name_len = CLIENT_RU16(client);
	CLIENT_READ(client, NULL, 2);
	if (request->length != 2 + (name_len + 3) / 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	char *name = malloc(name_len + 1);
	if (!name)
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	CLIENT_READ(client, name, name_len);
	CLIENT_RPAD(client, name_len);
	name[name_len] = '\0';
	struct atom *atom = atom_get(xsrv, name);
	if (!only_if_exists)
	{
		atom = atom_new(xsrv, name);
		if (!atom)
		{
			free(name);
			return CLIENT_BAD_ALLOC(xsrv, client, request);
		}
	}
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU32(client, atom ? atom->id : 0);
	CLIENT_WRITE(client, NULL, 20);
	return 1;
}

static int req_get_atom_name(struct xsrv *xsrv, struct client *client,
                             struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t atom_id = CLIENT_RU32(client);
	struct atom *atom = atom_get_id(xsrv, atom_id);
	if (!atom)
		return CLIENT_BAD_ATOM(xsrv, client, request, atom_id);
	uint32_t length = (atom->name_len + 3) / 4;
	if (ringbuf_write_size(&client->wbuf) < 32 + length * 4)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, length);
	CLIENT_WU16(client, atom->name_len);
	CLIENT_WRITE(client, NULL, 22);
	CLIENT_WRITE(client, atom->name, atom->name_len);
	CLIENT_WPAD(client, atom->name_len);
	return 1;
}

static int req_change_property(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	if (request->length < 6)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t mode = request->detail;
	if (mode > PropModeAppend)
		return CLIENT_BAD_VALUE(xsrv, client, request, mode);
	uint32_t window_id = CLIENT_RU32(client);
	uint32_t property_id = CLIENT_RU32(client);
	uint32_t type_id = CLIENT_RU32(client);
	uint8_t format = CLIENT_RU8(client);
	CLIENT_READ(client, NULL, 3);
	uint32_t data_len = CLIENT_RU32(client);
	if (format != 8 && format != 16 && format != 32)
		return CLIENT_BAD_VALUE(xsrv, client, request, format);
	uint32_t data_bytes = format / 8 * data_len;
	if (request->length != 6 + (data_bytes + 3) / 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	struct atom *property = atom_get_id(xsrv, property_id);
	if (!property)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_ATOM(xsrv, client, request, property_id);
	}
	struct atom *type = atom_get_id(xsrv, type_id);
	if (!type)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_ATOM(xsrv, client, request, type_id);
	}
	/* can't be more than 256k because of request length */
	void *data = malloc(data_bytes + 1); /* +1 to avoid 0 */
	if (!data)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	}
	switch (format)
	{
		case 8:
			CLIENT_READ(client, data, data_bytes);
			break;
		case 16:
			for (size_t i = 0; i < data_len; ++i)
				((uint16_t*)data)[i] = CLIENT_RU16(client);
			break;
		case 32:
			for (size_t i = 0; i < data_len; ++i)
				((uint32_t*)data)[i] = CLIENT_RU32(client);
			break;
	}
	CLIENT_RPAD(client, data_bytes);
	/* XXX move to window function */
	struct property *prop = window_get_property(window, property);
	if (!prop)
	{
		/* XXX move to window function */
		prop = malloc(sizeof(*prop));
		if (!prop)
		{
			free(data);
			object_free(xsrv, OBJECT(window));
			return CLIENT_BAD_ALLOC(xsrv, client, request);
		}
		prop->property = property;
		prop->type = type;
		prop->format = format;
		prop->data_bytes = data_bytes;
		prop->data_len = data_len;
		prop->data = data;
		TAILQ_INSERT_TAIL(&window->properties, prop, chain);
		window_property_notify(xsrv, window, prop->property,
		                       PropertyNewValue);
		object_free(xsrv, OBJECT(window));
		return 1;
	}
	if (mode == PropModeReplace)
	{
		free(prop->data);
		prop->property = property;
		prop->type = type;
		prop->format = format;
		prop->data_bytes = data_bytes;
		prop->data_len = data_len;
		prop->data = data;
		window_property_notify(xsrv, window, prop->property,
		                       PropertyNewValue);
		object_free(xsrv, OBJECT(window));
		return 1;
	}
	if (prop->type != type || prop->format != format)
	{
		free(data);
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	uint32_t new_data_bytes = data_bytes + prop->data_bytes;
	uint32_t new_data_len = data_len + prop->data_len;
	uint8_t *new_data = malloc(new_data_bytes + 1);
	if (!new_data)
	{
		free(data);
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	}
	if (mode == PropModePrepend)
	{
		memcpy(&new_data[0], data, data_bytes);
		memcpy(&new_data[data_bytes], prop->data, prop->data_bytes);
	}
	else
	{
		memcpy(&new_data[0], prop->data, prop->data_bytes);
		memcpy(&new_data[prop->data_bytes], data, data_bytes);
	}
	free(prop->data);
	free(data);
	prop->data_len = new_data_len;
	prop->data_bytes = new_data_bytes;
	prop->data = new_data;
	window_property_notify(xsrv, window, prop->property, PropertyNewValue);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_delete_property(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	if (request->length != 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	uint32_t property_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	struct atom *property = atom_get_id(xsrv, property_id);
	if (!property)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_ATOM(xsrv, client, request, property_id);
	}
	window_delete_property(xsrv, window, property);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_get_property(struct xsrv *xsrv, struct client *client,
                            struct request *request)
{
	if (request->length != 6)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t delete = request->detail;
	uint32_t window_id = CLIENT_RU32(client);
	uint32_t property_id = CLIENT_RU32(client);
	uint32_t type_id = CLIENT_RU32(client);
	uint32_t long_offset = CLIENT_RU32(client);
	uint32_t long_length = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	struct atom *property = atom_get_id(xsrv, property_id);
	if (!property)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_ATOM(xsrv, client, request, property_id);
	}
	struct atom *type;
	if (type_id)
	{
		type = atom_get_id(xsrv, type_id);
		if (!type)
		{
			object_free(xsrv, OBJECT(window));
			return CLIENT_BAD_ATOM(xsrv, client, request, type_id);
		}
	}
	else
	{
		type = NULL;
	}
	struct property *prop = window_get_property(window, property);
	if (!prop)
	{
		if (ringbuf_write_size(&client->wbuf) < 32)
		{
			object_free(xsrv, OBJECT(window));
			return -1;
		}
		CLIENT_WU8(client, 1);
		CLIENT_WU8(client, 0);
		CLIENT_WU16(client, client->sequence);
		CLIENT_WRITE(client, NULL, 28);
		object_free(xsrv, OBJECT(window));
		return 1;
	}
	if (type && prop->type != type)
	{
		if (ringbuf_write_size(&client->wbuf) < 32)
		{
			object_free(xsrv, OBJECT(window));
			return -1;
		}
		CLIENT_WU8(client, 1);
		CLIENT_WU8(client, prop->format);
		CLIENT_WU16(client, client->sequence);
		CLIENT_WU32(client, 0);
		CLIENT_WU32(client, prop->type->id);
		CLIENT_WU32(client, prop->data_bytes);
		CLIENT_WU32(client, prop->data_len);
		CLIENT_WRITE(client, NULL, 12);
		object_free(xsrv, OBJECT(window));
		return 1;
	}
	if (long_offset * 4 > prop->data_bytes)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_VALUE(xsrv, client, request, long_offset);
	}
	uint32_t bytes = prop->data_bytes - long_offset * 4;
	uint32_t bytes_after;
	if (long_length * 4 < bytes)
	{
		bytes_after = bytes - long_length * 4;
		bytes = long_length * 4;
	}
	else
	{
		bytes_after = 0;
	}
	uint32_t length = (bytes + 3) / 4;
	if (ringbuf_write_size(&client->wbuf) < 32 + length * 4)
	{
		object_free(xsrv, OBJECT(window));
		return -1;
	}
	uint32_t data_len = bytes / (prop->format / 8);
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, prop->format);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, length);
	CLIENT_WU32(client, prop->type->id);
	CLIENT_WU32(client, bytes_after);
	CLIENT_WU32(client, data_len);
	CLIENT_WRITE(client, NULL, 12);
	uint8_t *base = &prop->data8[long_offset * 4];
	switch (prop->format)
	{
		case 8:
			CLIENT_WRITE(client, base, bytes);
			break;
		case 16:
			for (size_t i = 0; i < data_len; ++i)
				CLIENT_WU16(client, ((uint16_t*)base)[i]);
			break;
		case 32:
			for (size_t i = 0; i < data_len; ++i)
				CLIENT_WU32(client, ((uint32_t*)base)[i]);
			break;
	}
	CLIENT_WPAD(client, bytes);
	if (!bytes_after && delete)
		window_delete_property(xsrv, window, property);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_list_properties(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	uint32_t nproperties = 0;
	struct property *property;
	TAILQ_FOREACH(property, &window->properties, chain)
		nproperties++;
	uint32_t length = nproperties;
	if (ringbuf_write_size(&client->wbuf) < 32 + nproperties * 4)
	{
		object_free(xsrv, OBJECT(window));
		return -1;
	}
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, length);
	CLIENT_WU16(client, nproperties);
	CLIENT_WRITE(client, NULL, 22);
	TAILQ_FOREACH(property, &window->properties, chain)
		CLIENT_WU32(client, property->property->id);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_grab_pointer(struct xsrv *xsrv, struct client *client,
                            struct request *request)
{
	if (request->length != 6)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t owner_events = request->detail;
	uint32_t window_id = CLIENT_RU32(client);
	uint16_t event_mask = CLIENT_RU16(client);
	uint8_t pointer_mode = CLIENT_RU8(client);
	uint8_t keyboard_mode = CLIENT_RU8(client);
	uint32_t confine_to_id = CLIENT_RU32(client);
	uint32_t cursor_id = CLIENT_RU32(client);
	uint32_t time = CLIENT_RU32(client);
	if (pointer_mode > GrabModeAsync)
		return CLIENT_BAD_VALUE(xsrv, client, request, pointer_mode);
	if (keyboard_mode > GrabModeAsync)
		return CLIENT_BAD_VALUE(xsrv, client, request, keyboard_mode);
	if (pointer_mode == GrabModeSync) /* XXX */
		return CLIENT_BAD_IMPLEM(xsrv, client, request);
	if (keyboard_mode == GrabModeSync) /* XXX */
		return CLIENT_BAD_IMPLEM(xsrv, client, request);
	if (confine_to_id) /* XXX */
		return CLIENT_BAD_IMPLEM(xsrv, client, request);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	struct window *confine_to;
	if (confine_to_id)
	{
		confine_to = window_get(xsrv, confine_to_id);
		if (!confine_to)
		{
			object_free(xsrv, OBJECT(window));
			return CLIENT_BAD_WINDOW(xsrv, client, request,
			                         confine_to_id);
		}
	}
	else
	{
		confine_to = NULL;
	}
	struct cursor *cursor;
	if (cursor_id)
	{
		cursor = cursor_get(xsrv, cursor_id);
		if (!cursor)
		{
			object_free(xsrv, OBJECT(confine_to));
			object_free(xsrv, OBJECT(window));
			return CLIENT_BAD_CURSOR(xsrv, client, request,
			                         cursor_id);
		}
	}
	else
	{
		cursor = NULL;
	}
	if (xsrv->pointer_grab.client && xsrv->pointer_grab.client != client)
	{
		object_free(xsrv, OBJECT(confine_to));
		object_free(xsrv, OBJECT(window));
		object_free(xsrv, OBJECT(cursor));
		if (ringbuf_write_size(&client->wbuf) < 32)
			return -1;
		CLIENT_WU8(client, 1);
		CLIENT_WU8(client, AlreadyGrabbed);
		CLIENT_WU16(client, client->sequence);
		CLIENT_WU32(client, 0);
		CLIENT_WRITE(client, NULL, 24);
		return 1;
	}
	if (!window_visible(window)
	 || (confine_to && !window_visible(confine_to)))
	{
		object_free(xsrv, OBJECT(confine_to));
		object_free(xsrv, OBJECT(window));
		object_free(xsrv, OBJECT(cursor));
		if (ringbuf_write_size(&client->wbuf) < 32)
			return -1;
		CLIENT_WU8(client, 1);
		CLIENT_WU8(client, NotViewable);
		CLIENT_WU16(client, client->sequence);
		CLIENT_WU32(client, 0);
		CLIENT_WRITE(client, NULL, 24);
		return 1;
	}
	/* XXX test for confine_to out of root bounds */
	if (time)
	{
		if (time < xsrv->pointer_grab.time || time > millitime())
		{
			object_free(xsrv, OBJECT(confine_to));
			object_free(xsrv, OBJECT(window));
			object_free(xsrv, OBJECT(cursor));
			if (ringbuf_write_size(&client->wbuf) < 32)
				return -1;
			CLIENT_WU8(client, 1);
			CLIENT_WU8(client, InvalidTime);
			CLIENT_WU16(client, client->sequence);
			CLIENT_WU32(client, 0);
			CLIENT_WRITE(client, NULL, 24);
			return 1;
		}
	}
	else
	{
		time = millitime();
	}
	xsrv_grab_pointer(xsrv, client, window, confine_to, cursor, event_mask,
	                  pointer_mode, keyboard_mode, owner_events, time);
	object_free(xsrv, OBJECT(confine_to));
	object_free(xsrv, OBJECT(cursor));
	object_free(xsrv, OBJECT(window));
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, Success);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WRITE(client, NULL, 24);
	return 1;
}

static int req_ungrab_pointer(struct xsrv *xsrv, struct client *client,
                              struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t time = CLIENT_RU32(client);
	if (xsrv->pointer_grab.client != client)
		return 1;
	if (time)
	{
		if (time < xsrv->pointer_grab.time || time > millitime())
			return 1;
	}
	else
	{
		time = millitime();
	}
	xsrv_ungrab_pointer(xsrv);
	return 1;
}

static int req_grab_button(struct xsrv *xsrv, struct client *client,
                           struct request *request)
{
	if (request->length != 6)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t owner_events = request->detail;
	uint32_t window_id = CLIENT_RU32(client);
	uint16_t event_mask = CLIENT_RU16(client);
	uint8_t pointer_mode = CLIENT_RU8(client);
	uint8_t keyboard_mode = CLIENT_RU8(client);
	uint32_t confine_to_id = CLIENT_RU32(client);
	uint32_t cursor_id = CLIENT_RU32(client);
	uint8_t button = CLIENT_RU8(client);
	CLIENT_READ(client, NULL, 1);
	uint16_t modifiers = CLIENT_RU16(client);
	if (pointer_mode > GrabModeAsync)
		return CLIENT_BAD_VALUE(xsrv, client, request, pointer_mode);
	if (keyboard_mode > GrabModeAsync)
		return CLIENT_BAD_VALUE(xsrv, client, request, keyboard_mode);
	if (button == AnyButton)
		return CLIENT_BAD_VALUE(xsrv, client, request, button);
	if (pointer_mode == GrabModeSync) /* XXX */
		return CLIENT_BAD_IMPLEM(xsrv, client, request);
	if (keyboard_mode == GrabModeSync) /* XXX */
		return CLIENT_BAD_IMPLEM(xsrv, client, request);
	if (confine_to_id) /* XXX */
		return CLIENT_BAD_IMPLEM(xsrv, client, request);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	struct window *confine_to;
	if (confine_to_id)
	{
		confine_to = window_get(xsrv, confine_to_id);
		if (!confine_to)
		{
			object_free(xsrv, OBJECT(window));
			return CLIENT_BAD_WINDOW(xsrv, client, request,
			                         confine_to_id);
		}
	}
	else
	{
		confine_to = NULL;
	}
	struct cursor *cursor;
	if (cursor_id)
	{
		cursor = cursor_get(xsrv, cursor_id);
		if (!cursor)
		{
			object_free(xsrv, OBJECT(confine_to));
			object_free(xsrv, OBJECT(window));
			return CLIENT_BAD_CURSOR(xsrv, client, request,
			                         cursor_id);
		}
	}
	else
	{
		cursor = NULL;
	}
	struct button_grab *button_grab = window_get_button_grab(xsrv, window,
	                                                         button,
	                                                         modifiers);
	if (button_grab)
	{
		object_free(xsrv, OBJECT(confine_to));
		object_free(xsrv, OBJECT(window));
		object_free(xsrv, OBJECT(cursor));
		return CLIENT_BAD_ACCESS(xsrv, client, request);
	}
	button_grab = malloc(sizeof(*button_grab));
	if (!button_grab)
	{
		object_free(xsrv, OBJECT(confine_to));
		object_free(xsrv, OBJECT(window));
		object_free(xsrv, OBJECT(cursor));
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	}
	button_grab->modifiers = modifiers;
	button_grab->button = button;
	button_grab->pointer_grab.client = client;
	button_grab->pointer_grab.window = window;
	button_grab->pointer_grab.confine_to = confine_to;
	button_grab->pointer_grab.cursor = cursor;
	button_grab->pointer_grab.event_mask = event_mask;
	button_grab->pointer_grab.pointer_mode = pointer_mode;
	button_grab->pointer_grab.keyboard_mode = keyboard_mode;
	button_grab->pointer_grab.owner_events = owner_events;
	button_grab->pointer_grab.time = CurrentTime;
	TAILQ_INSERT_TAIL(&client->button_grabs, button_grab, client_chain);
	TAILQ_INSERT_TAIL(&window->button_grabs, button_grab, window_chain);
	return 1;
}

static int req_ungrab_button(struct xsrv *xsrv, struct client *client,
                             struct request *request)
{
	if (request->length != 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t button = request->detail;
	uint32_t window_id = CLIENT_RU32(client);
	uint16_t modifiers = CLIENT_RU16(client);
	CLIENT_READ(client, NULL, 2);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	window_remove_button_grab(xsrv, window, button, modifiers);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_grab_server(struct xsrv *xsrv, struct client *client,
                           struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (xsrv->grab_client && xsrv->grab_client != client)
	{
		fprintf(stderr, "%s: double grab\n", xsrv->progname);
		abort();
	}
	xsrv->grab_client = client;
	xsrv->grab_client_nb++;
	return 1;
}

static int req_ungrab_server(struct xsrv *xsrv, struct client *client,
                             struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (!xsrv->grab_client_nb)
		return 1;
	if (xsrv->grab_client != client)
	{
		fprintf(stderr, "%s: ungrab from somewhere else\n",
		        xsrv->progname);
		abort();
	}
	xsrv->grab_client_nb--;
	if (!xsrv->grab_client_nb)
		xsrv->grab_client = NULL;
	return 1;
}

static int req_query_pointer(struct xsrv *xsrv, struct client *client,
                             struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t window_id = CLIENT_RU32(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	if (ringbuf_write_size(&client->wbuf) < 32)
	{
		object_free(xsrv, OBJECT(window));
		return -1;
	}
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 1); /* XXX */
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU32(client, OBJECT_ID(window->drawable.root));
	CLIENT_WU32(client, OBJECT_ID(window->parent));
	CLIENT_WI16(client, xsrv->pointer.x);
	CLIENT_WI16(client, xsrv->pointer.y);
	CLIENT_WI16(client, xsrv->pointer.x - window->x);
	CLIENT_WI16(client, xsrv->pointer.y - window->y);
	CLIENT_WU16(client, xsrv->keybutmask);
	CLIENT_WRITE(client, NULL, 6);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_warp_pointer(struct xsrv *xsrv, struct client *client,
                            struct request *request)
{
	if (request->length != 6)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t src_id = CLIENT_RU32(client);
	uint32_t dst_id = CLIENT_RU32(client);
	int16_t src_x = CLIENT_RI16(client);
	int16_t src_y = CLIENT_RI16(client);
	uint16_t src_width = CLIENT_RU16(client);
	uint16_t src_height = CLIENT_RU16(client);
	int16_t dst_x = CLIENT_RI16(client);
	int16_t dst_y = CLIENT_RI16(client);
	/* XXX */
	(void)src_id;
	(void)dst_id;
	(void)src_x;
	(void)src_y;
	(void)src_width;
	(void)src_height;
	(void)dst_x;
	(void)dst_y;
	return 1;
}

static int req_set_input_focus(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	if (request->length != 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t revert_to = request->detail;
	uint32_t window_id = CLIENT_RU32(client);
	uint32_t timestamp = CLIENT_RU32(client);
	struct window *window;
	if (window_id)
	{
		window = window_get(xsrv, window_id);
		if (!window)
			return CLIENT_BAD_WINDOW(xsrv, client, request,
			                         window_id);
		if (!window_visible(window))
		{
			object_free(xsrv, OBJECT(window));
			return CLIENT_BAD_MATCH(xsrv, client, request);
		}
	}
	else
	{
		window = NULL;
	}
	if (timestamp == CurrentTime)
	{
		timestamp = millitime();
	}
	else if (timestamp < xsrv->focus.timestamp || timestamp > millitime())
	{
		object_free(xsrv, OBJECT(window));
		return 1;
	}
	xsrv_set_focus(xsrv, window, revert_to, timestamp);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_get_input_focus(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, xsrv->focus.revert_to);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU32(client, OBJECT_ID(xsrv->focus.window));
	CLIENT_WU32(client, xsrv->focus.timestamp);
	CLIENT_WRITE(client, NULL, 16);
	return 1;
}

static int req_open_font(struct xsrv *xsrv, struct client *client,
                         struct request *request)
{
	if (request->length < 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t fid = CLIENT_RU32(client);
	uint16_t name_len = CLIENT_RU16(client);
	CLIENT_READ(client, NULL, 2);
	if (request->length != 3 + (name_len + 3) / 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	char *name = malloc(name_len);
	if (!name)
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	CLIENT_READ(client, name, name_len);
	CLIENT_RPAD(client, name_len);
	if (!client_has_free_id(xsrv, client, fid))
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, fid);
	struct font_def *font_def = NULL;
	for (size_t i = 0; i < xsrv->fonts_count; ++i)
	{
		struct font_def *it = xsrv->fonts[i];
		if (it->name_len == name_len
		 && !memcmp(it->name, name, name_len))
		{
			font_def = it;
			break;
		}
	}
	if (!font_def)
		return CLIENT_BAD_NAME(xsrv, client, request);
	struct font *font = font_new(xsrv, client, fid, font_def);
	if (!font)
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	object_add(xsrv, &font->object);
	TAILQ_INSERT_TAIL(&client->objects, &font->object, client_chain);
	return 1;
}

static int req_close_font(struct xsrv *xsrv, struct client *client,
                          struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t font_id = CLIENT_RU32(client);
	struct font *font = font_get(xsrv, font_id);
	if (!font)
		return CLIENT_BAD_FONT(xsrv, client, request, font_id);
	object_destroy(xsrv, OBJECT(font));
	object_free(xsrv, OBJECT(font));
	return 1;
}

static int req_list_fonts(struct xsrv *xsrv, struct client *client,
                          struct request *request)
{
	if (request->length < 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint16_t max_names = CLIENT_RU16(client);
	uint16_t pattern_len = CLIENT_RU16(client);
	if (request->length != 2 + (pattern_len + 3) / 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	char *pattern = malloc(pattern_len);
	if (!pattern)
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	CLIENT_READ(client, pattern, pattern_len);
	CLIENT_RPAD(client, pattern_len);
	/* XXX use pattern */
	uint32_t length = 0;
	for (size_t i = 0; i < xsrv->fonts_count && i < max_names; ++i)
		length += 1 + xsrv->fonts[i]->name_len;
	if (ringbuf_write_size(&client->wbuf) < 32 + length)
	{
		free(pattern);
		return -1;
	}
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, (length + 3) / 4);
	CLIENT_WU16(client, xsrv->fonts_count);
	CLIENT_WRITE(client, NULL, 22);
	for (size_t i = 0; i < xsrv->fonts_count && i < max_names; ++i)
	{
		struct font_def *font = xsrv->fonts[i];
		CLIENT_WU8(client, font->name_len);
		CLIENT_WRITE(client, font->name, font->name_len);
	}
	CLIENT_WPAD(client, length);
	free(pattern);
	return 1;
}

static int req_create_pixmap(struct xsrv *xsrv, struct client *client,
                             struct request *request)
{
	if (request->length != 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t pid = CLIENT_RU32(client);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint16_t width = CLIENT_RU16(client);
	uint16_t height = CLIENT_RU16(client);
	uint8_t depthv = request->detail;
	if (!width || !height)
		return CLIENT_BAD_VALUE(xsrv, client, request, 0);
	if (!client_has_free_id(xsrv, client, pid))
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, pid);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct window *root = drawable->root;
	struct depth *depth = NULL;
	for (size_t i = 0; i < root->screen->depths_count; ++i)
	{
		struct depth *tmp = root->screen->depths[i];
		if (tmp->depth == depthv)
		{
			depth = tmp;
			break;
		}
	}
	if (!depth)
		return CLIENT_BAD_VALUE(xsrv, client, request, depthv);
	const struct format *format = xsrv_get_format(xsrv, depthv);
	if (!format)
		return CLIENT_BAD_MATCH(xsrv, client, request);
	struct pixmap *pixmap = pixmap_new(xsrv, client, pid, width, height,
	                                   format, drawable->root);
	object_free(xsrv, OBJECT(drawable));
	if (!pixmap)
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	object_add(xsrv, &pixmap->object);
	TAILQ_INSERT_TAIL(&client->objects, &pixmap->object, client_chain);
	return 1;
}

static int req_free_pixmap(struct xsrv *xsrv, struct client *client,
                           struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t pixmap_id = CLIENT_RU32(client);
	struct pixmap *pixmap = pixmap_get(xsrv, pixmap_id);
	if (!pixmap)
		return CLIENT_BAD_PIXMAP(xsrv, client, request, pixmap_id);
	object_destroy(xsrv, OBJECT(pixmap));
	object_free(xsrv, OBJECT(pixmap));
	return 1;
}

static int parse_gc_values(struct xsrv *xsrv, struct client *client,
                           struct request *request,
                           struct drawable *drawable,
                           struct gcontext_values *values,
                           uint32_t value_mask)
{
	values->tile = NULL;
	values->stipple = NULL;
	values->font = NULL;
	values->clip_mask = NULL;
	uint32_t tile;
	uint32_t stipple;
	uint32_t font;
	uint32_t clip_mask;
	VM_RU8(GCFunction, values->function, GXcopy);
	VM_RU32(GCPlaneMask, values->plane_mask, (uint32_t)-1);
	VM_RU32(GCForeground, values->foreground, 0);
	VM_RU32(GCBackground, values->background, 1);
	VM_RU16(GCLineWidth, values->line_width, 0);
	VM_RU8(GCLineStyle, values->line_style, LineSolid);
	VM_RU8(GCCapStyle, values->cap_style, CapButt);
	VM_RU8(GCJoinStyle, values->join_style, JoinMiter);
	VM_RU8(GCFillStyle, values->fill_style, FillSolid);
	VM_RU8(GCFillRule, values->fill_rule, EvenOddRule);
	VM_RU32(GCTile, tile, 0);
	VM_RU32(GCStipple, stipple, 0);
	VM_RI16(GCTileStipXOrigin, values->stipple_x_origin, 0);
	VM_RI16(GCTileStipYOrigin, values->stipple_y_origin, 0);
	VM_RU32(GCFont, font, 0);
	VM_RU8(GCSubwindowMode, values->subwindow_mode, ClipByChildren);
	VM_RU8(GCGraphicsExposures, values->graphics_exposures, 1);
	VM_RU16(GCClipXOrigin, values->clip_x_origin, 0);
	VM_RU16(GCClipYOrigin, values->clip_y_origin, 0);
	VM_RU32(GCClipMask, clip_mask, 0);
	VM_RU16(GCDashOffset, values->dash_offset, 0);
	VM_RU8(GCDashList, values->dashes, 4);
	VM_RU8(GCArcMode, values->arc_mode, ArcPieSlice);
	if (values->function > GXset)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        values->function);
	if (values->line_style > LineDoubleDash)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        values->line_style);
	if (values->cap_style > CapProjecting)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        values->cap_style);
	if (values->join_style > JoinBevel)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        values->join_style);
	if (values->fill_style > FillOpaqueStippled)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        values->fill_style);
	if (values->fill_rule > WindingRule)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        values->fill_rule);
	if (values->subwindow_mode > IncludeInferiors)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        values->subwindow_mode);
	if (values->arc_mode > ArcPieSlice)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        values->arc_mode);
	if (tile)
	{
		values->tile = pixmap_get(xsrv, tile);
		if (!values->tile)
			return CLIENT_BAD_PIXMAP(xsrv, client, request, tile);
		if (values->tile->drawable.root != drawable->root
		 || values->tile->drawable.format->depth != drawable->format->depth)
			return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	if (stipple)
	{
		values->stipple = pixmap_get(xsrv, stipple);
		if (!values->stipple)
			return CLIENT_BAD_PIXMAP(xsrv, client, request, tile);
		if (values->stipple->drawable.format->depth != 1
		 || values->stipple->drawable.root != drawable->root)
			return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	if (font)
	{
		values->font = font_get(xsrv, font);
		if (!values->font)
			return CLIENT_BAD_FONT(xsrv, client, request, font);
	}
	if (clip_mask)
	{
		values->clip_mask = pixmap_get(xsrv, clip_mask);
		if (!values->clip_mask)
			return CLIENT_BAD_PIXMAP(xsrv, client, request,
			                         clip_mask);
		if (values->clip_mask->drawable.format->depth != 1
		 || values->clip_mask->drawable.root != drawable->root)
			return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	return 0;
}

static int req_create_gc(struct xsrv *xsrv, struct client *client,
                         struct request *request)
{
	if (request->length < 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t cid = CLIENT_RU32(client);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t value_mask = CLIENT_RU32(client);
	uint32_t values_count = 0;
	for (uint32_t i = 0; i < 23; ++i)
	{
		if (value_mask & (1 << i))
			values_count++;
	}
	if (request->length != 4 + values_count)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (!client_has_free_id(xsrv, client, cid))
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, cid);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext_values values;
	int ret = parse_gc_values(xsrv, client, request, drawable, &values,
	                          value_mask);
	if (ret)
		goto err;
	struct gcontext *gcontext = gcontext_new(xsrv, client, cid, drawable,
	                                         &values);
	if (!gcontext)
	{
		ret = CLIENT_BAD_ALLOC(xsrv, client, request);
		goto err;
	}
	object_add(xsrv, &gcontext->object);
	TAILQ_INSERT_TAIL(&client->objects, &gcontext->object, client_chain);
	return 1;

err:
	object_free(xsrv, OBJECT(values.tile));
	object_free(xsrv, OBJECT(values.stipple));
	object_free(xsrv, OBJECT(values.font));
	object_free(xsrv, OBJECT(values.clip_mask));
	object_free(xsrv, OBJECT(drawable));
	return ret;
}

static int req_change_gc(struct xsrv *xsrv, struct client *client,
                         struct request *request)
{
	if (request->length < 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t gcontext_id = CLIENT_RU32(client);
	uint32_t value_mask = CLIENT_RU32(client);
	uint32_t values_count = 0;
	for (uint32_t i = 0; i < 23; ++i)
	{
		if (value_mask & (1 << i))
			values_count++;
	}
	if (request->length != 3 + values_count)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	struct gcontext_values values;
	int ret = parse_gc_values(xsrv, client, request, gcontext->drawable,
	                          &values, value_mask);
	if (ret)
		goto err;
	if (value_mask & GCFunction)
		gcontext->values.function = values.function;
	if (value_mask & GCPlaneMask)
		gcontext->values.plane_mask = values.plane_mask;
	if (value_mask & GCForeground)
		gcontext->values.foreground = values.foreground;
	if (value_mask & GCBackground)
		gcontext->values.background = values.background;
	if (value_mask & GCLineWidth)
		gcontext->values.line_width = values.line_width;
	if (value_mask & GCLineStyle)
		gcontext->values.line_style = values.line_style;
	if (value_mask & GCCapStyle)
		gcontext->values.cap_style = values.cap_style;
	if (value_mask & GCJoinStyle)
		gcontext->values.join_style = values.join_style;
	if (value_mask & GCFillStyle)
		gcontext->values.fill_style = values.fill_style;
	if (value_mask & GCFillRule)
		gcontext->values.fill_rule = values.fill_rule;
	if (value_mask & GCTile)
	{
		object_free(xsrv, OBJECT(gcontext->values.tile));
		gcontext->values.tile = values.tile;
	}
	if (value_mask & GCStipple)
	{
		object_free(xsrv, OBJECT(gcontext->values.stipple));
		gcontext->values.stipple = values.stipple;
	}
	if (value_mask & GCTileStipXOrigin)
		gcontext->values.stipple_x_origin = values.stipple_x_origin;
	if (value_mask & GCTileStipYOrigin)
		gcontext->values.stipple_y_origin = values.stipple_y_origin;
	if (value_mask & GCFont)
	{
		object_free(xsrv, OBJECT(gcontext->values.font));
		gcontext->values.font = values.font;
	}
	if (value_mask & GCSubwindowMode)
		gcontext->values.subwindow_mode = values.subwindow_mode;
	if (value_mask & GCGraphicsExposures)
		gcontext->values.graphics_exposures = values.graphics_exposures;
	if (value_mask & GCClipXOrigin)
		gcontext->values.clip_x_origin = values.clip_x_origin;
	if (value_mask & GCClipYOrigin)
		gcontext->values.clip_y_origin = values.clip_y_origin;
	if (value_mask & GCClipMask)
	{
		object_free(xsrv, OBJECT(gcontext->values.clip_mask));
		gcontext->values.clip_mask = values.clip_mask;
	}
	if (value_mask & GCDashOffset)
		gcontext->values.dash_offset = values.dash_offset;
	if (value_mask & GCDashList)
		gcontext->values.dashes = values.dashes;
	if (value_mask & GCArcMode)
		gcontext->values.arc_mode = values.arc_mode;
	object_free(xsrv, OBJECT(gcontext));
	return 1;

err:
	object_free(xsrv, OBJECT(values.tile));
	object_free(xsrv, OBJECT(values.stipple));
	object_free(xsrv, OBJECT(values.font));
	object_free(xsrv, OBJECT(values.clip_mask));
	object_free(xsrv, OBJECT(gcontext));
	return ret;
}

static int req_copy_gc(struct xsrv *xsrv, struct client *client,
                       struct request *request)
{
	if (request->length != 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t src_gc_id = CLIENT_RU32(client);
	uint32_t dst_gc_id = CLIENT_RU32(client);
	uint32_t value_mask = CLIENT_RU32(client);
	struct gcontext *src_gc = gcontext_get(xsrv, src_gc_id);
	if (!src_gc)
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, src_gc_id);
	struct gcontext *dst_gc = gcontext_get(xsrv, dst_gc_id);
	if (!dst_gc)
	{
		object_free(xsrv, OBJECT(src_gc));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, dst_gc_id);
	}
	if (value_mask & GCFunction)
		dst_gc->values.function = src_gc->values.function;
	if (value_mask & GCPlaneMask)
		dst_gc->values.plane_mask = src_gc->values.plane_mask;
	if (value_mask & GCForeground)
		dst_gc->values.foreground = src_gc->values.foreground;
	if (value_mask & GCBackground)
		dst_gc->values.background = src_gc->values.background;
	if (value_mask & GCLineWidth)
		dst_gc->values.line_width = src_gc->values.line_width;
	if (value_mask & GCLineStyle)
		dst_gc->values.line_style = src_gc->values.line_style;
	if (value_mask & GCCapStyle)
		dst_gc->values.cap_style = src_gc->values.cap_style;
	if (value_mask & GCJoinStyle)
		dst_gc->values.join_style = src_gc->values.join_style;
	if (value_mask & GCFillStyle)
		dst_gc->values.fill_style = src_gc->values.fill_style;
	if (value_mask & GCFillRule)
		dst_gc->values.fill_rule = src_gc->values.fill_rule;
	if (value_mask & GCTile)
	{
		object_free(xsrv, OBJECT(dst_gc->values.tile));
		dst_gc->values.tile = src_gc->values.tile;
		if (dst_gc->values.tile)
			dst_gc->values.tile->object.refs++;
	}
	if (value_mask & GCStipple)
	{
		object_free(xsrv, OBJECT(dst_gc->values.stipple));
		dst_gc->values.stipple = src_gc->values.stipple;
		if (dst_gc->values.stipple)
			dst_gc->values.stipple->object.refs++;
	}
	if (value_mask & GCTileStipXOrigin)
		dst_gc->values.stipple_x_origin = src_gc->values.stipple_x_origin;
	if (value_mask & GCTileStipYOrigin)
		dst_gc->values.stipple_y_origin = src_gc->values.stipple_y_origin;
	if (value_mask & GCFont)
	{
		object_free(xsrv, OBJECT(dst_gc->values.font));
		dst_gc->values.font = src_gc->values.font;
		if (dst_gc->values.font)
			dst_gc->values.font->object.refs++;
	}
	if (value_mask & GCSubwindowMode)
		dst_gc->values.subwindow_mode = src_gc->values.subwindow_mode;
	if (value_mask & GCGraphicsExposures)
		dst_gc->values.graphics_exposures = src_gc->values.graphics_exposures;
	if (value_mask & GCClipXOrigin)
		dst_gc->values.clip_x_origin = src_gc->values.clip_x_origin;
	if (value_mask & GCClipYOrigin)
		dst_gc->values.clip_y_origin = src_gc->values.clip_y_origin;
	if (value_mask & GCClipMask)
	{
		object_free(xsrv, OBJECT(dst_gc->values.clip_mask));
		dst_gc->values.clip_mask = src_gc->values.clip_mask;
		if (dst_gc->values.clip_mask)
			dst_gc->values.clip_mask->object.refs++;
	}
	if (value_mask & GCDashOffset)
		dst_gc->values.dash_offset = src_gc->values.dash_offset;
	if (value_mask & GCDashList)
		dst_gc->values.dashes = src_gc->values.dashes;
	if (value_mask & GCArcMode)
		dst_gc->values.arc_mode = src_gc->values.arc_mode;
	object_free(xsrv, OBJECT(src_gc));
	object_free(xsrv, OBJECT(dst_gc));
	return 1;
}

static int req_free_gc(struct xsrv *xsrv, struct client *client,
                       struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t gcontext_id = CLIENT_RU32(client);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	object_destroy(xsrv, OBJECT(gcontext));
	object_free(xsrv, OBJECT(gcontext));
	return 1;
}

static int req_clear_area(struct xsrv *xsrv, struct client *client,
                          struct request *request)
{
	if (request->length != 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t exposures = request->detail;
	(void)exposures; /* XXX */
	uint32_t window_id = CLIENT_RU32(client);
	int16_t x = CLIENT_RI16(client);
	int16_t y = CLIENT_RI16(client);
	uint16_t width = CLIENT_RU16(client);
	uint16_t height = CLIENT_RU16(client);
	struct window *window = window_get(xsrv, window_id);
	if (!window)
		return CLIENT_BAD_WINDOW(xsrv, client, request, window_id);
	if (window->class == InputOnly)
	{
		object_free(xsrv, OBJECT(window));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	if (!width)
		width = window->drawable.width - 1;
	if (!height)
		height = window->drawable.height - 1;
	window_clear(xsrv, window, x, y, width, height);
	object_free(xsrv, OBJECT(window));
	return 1;
}

static int req_copy_area(struct xsrv *xsrv, struct client *client,
                         struct request *request)
{
	if (request->length != 7)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t src_id = CLIENT_RU32(client);
	uint32_t dst_id = CLIENT_RU32(client);
	uint32_t gc_id = CLIENT_RU32(client);
	int16_t src_x = CLIENT_RI16(client);
	int16_t src_y = CLIENT_RI16(client);
	int16_t dst_x = CLIENT_RI16(client);
	int16_t dst_y = CLIENT_RI16(client);
	uint16_t width = CLIENT_RU16(client);
	uint16_t height = CLIENT_RU16(client);
	struct drawable *src = drawable_get(xsrv, src_id);
	if (!src)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, src_id);
	struct drawable *dst = drawable_get(xsrv, dst_id);
	if (!dst)
	{
		object_free(xsrv, OBJECT(src));
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, dst_id);
	}
	struct gcontext *gcontext = gcontext_get(xsrv, gc_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(src));
		object_free(xsrv, OBJECT(dst));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gc_id);
	}
	if (src->root != dst->root || src->format->depth != dst->format->depth)
	{
		object_free(xsrv, OBJECT(src));
		object_free(xsrv, OBJECT(dst));
		object_free(xsrv, OBJECT(gcontext));
		return CLIENT_BAD_MATCH(xsrv, client, request);
	}
	if (src_x < 0)
	{
		if (width <= -src_x)
			goto end;
		dst_x -= src_x;
		width += src_x;
		src_x = 0;
	}
	if (src_y < 0)
	{
		if (height <= -src_y)
			goto end;
		dst_y -= src_y;
		height += src_y;
		src_y = 0;
	}
	if (dst_x < 0)
	{
		if (width <= -dst_x)
			goto end;
		src_x -= dst_x;
		width += dst_x;
		dst_x = 0;
	}
	if (dst_y < 0)
	{
		if (height <= -dst_y)
			goto end;
		src_y -= dst_y;
		height += dst_y;
		dst_y = 0;
	}
	uint32_t src_l = src_x;
	uint32_t src_r = src_l + width;
	uint32_t src_t = src_y;
	uint32_t src_b = src_t + height;
	uint32_t dst_l = dst_x;
	uint32_t dst_r = dst_l + width;
	uint32_t dst_t = dst_y;
	uint32_t dst_b = dst_t + height;
	if (src_l >= src->width
	 || src_y >= src->height
	 || dst_l >= dst->width
	 || dst_t >= dst->height)
		goto end;
	if (src_r > src->width)
	{
		uint32_t diff = src_r - src->width;
		width -= diff;
		src_r -= diff;
		dst_r -= diff;
	}
	if (dst_r > dst->width)
	{
		uint32_t diff = dst_r - dst->width;
		width -= diff;
		src_r -= diff;
		dst_r -= diff;
	}
	if (src_b > src->height)
	{
		uint32_t diff = src_b - src->height;
		height -= diff;
		src_b -= diff;
		dst_b -= diff;
	}
	if (dst_b > dst->height)
	{
		uint32_t diff = dst_b - dst->height;
		height -= diff;
		src_b -= diff;
		dst_b -= diff;
	}
	uint8_t *src_data = (uint8_t*)src->data;
	src_data += src_y * src->pitch;
	src_data += src_x * src->format->bpp / 8;
	uint8_t *dst_data = (uint8_t*)dst->data;
	dst_data += dst_y * dst->pitch;
	dst_data += dst_x * src->format->bpp / 8;
	size_t bytes = width * src->format->bpp / 8;
	for (size_t i = 0; i < height; ++i)
	{
		memcpy(dst_data, src_data, bytes);
		src_data += src->pitch;
		dst_data += dst->pitch;
	}
	test_fb_redraw(xsrv, dst, dst_l, dst_t, dst_r, dst_b);

end:
	object_free(xsrv, OBJECT(src));
	object_free(xsrv, OBJECT(dst));
	object_free(xsrv, OBJECT(gcontext));
	return 1;
}

static int req_poly_point(struct xsrv *xsrv, struct client *client,
                          struct request *request)
{
	if (request->length < 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gcontext_id = CLIENT_RU32(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	}
	int32_t min_x = INT32_MAX;
	int32_t min_y = INT32_MAX;
	int32_t max_x = INT32_MIN;
	int32_t max_y = INT32_MIN;
	uint32_t npoints = request->length - 3;
	for (size_t i = 0; i < npoints; ++i)
	{
		int16_t x = CLIENT_RI16(client);
		int16_t y = CLIENT_RI16(client);
		TEST_MIN_MAX(x, y);
		poly_point(xsrv, drawable, gcontext, x, y);
	}
	test_fb_redraw(xsrv, drawable, min_x, min_y, max_x, max_y);
	object_free(xsrv, OBJECT(gcontext));
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_poly_line(struct xsrv *xsrv, struct client *client,
                         struct request *request)
{
	if (request->length < 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gcontext_id = CLIENT_RU32(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	}
	uint32_t npoints = request->length - 3;
	int16_t prv_x = CLIENT_RI16(client);
	int16_t prv_y = CLIENT_RI16(client);
	int32_t min_x = prv_x;
	int32_t min_y = prv_y;
	int32_t max_x = prv_x;
	int32_t max_y = prv_y;
	for (size_t i = 1; i < npoints; ++i)
	{
		int16_t x = CLIENT_RI16(client);
		int16_t y = CLIENT_RI16(client);
		TEST_MIN_MAX(x, y);
		poly_line(xsrv, drawable, gcontext, prv_x, prv_y, x, y);
		prv_x = x;
		prv_y = y;
	}
	test_fb_redraw(xsrv, drawable, min_x, min_y, max_x, max_y);
	object_free(xsrv, OBJECT(gcontext));
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_poly_segment(struct xsrv *xsrv, struct client *client,
                            struct request *request)
{
	if (request->length < 3 || !(request->length % 2))
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gcontext_id = CLIENT_RU32(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	}
	int32_t min_x = INT32_MAX;
	int32_t min_y = INT32_MAX;
	int32_t max_x = INT32_MIN;
	int32_t max_y = INT32_MIN;
	uint32_t nsegs = (request->length - 3) / 2;
	for (size_t i = 0; i < nsegs; ++i)
	{
		int16_t x1 = CLIENT_RI16(client);
		int16_t y1 = CLIENT_RI16(client);
		int16_t x2 = CLIENT_RI16(client);
		int16_t y2 = CLIENT_RI16(client);
		TEST_MIN_MAX(x1, y1);
		TEST_MIN_MAX(x2, y2);
		poly_line(xsrv, drawable, gcontext, x1, y1, x2, y2);
	}
	test_fb_redraw(xsrv, drawable, min_x, min_y, max_x, max_y);
	object_free(xsrv, OBJECT(gcontext));
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_poly_rectangle(struct xsrv *xsrv, struct client *client,
                              struct request *request)
{
	if (request->length < 3 || !(request->length % 2))
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gcontext_id = CLIENT_RU32(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	}
	int32_t min_x = INT32_MAX;
	int32_t min_y = INT32_MAX;
	int32_t max_x = INT32_MIN;
	int32_t max_y = INT32_MIN;
	uint32_t nrects = (request->length - 3) / 2;
	for (size_t i = 0; i < nrects; ++i)
	{
		int16_t x = CLIENT_RI16(client);
		int16_t y = CLIENT_RI16(client);
		uint16_t width = CLIENT_RU16(client);
		uint16_t height = CLIENT_RU16(client);
		int32_t right = x + width;
		int32_t bottom = y + height;
		TEST_MIN_MAX(x, y);
		TEST_MIN_MAX(right, bottom);
		poly_rect(xsrv, drawable, gcontext, x, y, width, height);
	}
	test_fb_redraw(xsrv, drawable, min_x, min_y, max_x, max_y);
	object_free(xsrv, OBJECT(gcontext));
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_poly_arc(struct xsrv *xsrv, struct client *client,
                        struct request *request)
{
	if (request->length < 3 || (request->length % 3))
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gcontext_id = CLIENT_RU32(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	}
	int32_t min_x = INT32_MAX;
	int32_t min_y = INT32_MAX;
	int32_t max_x = INT32_MIN;
	int32_t max_y = INT32_MIN;
	uint32_t narcs = (request->length - 3) / 3;
	for (size_t i = 0; i < narcs; ++i)
	{
		int16_t x = CLIENT_RI16(client);
		int16_t y = CLIENT_RI16(client);
		uint16_t width = CLIENT_RU16(client);
		uint16_t height = CLIENT_RU16(client);
		int16_t angle1 = CLIENT_RI16(client);
		int16_t angle2 = CLIENT_RI16(client);
		int32_t right = x + width;
		int32_t bottom = y + height;
		TEST_MIN_MAX(x, y);
		TEST_MIN_MAX(right, bottom);
		poly_arc(xsrv, drawable, gcontext, x, y, width, height, angle1,
		         angle2);
	}
	test_fb_redraw(xsrv, drawable, min_x - 1, min_y - 1, max_x + 1, max_y + 1);
	object_free(xsrv, OBJECT(gcontext));
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_fill_poly(struct xsrv *xsrv, struct client *client,
                         struct request *request)
{
	/* XXX */
	(void)xsrv;
	(void)client;
	(void)request;
	return 1;
}

static int req_poly_fill_rectangle(struct xsrv *xsrv, struct client *client,
                                   struct request *request)
{
	if (request->length < 3 || !(request->length % 2))
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gcontext_id = CLIENT_RU32(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	}
	int32_t min_x = INT32_MAX;
	int32_t min_y = INT32_MAX;
	int32_t max_x = INT32_MIN;
	int32_t max_y = INT32_MIN;
	uint32_t nrects = (request->length - 3) / 2;
	for (size_t i = 0; i < nrects; ++i)
	{
		int16_t x = CLIENT_RI16(client);
		int16_t y = CLIENT_RI16(client);
		uint16_t width = CLIENT_RU16(client);
		uint16_t height = CLIENT_RU16(client);
		int32_t right = x + width;
		int32_t bottom = y + height;
		TEST_MIN_MAX(x, y);
		TEST_MIN_MAX(right, bottom);
		poly_fill_rect(xsrv, drawable, gcontext, x, y, width, height);
	}
	test_fb_redraw(xsrv, drawable, min_x - 1, min_y - 1, max_x + 1, max_y + 1);
	object_free(xsrv, OBJECT(gcontext));
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_poly_fill_arc(struct xsrv *xsrv, struct client *client,
                             struct request *request)
{
	/* XXX */
	(void)xsrv;
	(void)client;
	(void)request;
	return 1;
}

static void put_1bpp(struct xsrv *xsrv, struct client *client,
                     struct drawable *drawable, int16_t x, int16_t y,
                     uint16_t width, uint16_t height)
{
	for (int32_t yy = y; yy < y + height; ++yy)
	{
		for (int32_t xx = x; xx < x + width; xx += 8)
		{
			uint8_t v = CLIENT_RU8(client);
			for (int32_t n = 0; n < 8; ++n)
			{
				int32_t xxx = xx + n;
				if (xxx >= 0 && xxx < drawable->width
				 && yy >= y && yy < drawable->height)
				{
					drawable_set_pixel(xsrv, drawable,
					                   xxx, yy, v >> n);
				}
			}
		}
	}
}

static void put_32bpp(struct xsrv *xsrv, struct client *client,
                      struct drawable *drawable, int16_t x, int16_t y,
                      uint16_t width, uint16_t height)
{
	for (int32_t yy = y; yy < y + height; ++yy)
	{
		for (int32_t xx = x; xx < x + width; ++xx)
		{
			uint32_t v = CLIENT_RU32(client);
			if (xx >= 0 && xx < drawable->width
			 && yy >= 0 && yy < drawable->height)
			{
				drawable_set_pixel(xsrv, drawable, xx, yy, v);
			}
		}
	}
}

static int req_put_image(struct xsrv *xsrv, struct client *client,
                         struct request *request)
{
	if (request->length < 6)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t data_format = request->detail;
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gc_id = CLIENT_RU32(client);
	uint16_t width = CLIENT_RU16(client);
	uint16_t height = CLIENT_RU16(client);
	int16_t dst_x = CLIENT_RI16(client);
	int16_t dst_y = CLIENT_RI16(client);
	uint8_t left_pad = CLIENT_RU8(client);
	uint8_t depth = CLIENT_RU8(client);
	CLIENT_READ(client, NULL, 2);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gc_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gc_id);
	}
	int ret;
	switch (data_format)
	{
		case XYBitmap:
		{
			if (depth != 1)
			{
				ret = CLIENT_BAD_MATCH(xsrv, client, request);
				break;
			}
			const struct format *format = xsrv_get_format(xsrv,
			                                              depth);
			if (!format)
			{
				ret = CLIENT_BAD_VALUE(xsrv, client, request,
				                       depth);
				break;
			}
			uint32_t line_width = (width * format->bpp + 31) / 32;
			if (request->length != height * line_width)
			{
				ret = CLIENT_BAD_LENGTH(xsrv, client, request);
				break;
			}
			/* XXX */
			ret = 1;
			break;
		}
		case XYPixmap:
		{
			if (left_pad != 0) /* XXX */
			{
				ret = CLIENT_BAD_IMPLEM(xsrv, client, request);
				break;
			}
			if (depth != 1) /* XXX */
			{
				ret = CLIENT_BAD_IMPLEM(xsrv, client, request);
				break;
			}
			if (depth != drawable->format->depth)
			{
				ret = CLIENT_BAD_MATCH(xsrv, client, request);
				break;
			}
			const struct format *format = xsrv_get_format(xsrv,
			                                              depth);
			if (!format)
			{
				ret = CLIENT_BAD_VALUE(xsrv, client, request,
				                       depth);
				break;
			}
			uint32_t line_bits = width * format->bpp;
			line_bits += format->scanline_pad - 1;
			line_bits -= line_bits % format->scanline_pad;
			if (request->length != 6 + (height * line_bits + 31) / 32)
			{
				ret = CLIENT_BAD_LENGTH(xsrv, client, request);
				break;
			}
			put_1bpp(xsrv, client, drawable, dst_x, dst_y,
			         width, height);
			ret = 1;
			break;
		}
		case ZPixmap:
		{
			if (left_pad != 0) /* XXX */
			{
				ret = CLIENT_BAD_IMPLEM(xsrv, client, request);
				break;
			}
			if (depth != drawable->format->depth)
			{
				ret = CLIENT_BAD_MATCH(xsrv, client, request);
				break;
			}
			const struct format *format = xsrv_get_format(xsrv,
			                                              depth);
			if (!format)
			{
				ret = CLIENT_BAD_VALUE(xsrv, client, request,
				                       depth);
				break;
			}
			uint32_t line_bits = width * format->bpp;
			line_bits += format->scanline_pad - 1;
			line_bits -= line_bits % format->scanline_pad;
			if (request->length != 6 + (height * line_bits + 31) / 32)
			{
				ret = CLIENT_BAD_LENGTH(xsrv, client, request);
				break;
			}
			put_32bpp(xsrv, client, drawable, dst_x, dst_y,
			          width, height);
			ret = 1;
			break;
		}
		default:
			ret = CLIENT_BAD_VALUE(xsrv, client, request,
			                       data_format);
			break;
	}
	test_fb_redraw(xsrv, drawable, dst_x, dst_y,
	               dst_x + width, dst_y + height);
	object_free(xsrv, OBJECT(drawable));
	object_free(xsrv, OBJECT(gcontext));
	return ret;
}

static int req_get_image(struct xsrv *xsrv, struct client *client,
                         struct request *request)
{
	if (request->length != 5)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t format = request->detail;
	uint32_t drawable_id = CLIENT_RU32(client);
	int16_t x = CLIENT_RI16(client);
	int16_t y = CLIENT_RI16(client);
	uint16_t width = CLIENT_RU16(client);
	uint16_t height = CLIENT_RU16(client);
	uint32_t plane_mask = CLIENT_RU32(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	if (format != ZPixmap) /* XXX */
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_IMPLEM(xsrv, client, request);
	}
	/* XXX */
	(void)x;
	(void)y;
	(void)width;
	(void)height;
	(void)plane_mask;
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_image_text8(struct xsrv *xsrv, struct client *client,
                           struct request *request)
{
	if (request->length < 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t length = request->detail;
	uint32_t drawable_id = CLIENT_RU32(client);
	uint32_t gcontext_id = CLIENT_RU32(client);
	int16_t x = CLIENT_RI16(client);
	int16_t y = CLIENT_RI16(client);
	if (request->length != 4 + (length + 3) / 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t text[255];
	CLIENT_READ(client, text, length);
	CLIENT_RPAD(client, length);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	struct gcontext *gcontext = gcontext_get(xsrv, gcontext_id);
	if (!gcontext)
	{
		object_free(xsrv, OBJECT(drawable));
		return CLIENT_BAD_GCONTEXT(xsrv, client, request, gcontext_id);
	}
	uint32_t minx;
	uint32_t miny;
	uint32_t maxx;
	uint32_t maxy;
	poly_text8(xsrv, drawable, gcontext, x, y, length, text,
	           &minx, &miny, &maxx, &maxy);
	test_fb_redraw(xsrv, drawable, minx, miny, maxx, maxy);
	object_free(xsrv, OBJECT(drawable));
	object_free(xsrv, OBJECT(gcontext));
	return 1;
}

static int req_create_cursor(struct xsrv *xsrv, struct client *client,
                             struct request *request)
{
	if (request->length != 8)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t cid = CLIENT_RU32(client);
	uint32_t source_id = CLIENT_RU32(client);
	uint32_t mask_id = CLIENT_RU32(client);
	uint16_t fore_red = CLIENT_RU16(client);
	uint16_t fore_green = CLIENT_RU16(client);
	uint16_t fore_blue = CLIENT_RU16(client);
	uint16_t back_red = CLIENT_RU16(client);
	uint16_t back_green = CLIENT_RU16(client);
	uint16_t back_blue = CLIENT_RU16(client);
	uint16_t xhot = CLIENT_RU16(client);
	uint16_t yhot = CLIENT_RU16(client);
	if (!client_has_free_id(xsrv, client, cid))
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, cid);
	struct pixmap *source = pixmap_get(xsrv, source_id);
	if (!source)
		return CLIENT_BAD_PIXMAP(xsrv, client, request, source_id);
	struct pixmap *mask;
	if (mask_id)
	{
		mask = pixmap_get(xsrv, mask_id);
		if (!mask)
		{
			object_free(xsrv, OBJECT(source));
			return CLIENT_BAD_PIXMAP(xsrv, client, request, mask_id);
		}
	}
	else
	{
		mask = NULL;
	}
	struct cursor *cursor = cursor_new(xsrv, client, cid, source, mask,
	                                   fore_red, fore_green, fore_blue,
	                                   back_red, back_green, back_blue,
	                                   xhot, yhot, 0, 0);
	if (!cursor)
	{
		object_free(xsrv, OBJECT(source));
		object_free(xsrv, OBJECT(mask));
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	}
	object_add(xsrv, &cursor->object);
	TAILQ_INSERT_TAIL(&client->objects, &cursor->object, client_chain);
	return 1;
}

static int req_create_glyph_cursor(struct xsrv *xsrv, struct client *client,
                                   struct request *request)
{
	if (request->length != 8)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t cid = CLIENT_RU32(client);
	uint32_t source_font_id = CLIENT_RU32(client);
	uint32_t mask_font_id = CLIENT_RU32(client);
	uint16_t source_char = CLIENT_RU16(client);
	uint16_t mask_char = CLIENT_RU16(client);
	uint16_t fore_red = CLIENT_RU16(client);
	uint16_t fore_green = CLIENT_RU16(client);
	uint16_t fore_blue = CLIENT_RU16(client);
	uint16_t back_red = CLIENT_RU16(client);
	uint16_t back_green = CLIENT_RU16(client);
	uint16_t back_blue = CLIENT_RU16(client);
	if (!client_has_free_id(xsrv, client, cid))
		return CLIENT_BAD_ID_CHOICE(xsrv, client, request, cid);
	struct pixmap *source;
	uint16_t xhot;
	uint16_t yhot;
	struct font *source_font = font_get(xsrv, source_font_id);
	if (!source_font)
		return CLIENT_BAD_FONT(xsrv, client, request, source_font_id);
	if (source_char >= 256)
	{
		object_free(xsrv, OBJECT(source_font));
		return CLIENT_BAD_VALUE(xsrv, client, request, source_char);
	}
	struct font_glyph *source_glyph = source_font->def->glyphs[source_char];
	if (!source_glyph)
	{
		source = pixmap_new(xsrv, client, 0, 32, 16,
		                    xsrv_get_format(xsrv, 1),
		                    xsrv->screens[0]->root); /* XXX */
		//font_free(xsrv, source_font);
		//return CLIENT_BAD_VALUE(xsrv, client, request, source_char);
		xhot = 0;
		yhot = 0;
	}
	else
	{
		source = source_glyph->pixmap;
		xhot = -source_glyph->left_bearing;
		yhot = source_glyph->ascent;
	}
	source->object.refs++;
	object_free(xsrv, OBJECT(source_font));
	int16_t maskx;
	int16_t masky;
	struct pixmap *mask;
	if (mask_font_id)
	{
		struct font *mask_font = font_get(xsrv, mask_font_id);
		if (!mask_font)
		{
			object_free(xsrv, OBJECT(source));
			return CLIENT_BAD_FONT(xsrv, client, request,
			                       mask_font_id);
		}
		if (mask_char >= 256)
		{
			object_free(xsrv, OBJECT(mask_font));
			object_free(xsrv, OBJECT(source));
			return CLIENT_BAD_VALUE(xsrv, client, request,
			                        mask_char);
		}
		struct font_glyph *mask_glyph = mask_font->def->glyphs[mask_char];
		if (!mask_glyph)
		{
			mask = NULL;
			maskx = 0;
			masky = 0;
			//font_free(xsrv, mask_font);
			//pixmap_free(xsrv, source);
			//return CLIENT_BAD_VALUE(xsrv, client, request,
			//                        mask_char);
		}
		else
		{
			mask = mask_glyph->pixmap;
			maskx = -mask_glyph->left_bearing - xhot;
			masky = mask_glyph->ascent - yhot;
			mask->object.refs++;
			object_free(xsrv, OBJECT(mask_font));
		}
	}
	else
	{
		mask = NULL;
		maskx = 0;
		masky = 0;
	}
	struct cursor *cursor = cursor_new(xsrv, client, cid, source, mask,
	                                   fore_red, fore_green, fore_blue,
	                                   back_red, back_green, back_blue,
	                                   xhot, yhot, maskx, masky);
	if (!cursor)
	{
		object_free(xsrv, OBJECT(mask));
		object_free(xsrv, OBJECT(source));
		return CLIENT_BAD_ALLOC(xsrv, client, request);
	}
	object_add(xsrv, &cursor->object);
	TAILQ_INSERT_TAIL(&client->objects, &cursor->object, client_chain);
	return 1;
}

static int req_free_cursor(struct xsrv *xsrv, struct client *client,
                           struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t cursor_id = CLIENT_RU32(client);
	struct cursor *cursor = cursor_get(xsrv, cursor_id);
	if (!cursor)
		return CLIENT_BAD_CURSOR(xsrv, client, request, cursor_id);
	object_destroy(xsrv, OBJECT(cursor));
	object_free(xsrv, OBJECT(cursor));
	return 1;
}

static int req_recolor_cursor(struct xsrv *xsrv, struct client *client,
                              struct request *request)
{
	if (request->length != 5)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t cursor_id = CLIENT_RU32(client);
	uint16_t fore_red = CLIENT_RU16(client);
	uint16_t fore_green = CLIENT_RU16(client);
	uint16_t fore_blue = CLIENT_RU16(client);
	uint16_t back_red = CLIENT_RU16(client);
	uint16_t back_green = CLIENT_RU16(client);
	uint16_t back_blue = CLIENT_RU16(client);
	struct cursor *cursor = cursor_get(xsrv, cursor_id);
	if (!cursor)
		return CLIENT_BAD_CURSOR(xsrv, client, request, cursor_id);
	cursor->fore_red = fore_red;
	cursor->fore_green = fore_green;
	cursor->fore_blue = fore_blue;
	cursor->back_red = back_red;
	cursor->back_green = back_green;
	cursor->back_blue = back_blue;
	object_free(xsrv, OBJECT(cursor));
	return 1;
}

static int req_query_best_size(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	if (request->length != 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t class = request->detail;
	uint32_t drawable_id = CLIENT_RU32(client);
	uint16_t width = CLIENT_RU16(client);
	uint16_t height = CLIENT_RU16(client);
	struct drawable *drawable = drawable_get(xsrv, drawable_id);
	if (!drawable)
		return CLIENT_BAD_DRAWABLE(xsrv, client, request, drawable_id);
	switch (class)
	{
		case CursorShape:
			if (width > 256)
				width = 256;
			if (height > 256)
				height = 256;
			/* XXX pad width / heigth to bpp */
			break;
		case TileShape:
			/* XXX pad width / heigth to bpp */
			break;
		case StippleShape:
			/* XXX pad width / heigth to bpp */
			break;
		default:
			object_free(xsrv, OBJECT(drawable));
			return CLIENT_BAD_VALUE(xsrv, client, request, class);
	}
	if (ringbuf_write_size(&client->wbuf) < 32)
	{
		object_free(xsrv, OBJECT(drawable));
		return -1;
	}
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU16(client, width);
	CLIENT_WU16(client, height);
	CLIENT_WRITE(client, NULL, 20);
	object_free(xsrv, OBJECT(drawable));
	return 1;
}

static int req_query_extension(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	char name[256];
	if (request->length < 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint16_t name_len = CLIENT_RU16(client);
	CLIENT_READ(client, NULL, 2);
	if (name_len > 256)
		return CLIENT_BAD_IMPLEM(xsrv, client, request);
	CLIENT_READ(client, name, name_len);
	struct extension *extension = NULL;
	for (size_t i = 0; i < xsrv->extensions_count; ++i)
	{
		struct extension *ext = xsrv->extensions[i];
		if (ext->name_len == name_len
		 && !memcmp(ext->name, name, name_len))
		{
			extension = ext;
			break;
		}
	}
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WRITE(client, NULL, 1);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU8(client, extension ? 1 : 0);
	CLIENT_WU8(client, extension ? extension->major_opcode : 0);
	CLIENT_WU8(client, extension ? extension->first_event : 0);
	CLIENT_WU8(client, extension ? extension->first_error : 0);
	CLIENT_WRITE(client, NULL, 20);
	return 1;
}

static int req_list_extensions(struct xsrv *xsrv, struct client *client,
                               struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t length = 0;
	for (size_t i = 0; i < xsrv->extensions_count; ++i)
		length += 1 + xsrv->extensions[i]->name_len;
	if (ringbuf_write_size(&client->wbuf) < 32 + length)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 2);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, (length + 3) / 4);
	CLIENT_WRITE(client, NULL, 24);
	for (size_t i = 0; i < xsrv->extensions_count; ++i)
	{
		struct extension *ext = xsrv->extensions[i];
		CLIENT_WU8(client, ext->name_len);
		CLIENT_WRITE(client, ext->name, ext->name_len);
	}
	CLIENT_WPAD(client, length);
	return 1;
}

static int req_change_keyboard_mapping(struct xsrv *xsrv, struct client *client,
                                       struct request *request)
{
	if (request->length < 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t keycode_count = request->detail;
	uint8_t first_keycode = CLIENT_RU8(client);
	uint8_t keysyms_per_keycode = CLIENT_RU8(client);
	CLIENT_READ(client, NULL, 2);
	if (request->length != 2 + keycode_count * keysyms_per_keycode)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (first_keycode < xsrv->keyboard.min_keycode)
		return CLIENT_BAD_VALUE(xsrv, client, request, first_keycode);
	if (first_keycode + keycode_count > xsrv->keyboard.max_keycode)
		return CLIENT_BAD_VALUE(xsrv, client, request, keycode_count);
	if (!keysyms_per_keycode || keysyms_per_keycode > 8)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        keysyms_per_keycode);
	for (size_t i = 0; i < keycode_count; ++i)
	{
		size_t j;
		for (j = 0; j < keysyms_per_keycode; ++j)
			xsrv->keyboard.syms[first_keycode + i][j] = CLIENT_RU32(client);
		for (; j < 8; ++j)
			xsrv->keyboard.syms[first_keycode + i][j] = NoSymbol;
	}
	/* XXX MappingNotify */
	return 1;
}

static int req_get_keyboard_mapping(struct xsrv *xsrv, struct client *client,
                                    struct request *request)
{
	if (request->length != 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t first_keycode = CLIENT_RU8(client);
	uint8_t count = CLIENT_RU8(client);
	CLIENT_READ(client, NULL, 2);
	if (first_keycode < xsrv->keyboard.min_keycode)
		return CLIENT_BAD_VALUE(xsrv, client, request, first_keycode);
	if (first_keycode + count > xsrv->keyboard.max_keycode + 1)
		return CLIENT_BAD_VALUE(xsrv, client, request, count);
	uint32_t length = count * xsrv->keyboard.syms_per_code;
	if (ringbuf_write_size(&client->wbuf) < 32 + length)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, xsrv->keyboard.syms_per_code);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, length);
	CLIENT_WRITE(client, NULL, 24);
	for (size_t i = 0; i < count; ++i)
	{
		for (size_t j = 0; j < xsrv->keyboard.syms_per_code; ++j)
			CLIENT_WU32(client, xsrv->keyboard.syms[first_keycode + i][j]);
	}
	return 1;
}

static int req_change_keyboard_control(struct xsrv *xsrv, struct client *client,
                                       struct request *request)
{
	if (request->length < 2)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t value_mask = CLIENT_RU32(client);
	(void)value_mask;
	/* XXX */
	return 1;
}

static int req_get_keyboard_control(struct xsrv *xsrv, struct client *client,
                                    struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t length = 5;
	if (ringbuf_write_size(&client->wbuf) < 32 + length)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, xsrv->keyboard.global_auto_repeat);
	CLIENT_WU16(client, request->sequence);
	CLIENT_WU32(client, length);
	CLIENT_WU32(client, xsrv->keyboard.led_mask);
	CLIENT_WU8(client, xsrv->keyboard.key_click_percent);
	CLIENT_WU8(client, xsrv->keyboard.bell_percent);
	CLIENT_WU16(client, xsrv->keyboard.bell_pitch);
	CLIENT_WU16(client, xsrv->keyboard.bell_duration);
	CLIENT_WRITE(client, NULL, 2);
	CLIENT_WRITE(client, xsrv->keyboard.auto_repeats, 32);
	return 1;
}

static int req_bell(struct xsrv *xsrv, struct client *client,
                    struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	/* XXX */
	return 1;
}

static int req_change_pointer_control(struct xsrv *xsrv, struct client *client,
                                      struct request *request)
{
	if (request->length != 3)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	int16_t accel_numerator = CLIENT_RI16(client);
	int16_t accel_denominator = CLIENT_RI16(client);
	int16_t threshold = CLIENT_RI16(client);
	uint8_t do_accel = CLIENT_RU8(client);
	uint8_t do_threshold = CLIENT_RU8(client);
	if (accel_numerator == -1)
		accel_numerator = 0;
	else if (accel_numerator < 0)
		return CLIENT_BAD_VALUE(xsrv, client, request, accel_numerator);
	if (accel_denominator == -1)
		accel_denominator = 1;
	else if (accel_denominator <= 0)
		return CLIENT_BAD_VALUE(xsrv, client, request, accel_denominator);
	if (threshold == -1)
		threshold = 0;
	else if (threshold < 0)
		return CLIENT_BAD_VALUE(xsrv, client, request, threshold);
	xsrv->pointer.accel_numerator = accel_numerator;
	xsrv->pointer.accel_denominator = accel_denominator;
	xsrv->pointer.threshold = threshold;
	xsrv->pointer.do_accel = do_accel;
	xsrv->pointer.do_threshold = do_threshold;
	return 1;
}

static int req_get_pointer_control(struct xsrv *xsrv, struct client *client,
                                   struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, 0);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WU16(client, xsrv->pointer.accel_numerator);
	CLIENT_WU16(client, xsrv->pointer.accel_denominator);
	CLIENT_WU16(client, xsrv->pointer.threshold);
	CLIENT_WRITE(client, NULL, 18);
	return 1;
}

static int req_set_pointer_mapping(struct xsrv *xsrv, struct client *client,
                                   struct request *request)
{
	if (request->length < 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint8_t map_len = request->detail;
	if (request->length != 1 + (map_len + 3) / 4)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (map_len != sizeof(xsrv->pointer.map) / sizeof(*xsrv->pointer.map))
		return CLIENT_BAD_VALUE(xsrv, client, request, map_len);
	uint8_t map[256];
	uint8_t dup[256] = {0};
	CLIENT_READ(client, map, map_len);
	CLIENT_RPAD(client, map_len);
	uint8_t status = MappingSuccess;
	for (size_t i = 0; i < map_len; ++i)
	{
		if (dup[map[i]])
			return CLIENT_BAD_VALUE(xsrv, client, request, map[i]);
		if (map[i] != xsrv->pointer.map[i]
		 && xsrv->pointer.state[i])
		{
			status = MappingBusy;
			break;
		}
		dup[map[i]] = 1;
	}
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, status);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WRITE(client, NULL, 24);
	return 1;
}

static int req_get_pointer_mapping(struct xsrv *xsrv, struct client *client,
                                   struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t map_len = sizeof(xsrv->pointer.map) / sizeof(*xsrv->pointer.map);
	uint32_t length = (map_len + 3) / 4;
	if (ringbuf_write_size(&client->wbuf) < 32 + length)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, map_len);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, length);
	CLIENT_WRITE(client, NULL, 24);
	CLIENT_WRITE(client, xsrv->pointer.map, map_len);
	CLIENT_WPAD(client, map_len);
	return 1;
}

static int req_set_modifier_mapping(struct xsrv *xsrv, struct client *client,
                                    struct request *request)
{
	uint8_t keycodes_per_modifier = request->detail;
	if (request->length != 1 + 2 * keycodes_per_modifier)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	if (!keycodes_per_modifier || keycodes_per_modifier > 8)
		return CLIENT_BAD_VALUE(xsrv, client, request,
		                        keycodes_per_modifier);
	uint8_t status = MappingSuccess;
	uint8_t values[8][8];
	for (size_t i = 0; i < 8; ++i)
	{
		for (size_t j = 0; j < keycodes_per_modifier; ++j)
		{
			uint8_t v = CLIENT_RU8(client);
			values[i][j] = v;
			if (!v)
				continue;
			if (v < xsrv->keyboard.min_keycode
			 || v > xsrv->keyboard.max_keycode)
				return CLIENT_BAD_VALUE(xsrv, client, request,
				                        v);
			if (xsrv->keyboard.state[v / 8] & (1 << (v % 8)))
				status = MappingBusy;
		}
	}
	if (status == Success)
	{
		xsrv->keyboard.codes_per_modifier = keycodes_per_modifier;
		memcpy(xsrv->keyboard.modifiers, values, sizeof(values));
		/* XXX MappingNotify */
	}
	if (ringbuf_write_size(&client->wbuf) < 32)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, status);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, 0);
	CLIENT_WRITE(client, NULL, 24);
	return 1;
}

static int req_get_modifier_mapping(struct xsrv *xsrv, struct client *client,
                                    struct request *request)
{
	if (request->length != 1)
		return CLIENT_BAD_LENGTH(xsrv, client, request);
	uint32_t length = 2 * xsrv->keyboard.codes_per_modifier;
	if (ringbuf_write_size(&client->wbuf) < 32 + length * 4)
		return -1;
	CLIENT_WU8(client, 1);
	CLIENT_WU8(client, xsrv->keyboard.codes_per_modifier);
	CLIENT_WU16(client, client->sequence);
	CLIENT_WU32(client, length);
	CLIENT_WRITE(client, NULL, 24);
	for (size_t i = 0; i < 8; ++i)
	{
		for (size_t j = 0; j < xsrv->keyboard.codes_per_modifier; ++j)
			CLIENT_WU8(client, xsrv->keyboard.modifiers[i][j]);
	}
	return 1;
}

static int req_no_operation(struct xsrv *xsrv, struct client *client,
                            struct request *request)
{
	(void)xsrv;
	(void)client;
	(void)request;
	return 1;
}

req_handler_t g_req_handlers[256] =
{
	[CREATE_WINDOW]            = req_create_window,
	[CHANGE_WINDOW_ATTRIBUTES] = req_change_window_attributes,
	[GET_WINDOW_ATTRIBUTES]    = req_get_window_attributes,
	[DESTROY_WINDOW]           = req_destroy_window,
	[DESTROY_SUBWINDOWS]       = req_destroy_subwindows,
	[REPARENT_WINDOW]          = req_reparent_window,
	[MAP_WINDOW]               = req_map_window,
	[MAP_SUBWINDOWS]           = req_map_subwindows,
	[UNMAP_WINDOW]             = req_unmap_window,
	[UNMAP_SUBWINDOWS]         = req_unmap_subwindows,
	[CONFIGURE_WINDOW]         = req_configure_window,
	[GET_GEOMETRY]             = req_get_geometry,
	[QUERY_TREE]               = req_query_tree,
	[INTERN_ATOM]              = req_intern_atom,
	[GET_ATOM_NAME]            = req_get_atom_name,
	[CHANGE_PROPERTY]          = req_change_property,
	[DELETE_PROPERTY]          = req_delete_property,
	[GET_PROPERTY]             = req_get_property,
	[LIST_PROPERTIES]          = req_list_properties,
	[GRAB_POINTER]             = req_grab_pointer,
	[UNGRAB_POINTER]           = req_ungrab_pointer,
	[GRAB_BUTTON]              = req_grab_button,
	[UNGRAB_BUTTON]            = req_ungrab_button,
	[GRAB_SERVER]              = req_grab_server,
	[UNGRAB_SERVER]            = req_ungrab_server,
	[QUERY_POINTER]            = req_query_pointer,
	[WARP_POINTER]             = req_warp_pointer,
	[SET_INPUT_FOCUS]          = req_set_input_focus,
	[GET_INPUT_FOCUS]          = req_get_input_focus,
	[OPEN_FONT]                = req_open_font,
	[CLOSE_FONT]               = req_close_font,
	[LIST_FONTS]               = req_list_fonts,
	[CREATE_PIXMAP]            = req_create_pixmap,
	[FREE_PIXMAP]              = req_free_pixmap,
	[CREATE_GC]                = req_create_gc,
	[CHANGE_GC]                = req_change_gc,
	[COPY_GC]                  = req_copy_gc,
	[FREE_GC]                  = req_free_gc,
	[CLEAR_AREA]               = req_clear_area,
	[COPY_AREA]                = req_copy_area,
	[POLY_POINT]               = req_poly_point,
	[POLY_LINE]                = req_poly_line,
	[POLY_SEGMENT]             = req_poly_segment,
	[POLY_RECTANGLE]           = req_poly_rectangle,
	[POLY_ARC]                 = req_poly_arc,
	[FILL_POLY]                = req_fill_poly,
	[POLY_FILL_RECTANGLE]      = req_poly_fill_rectangle,
	[POLY_FILL_ARC]            = req_poly_fill_arc,
	[PUT_IMAGE]                = req_put_image,
	[GET_IMAGE]                = req_get_image,
	[IMAGE_TEXT8]              = req_image_text8,
	[CREATE_CURSOR]            = req_create_cursor,
	[CREATE_GLYPH_CURSOR]      = req_create_glyph_cursor,
	[FREE_CURSOR]              = req_free_cursor,
	[RECOLOR_CURSOR]           = req_recolor_cursor,
	[QUERY_BEST_SIZE]          = req_query_best_size,
	[QUERY_EXTENSION]          = req_query_extension,
	[LIST_EXTENSIONS]          = req_list_extensions,
	[CHANGE_KEYBOARD_MAPPING]  = req_change_keyboard_mapping,
	[GET_KEYBOARD_MAPPING]     = req_get_keyboard_mapping,
	[CHANGE_KEYBOARD_CONTROL]  = req_change_keyboard_control,
	[GET_KEYBOARD_CONTROL]     = req_get_keyboard_control,
	[BELL]                     = req_bell,
	[CHANGE_POINTER_CONTROL]   = req_change_pointer_control,
	[GET_POINTER_CONTROL]      = req_get_pointer_control,
	[SET_POINTER_MAPPING]      = req_set_pointer_mapping,
	[GET_POINTER_MAPPING]      = req_get_pointer_mapping,
	[SET_MODIFIER_MAPPING]     = req_set_modifier_mapping,
	[GET_MODIFIER_MAPPING]     = req_get_modifier_mapping,
	[NO_OPERATION]             = req_no_operation,
};
