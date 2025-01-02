#ifndef XSRV_H
#define XSRV_H

#include "ringbuf.h"

#include <X11/X.h>

#include <sys/queue.h>

#define CLIENT_SOCK_RECV (1 << 0)
#define CLIENT_SOCK_SEND (1 << 1)

#define VENDOR "xsrv"

#define VERSION_MAJOR 11
#define VERSION_MINOR 0

#define RELEASE_NUMBER 1000000

struct font_def;
struct format;
struct atom;

struct object;
struct xsrv;

struct object_def
{
	const char *name;
	void (*free)(struct xsrv *xsrv, struct object *object);
	void (*destroy)(struct xsrv *xsrv, struct object *object);
};

struct object
{
	uint8_t type;
	TAILQ_ENTRY(object) map_chain;
	TAILQ_ENTRY(object) client_chain;
	struct client *client;
	uint32_t id;
	uint32_t refs;
};

TAILQ_HEAD(object_head, object);
TAILQ_HEAD(window_head, window);

struct drawable
{
	struct object object;
	const struct format *format;
	struct window *root;
	uint16_t width;
	uint16_t height;
	uint16_t alloc_width;
	uint16_t alloc_height;
	uint32_t pitch;
	uint8_t *data;
};

struct window_attributes
{
	struct pixmap *background_pixmap;
	uint32_t background_pixel;
	struct pixmap *border_pixmap;
	uint32_t border_pixel;
	uint8_t bit_gravity;
	uint8_t win_gravity;
	uint8_t backing_store;
	uint32_t backing_planes;
	uint32_t backing_pixel;
	uint8_t override_redirect;
	uint8_t save_under;
	uint32_t event_mask;
	uint32_t do_not_propagate_mask;
	struct colormap *colormap;
	struct cursor *cursor;
};

struct window_event
{
	struct window *window;
	struct client *client;
	uint32_t mask;
	TAILQ_ENTRY(window_event) window_chain;
	TAILQ_ENTRY(window_event) client_chain;
};

TAILQ_HEAD(window_event_head, window_event);

struct pointer_grab
{
	struct client *client;
	struct window *window;
	struct window *confine_to;
	struct cursor *cursor;
	uint16_t event_mask;
	uint8_t pointer_mode;
	uint8_t keyboard_mode;
	uint8_t owner_events;
	uint32_t time;
};

struct button_grab
{
	struct pointer_grab pointer_grab;
	uint16_t modifiers;
	uint8_t button;
	TAILQ_ENTRY(button_grab) window_chain;
	TAILQ_ENTRY(button_grab) client_chain;
};

TAILQ_HEAD(button_grab_head, button_grab);

struct property
{
	struct atom *property;
	struct atom *type;
	uint8_t format;
	uint32_t data_bytes;
	uint32_t data_len;
	union
	{
		uint32_t *data32;
		uint16_t *data16;
		uint8_t *data8;
		void *data;
	};
	TAILQ_ENTRY(property) chain;
};

#define WINDOW_DIRTY  (1 << 0)
#define WINDOW_MAPPED (1 << 1)

struct window
{
	union
	{
		struct object object;
		struct drawable drawable;
	};
	struct window_attributes attributes;
	struct window_event_head events;
	struct screen *screen;
	struct window *parent;
	struct visual *visual;
	uint16_t tree_depth;
	uint16_t border_width;
	uint32_t flags;
	uint16_t class;
	int16_t x;
	int16_t y;
	struct window_head children;
	struct button_grab_head button_grabs;
	TAILQ_HEAD(, property) properties; /* XXX hash table */
	TAILQ_ENTRY(window) chain;
};

struct colormap
{
	struct object object;
};

struct pixmap
{
	union
	{
		struct object object;
		struct drawable drawable;
	};
};

struct cursor
{
	struct object object;
	struct pixmap *color;
	struct pixmap *mask;
	uint16_t fore_red;
	uint16_t fore_green;
	uint16_t fore_blue;
	uint16_t back_red;
	uint16_t back_green;
	uint16_t back_blue;
	uint16_t xhot;
	uint16_t yhot;
	int16_t maskx; /* offset of mask compared to source */
	int16_t masky;
};

struct font
{
	struct object object;
	struct font_def *def;
};

struct gcontext_values
{
	struct pixmap *tile;
	struct pixmap *stipple;
	struct font *font;
	struct pixmap *clip_mask;
	uint8_t depth;
	uint8_t function;
	uint8_t fill_rule;
	uint8_t cap_style;
	uint32_t plane_mask;
	uint32_t foreground;
	uint32_t background;
	uint16_t line_width;
	int16_t stipple_x_origin;
	int16_t stipple_y_origin;
	uint16_t dash_offset;
	uint8_t dashes;
	uint8_t line_style;
	uint8_t join_style;
	uint8_t fill_style;
	uint8_t subwindow_mode;
	uint8_t graphics_exposures;
	int16_t clip_x_origin;
	int16_t clip_y_origin;
	uint8_t arc_mode;
};

struct gcontext
{
	struct object object;
	struct window *root;
	struct drawable *drawable;
	struct gcontext_values values;
};

struct visual
{
	uint32_t id;
	uint8_t class;
	uint8_t bpp;
	uint16_t colormap_entries;
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
};

struct format
{
	uint8_t depth;
	uint8_t bpp;
	uint8_t scanline_pad;
};

struct depth
{
	uint8_t depth;
	uint16_t visuals_count;
	struct visual **visuals;
};

struct screen
{
	struct window *root;
	struct colormap *colormap;
	uint32_t white_pixel;
	uint32_t black_pixel;
	uint32_t input_mask;
	uint16_t width;
	uint16_t height;
	uint16_t mm_width;
	uint16_t mm_height;
	uint16_t min_maps;
	uint16_t max_maps;
	struct visual *visual;
	uint8_t backing_stores;
	uint8_t save_unders;
	uint8_t root_depth;
	uint8_t depths_count;
	struct depth **depths;
};

struct backend
{
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bpp;
	void *data;
	int fd;
};

enum client_state
{
	CLIENT_SETUP,      /* waiting for initial 12 bytes */
	CLIENT_CONNECTING, /* waiting for auth proto name / auth proto data */
	CLIENT_CONNECTED,  /* normal session */
	CLIENT_CLOSING,    /* waiting for write buf to be empty */
	CLIENT_CLOSED,     /* waiting for deletion */
};

struct request
{
	uint8_t opcode;
	uint8_t detail;
	uint16_t length;
	uint32_t sequence;
};

struct client
{
	enum client_state state;
	struct
	{
		uint8_t byte_order;
		uint16_t major_version;
		uint16_t minor_version;
		uint16_t auth_proto_name_len;
		uint16_t auth_proto_data_len;
	} init_data;
	int has_req;
	struct request request;
	int fd;
	int sock_state;
	uint32_t sequence;
	uint32_t id_base;
	uint32_t id_mask;
	struct ringbuf rbuf;
	struct ringbuf wbuf;
	struct window_event_head events;
	struct button_grab_head button_grabs;
	struct object_head objects;
	TAILQ_ENTRY(client) chain;
	TAILQ_ENTRY(client) gc_chain;
};

struct extension
{
	char *name;
	uint8_t name_len;
	uint8_t major_opcode;
	uint8_t first_event;
	uint8_t first_error;
};

struct font_glyph
{
	struct pixmap *pixmap;
	int16_t left_bearing;
	int16_t right_bearing;
	int16_t width;
	int16_t ascent;
	int16_t descent;
	uint16_t attributes;
};

struct font_def
{
	char *name;
	uint8_t name_len;
	struct font_glyph *glyphs[256]; /* XXX come on.. */
};

struct focus
{
	struct window *window;
	uint8_t revert_to;
	uint32_t timestamp;
};

struct pointer
{
	uint8_t map[32];
	uint8_t state[32];
	uint8_t do_accel;
	uint8_t do_threshold;
	uint16_t accel_numerator;
	uint16_t accel_denominator;
	uint16_t threshold;
	uint16_t x;
	uint16_t y;
	uint32_t wx;
	uint32_t wy;
	struct window *window;
	struct cursor *cursor;
};

struct keyboard
{
	uint8_t state[32];
	uint8_t syms_per_code;
	uint8_t codes_per_modifier;
	uint32_t syms[256][8];
	uint8_t modifiers[8][8];
	uint8_t min_keycode;
	uint8_t max_keycode;
	uint8_t global_auto_repeat;
	uint32_t led_mask;
	uint8_t key_click_percent;
	uint8_t bell_percent;
	uint16_t bell_pitch;
	uint16_t bell_duration;
	uint8_t auto_repeats[32];
};

struct rect
{
	uint32_t left;
	uint32_t right;
	uint32_t top;
	uint32_t bottom;
};

struct atom
{
	uint32_t id;
	char *name;
	uint32_t name_len;
	TAILQ_ENTRY(atom) chain;
};

TAILQ_HEAD(atom_head, atom);

struct stats
{
	uint64_t client_count;
};

struct object_stat
{
	uint64_t count;
	uint64_t destroyed_count;
};

struct xsrv
{
	size_t id_base_bitmap[0x200 / (sizeof(size_t) * 8)];
	const char *progname;
	int sock;
	uint32_t xid;
	struct rect redraw_rect;
	struct backend backend;
	struct focus focus;
	struct pointer pointer;
	struct keyboard keyboard;
	struct client *grab_client;
	size_t grab_client_nb;
	uint16_t keybutmask;
	uint8_t formats_count;
	uint8_t screens_count;
	uint8_t extensions_count;
	uint16_t fonts_count;
	int need_process; /* some clients need to be processed */
	struct format **formats;
	struct screen **screens;
	struct extension **extensions;
	struct font_def **fonts;
	const struct object_def *objects_defs[256];
	struct object_stat objects_stats[256];
	uint32_t objects_map_count;
	uint32_t objects_map_size;
	struct object_head *objects_map;
	struct pointer_grab pointer_grab;
	struct atom_head atoms; /* XXX hash table */
	struct stats stats;
	uint8_t obj_window;
	uint8_t obj_colormap;
	uint8_t obj_pixmap;
	uint8_t obj_cursor;
	uint8_t obj_font;
	uint8_t obj_gcontext;
	uint8_t obj_shmseg;
	uint8_t obj_picture;
	uint8_t obj_glyphset;
	TAILQ_HEAD(, client) clients;
};

int backend_init(struct xsrv *xsrv);
int backend_poll(struct xsrv *xsrv);
int backend_display(struct xsrv *xsrv, struct rect *rect);

uint64_t nanotime(void);
uint64_t millitime(void);
uint32_t npot32(uint32_t val);

struct extension *register_extension(struct xsrv *xsrv, const char *name);

uint8_t register_object_type(struct xsrv *xsrv, const struct object_def *def);
struct object *object_get_typed(struct xsrv *xsrv, uint32_t id, uint8_t type);

void xsrv_set_focus(struct xsrv *xsrv, struct window *window,
                    uint8_t revert_to, uint32_t timestamp);
void xsrv_revert_focus(struct xsrv *xsrv);

void xsrv_grab_pointer(struct xsrv *xsrv, struct client *client,
                       struct window *window, struct window *confine_to,
                       struct cursor *cursor, uint16_t event_mask,
                       uint8_t pointer_mode, uint8_t keyboard_mode,
                       uint8_t owner_events, uint32_t time);
void xsrv_ungrab_pointer(struct xsrv *xsrv);

void delete_button_grab(struct xsrv *xsrv, struct button_grab *button_grab);

void xsrv_key_press(struct xsrv *xsrv, uint8_t key);
void xsrv_key_release(struct xsrv *xsrv, uint8_t key);
void xsrv_cursor_motion(struct xsrv *xsrv, uint16_t x, uint16_t y);
void xsrv_button_press(struct xsrv *xsrv, uint8_t button);
void xsrv_button_release(struct xsrv *xsrv, uint8_t button);

struct client *client_new(struct xsrv *xsrv, int fd);
void client_delete(struct xsrv *xsrv, struct client *client);
int client_run(struct xsrv *xsrv, struct client *client);
int client_has_free_id(struct xsrv *xsrv, struct client *client, uint32_t id);
int client_recv(struct xsrv *xsrv, struct client *client);
int client_send(struct xsrv *xsrv, struct client *client);

void object_init(struct xsrv *xsrv, struct client *client,
                 struct object *object, uint8_t type, uint32_t id);
void object_add(struct xsrv *xsrv, struct object *object);
void object_remove(struct xsrv *xsrv, struct object *object);
struct object *object_get(struct xsrv *xsrv, uint32_t id);
struct window *window_get(struct xsrv *xsrv, uint32_t id);
struct colormap *colormap_get(struct xsrv *xsrv, uint32_t id);
struct pixmap *pixmap_get(struct xsrv *xsrv, uint32_t id);
struct cursor *cursor_get(struct xsrv *xsrv, uint32_t id);
struct font *font_get(struct xsrv *xsrv, uint32_t id);
struct gcontext *gcontext_get(struct xsrv *xsrv, uint32_t id);
struct drawable *drawable_get(struct xsrv *xsrv, uint32_t id);
void object_free(struct xsrv *xsrv, struct object *object);
void object_destroy(struct xsrv *xsrv, struct object *object);
int drawable_init(struct xsrv *xsrv, struct drawable *drawable,
                  uint16_t width, uint16_t height,
                  const struct format *format, struct window *root);

uint32_t xsrv_allocate_id(struct xsrv *xsrv);
uint32_t xsrv_allocate_id_base(struct xsrv *xsrv);
void xsrv_release_id_base(struct xsrv *xsrv, uint32_t base);

struct window *get_window_at(struct xsrv *xsrv, uint32_t *x, uint32_t *y);

const struct format *xsrv_get_format(struct xsrv *xsrv, uint8_t depth);
uint32_t drawable_get_pixel(struct xsrv *xsrv, struct drawable *drawable,
                            uint16_t x, uint16_t y);
void drawable_set_pixel(struct xsrv *xsrv, struct drawable *drawable,
                        uint16_t x, uint16_t y, uint32_t value);

void window_register(struct xsrv *xsrv);
struct window *window_new(struct xsrv *xsrv, struct client *client, uint32_t id,
                          struct window *parent, int16_t x, int16_t y,
                          uint16_t width, uint16_t height,
                          const struct format *format, uint16_t border_width,
                          struct visual *visual, uint16_t class,
                          struct window_attributes *attributes);
int window_visible(struct window *window);
void window_map(struct xsrv *xsrv, struct client *client, struct window *window);
void window_unmap(struct xsrv *xsrv, struct window *window);
struct cursor *window_get_cursor(struct xsrv *xsrv, struct window *window);
int window_resize(struct xsrv *xsrv, struct window *window,
                  uint16_t width, uint16_t height);
void window_get_fb_rect(struct xsrv *xsrv, struct window *window,
                        uint32_t *x, uint32_t *y,
                        uint32_t *width, uint32_t *height);
void window_get_full_rect(struct xsrv *xsrv, struct window *window,
                          uint32_t *x, uint32_t *y,
                          uint32_t *width, uint32_t *height);
void window_reparent(struct xsrv *xsrv, struct client *client,
                     struct window *window, struct window *parent,
                     int16_t x, int16_t y);
struct property *window_get_property(struct window *window, struct atom *prop);
void window_delete_property(struct xsrv *xsrv, struct window *window,
                            struct atom *prop);
struct button_grab *window_get_button_grab(struct xsrv *xsrv,
                                           struct window *window,
                                           uint8_t button,
                                           uint16_t modifiers);
void window_remove_button_grab(struct xsrv *xsrv, struct window *window,
                               uint8_t button, uint16_t modifiers);
void window_redraw(struct xsrv *xsrv, struct window *window);
void window_set_cursor(struct xsrv *xsrv, struct window *window,
                       struct cursor *cursor);
void window_clear(struct xsrv *xsrv, struct window *window,
                  int16_t x, int16_t y, uint16_t width, uint16_t height);
void window_button_press(struct xsrv *xsrv, struct window *window,
                         uint8_t button, uint32_t wx, uint32_t wy);
void window_button_release(struct xsrv *xsrv, struct window *window,
                           uint8_t button, uint32_t wx, uint32_t wy);
void window_motion_notify(struct xsrv *xsrv, struct window *window,
                          uint8_t detail, uint32_t wx, uint32_t wy);
void window_key_press(struct xsrv *xsrv, struct window *window, uint8_t key);
void window_key_release(struct xsrv *xsrv, struct window *window, uint8_t key);
void window_enter_notify(struct xsrv *xsrv, struct window *window,
                         uint8_t detail);
void window_leave_notify(struct xsrv *xsrv, struct window *window,
                         uint8_t detail);
int window_map_request(struct xsrv *xsrv, struct client *client,
                       struct window *window);
void window_map_notify(struct xsrv *xsrv, struct window *window);
void window_unmap_notify(struct xsrv *xsrv, struct window *window);
void window_focus_in(struct xsrv *xsrv, struct window *window, uint8_t detail);
void window_focus_out(struct xsrv *xsrv, struct window *window, uint8_t detail);
void window_keymap_notify(struct xsrv *xsrv, struct window *window);
void window_reparent_notify(struct xsrv *xsrv, struct window *window,
                            struct window *oldparent);
int window_resize_request(struct xsrv *xsrv, struct client *client,
                          struct window *window,
                          uint16_t width, uint16_t height);
void window_expose(struct xsrv *xsrv, struct window *window);
void window_create_notify(struct xsrv *xsrv, struct window *window);
void window_destroy_notify(struct xsrv *xsrv, struct window *window);
int window_configure_request(struct xsrv *xsrv, struct client *client,
                             struct window *window,  uint32_t value_mask,
                             int16_t x, int16_t y,
                             uint16_t width, uint16_t height,
                             uint16_t border_width, struct window *sibling,
                             uint8_t stack_mode);
void window_configure_notify(struct xsrv *xsrv, struct window *window);
void window_property_notify(struct xsrv *xsrv, struct window *window,
                            struct atom *property, uint8_t state);

void colormap_register(struct xsrv *xsrv);
struct colormap *colormap_new(struct xsrv *xsrv, struct client *client,
                              uint32_t id);

void font_register(struct xsrv *xsrv);
struct font *font_new(struct xsrv *xsrv, struct client *client, uint32_t id,
                      struct font_def *def);

void gcontext_register(struct xsrv *xsrv);
struct gcontext *gcontext_new(struct xsrv *xsrv, struct client *client,
                              uint32_t id, struct drawable *drawable,
                              struct gcontext_values *values);

void pixmap_register(struct xsrv *xsrv);
struct pixmap *pixmap_new(struct xsrv *xsrv, struct client *client,
                          uint32_t id, uint16_t width, uint16_t height,
                          const struct format *format, struct window *root);

void cursor_register(struct xsrv *xsrv);
struct cursor *cursor_new(struct xsrv *xsrv, struct client *client, uint32_t id,
                          struct pixmap *color, struct pixmap *mask,
                          uint16_t fore_red, uint16_t fore_green,
                          uint16_t fore_blue, uint16_t back_red,
                          uint16_t back_green, uint16_t back_blue,
                          uint16_t xhot, uint16_t yhot,
                          int16_t maskx, int16_t masky);

void framebuffer_redraw(struct xsrv *xsrv, int16_t x, int16_t y,
                        uint16_t width, uint16_t height);
void framebuffer_update(struct xsrv *xsrv, struct rect *rect);

void poly_point(struct xsrv *xsrv, struct drawable *drawable,
                struct gcontext *gcontext, int16_t x, int16_t y);
void poly_line(struct xsrv *xsrv, struct drawable *drawable,
               struct gcontext *gcontext, int16_t x1, int16_t y1,
               int16_t x2, int16_t y2);
void poly_rect(struct xsrv *xsrv, struct drawable *drawable,
               struct gcontext *gcontext, int16_t x, int16_t y,
               uint16_t width, uint16_t height);
void poly_arc(struct xsrv *xsrv, struct drawable *drawable,
              struct gcontext *gcontext, int16_t x, int16_t y,
              uint16_t width, uint16_t height, int16_t angle1, int16_t angle2);
void poly_fill_rect(struct xsrv *xsrv, struct drawable *drawable,
                    struct gcontext *gcontext, int16_t x, int16_t y,
                    uint16_t width, uint16_t height);
void poly_text8(struct xsrv *xsrv, struct drawable *drawable,
                struct gcontext *gcontext, int16_t x, int16_t y,
                uint8_t text_len, const uint8_t *text,
                uint32_t *minx, uint32_t *miny,
                uint32_t *maxx, uint32_t *maxy);

struct atom *atom_get(struct xsrv *xsrv, const char *name);
struct atom *atom_get_id(struct xsrv *xsrv, uint32_t id);
struct atom *atom_new(struct xsrv *xsrv, const char *name);

void test_fb_redraw(struct xsrv *xsrv, struct drawable *drawable,
                    int32_t min_x, int32_t min_y,
                    int32_t max_x, int32_t max_y);

#define CLIENT_RI8(c) ringbuf_ri8(&(c)->rbuf)
#define CLIENT_RU8(c) ringbuf_ru8(&(c)->rbuf)
#define CLIENT_RI16(c) ringbuf_ri16(&(c)->rbuf)
#define CLIENT_RU16(c) ringbuf_ru16(&(c)->rbuf)
#define CLIENT_RI32(c) ringbuf_ri32(&(c)->rbuf)
#define CLIENT_RU32(c) ringbuf_ru32(&(c)->rbuf)
#define CLIENT_READ(c, d, n) ringbuf_read(&(c)->rbuf, d, n)
#define CLIENT_RPAD(c, v) ringbuf_rpad(&(c)->rbuf, v)
#define CLIENT_WI8(c, v) ringbuf_wi8(&(c)->wbuf, v)
#define CLIENT_WU8(c, v) ringbuf_wu8(&(c)->wbuf, v)
#define CLIENT_WI16(c, v) ringbuf_wi16(&(c)->wbuf, v)
#define CLIENT_WU16(c, v) ringbuf_wu16(&(c)->wbuf, v)
#define CLIENT_WU32(c, v) ringbuf_wi32(&(c)->wbuf, v)
#define CLIENT_WI32(c, v) ringbuf_wu32(&(c)->wbuf, v)
#define CLIENT_WRITE(c, d, n) ringbuf_write(&(c)->wbuf, d, n)
#define CLIENT_WPAD(c, n) ringbuf_wpad(&(c)->wbuf, n)

#define OBJECT(obj) (&(obj)->object)

int client_error(struct xsrv *xsrv, struct client *client,
                 struct request *request, uint8_t error_code,
                 uint32_t xid);

#define CLIENT_BAD_REQUEST(x, c, r)      client_error(x, c, r, BadRequest, 0)
#define CLIENT_BAD_VALUE(x, c, r, i)     client_error(x, c, r, BadValue, i)
#define CLIENT_BAD_WINDOW(x, c, r, i)    client_error(x, c, r, BadWindow, i)
#define CLIENT_BAD_PIXMAP(x, c, r, i)    client_error(x, c, r, BadPixmap, i)
#define CLIENT_BAD_ATOM(x, c, r, i)      client_error(x, c, r, BadAtom, i)
#define CLIENT_BAD_CURSOR(x, c, r, i)    client_error(x, c, r, BadCursor, i)
#define CLIENT_BAD_FONT(x, c, r, i)      client_error(x, c, r, BadFont, i)
#define CLIENT_BAD_MATCH(x, c, r)        client_error(x, c, r, BadMatch, 0)
#define CLIENT_BAD_DRAWABLE(x, c, r, i)  client_error(x, c, r, BadDrawable, i)
#define CLIENT_BAD_ACCESS(x, c, r)       client_error(x, c, r, BadAccess, 0)
#define CLIENT_BAD_ALLOC(x, c, r)        client_error(x, c, r, BadAlloc, 0)
#define CLIENT_BAD_COLORMAP(x, c, r, i)  client_error(x, c, r, BadColor, i)
#define CLIENT_BAD_GCONTEXT(x, c, r, i)  client_error(x, c, r, BadGC, i)
#define CLIENT_BAD_ID_CHOICE(x, c, r, i) client_error(x, c, r, BadIDChoice, i)
#define CLIENT_BAD_NAME(x, c, r)         client_error(x, c, r, BadName, 0)
#define CLIENT_BAD_LENGTH(x, c, r)       client_error(x, c, r, BadLength, 0)
#define CLIENT_BAD_IMPLEM(x, c, r)       client_error(x, c, r, BadImplementation, 0)

int ev_key_press(struct xsrv *xsrv, struct client *client, uint8_t detail,
                 uint32_t time, struct window *root, struct window *event,
                 struct window *child, int16_t root_x, int16_t root_y,
                 int16_t event_x, int16_t event_y, uint16_t state,
                 uint8_t same_screen);
int ev_key_release(struct xsrv *xsrv, struct client *client, uint8_t detail,
                   uint32_t time, struct window *root, struct window *event,
                   struct window *child, int16_t root_x, int16_t root_y,
                   int16_t event_x, int16_t event_y, uint16_t state,
                   uint8_t same_screen);
int ev_button_press(struct xsrv *xsrv, struct client *client, uint8_t detail,
                    uint32_t time, struct window *root, struct window *event,
                    struct window *child, int16_t root_x, int16_t root_y,
                    int16_t event_x, int16_t event_y, uint16_t state,
                    uint8_t same_screen);
int ev_button_release(struct xsrv *xsrv, struct client *client, uint8_t detail,
                      uint32_t time, struct window *root, struct window *event,
                      struct window *child, int16_t root_x, int16_t root_y,
                      int16_t event_x, int16_t event_y, uint16_t state,
                      uint8_t same_screen);
int ev_motion_notify(struct xsrv *xsrv, struct client *client, uint8_t detail,
                     uint32_t time, struct window *root, struct window *event,
                     struct window *child, int16_t root_x, int16_t root_y,
                     int16_t event_x, int16_t event_y, uint16_t state,
                     uint8_t same_screen);
int ev_enter_notify(struct xsrv *xsrv, struct client *client, uint8_t detail,
                    uint32_t time, struct window *root, struct window *event,
                    struct window *child, int16_t root_x, int16_t root_y,
                    int16_t event_x, int16_t event_y, uint16_t state,
                    uint8_t same_screen);
int ev_leave_notify(struct xsrv *xsrv, struct client *client, uint8_t detail,
                    uint32_t time, struct window *root, struct window *event,
                    struct window *child, int16_t root_x, int16_t root_y,
                    int16_t event_x, int16_t event_y, uint16_t state,
                    uint8_t same_screen);
int ev_focus_in(struct xsrv *xsrv, struct client *client,
                uint8_t detail, struct window *window, uint8_t mode);
int ev_focus_out(struct xsrv *xsrv, struct client *client,
                 uint8_t detail, struct window *window, uint8_t mode);
int ev_keymap_notify(struct xsrv *xsrv, struct client *client, uint8_t *keys);
int ev_expose(struct xsrv *xsrv, struct client *client,
              struct window *window, int16_t x, int16_t y,
              uint16_t width, uint16_t height, uint16_t count);
int ev_graphics_exposure(struct xsrv *xsrv, struct client *client,
                         struct drawable *drawable, uint16_t x, uint16_t y,
                         uint16_t width, uint16_t height,
                         uint16_t minor_opcode, uint16_t count,
                         uint8_t major_opcode);
int ev_no_exposure(struct xsrv *xsrv, struct client *client,
                   struct drawable *drawable, uint16_t minor_opcode,
                   uint8_t major_opcode);
int ev_visibility_notify(struct xsrv *xsrv, struct client *client,
                         struct window *window, uint8_t state);
int ev_create_notify(struct xsrv *xsrv, struct client *client,
                     struct window *parent, struct window *window,
                     int16_t x, int16_t y, uint16_t width, uint16_t height,
                     uint16_t border_width, uint8_t override_redirect);
int ev_destroy_notify(struct xsrv *xsrv, struct client *client,
                      struct window *event, struct window *window);
int ev_unmap_notify(struct xsrv *xsrv, struct client *client,
                    struct window *event, struct window *window,
                    uint8_t from_configure);
int ev_map_notify(struct xsrv *xsrv, struct client *client,
                  struct window *event, struct window *window,
                  uint8_t override_redirect);
int ev_map_request(struct xsrv *xsrv, struct client *client,
                   struct window *parent, struct window *window);
int ev_reparent_notify(struct xsrv *xsrv, struct client *client,
                       struct window *event, struct window *window,
                       struct window *parent, int16_t x, int16_t y,
                       uint8_t override_redirect);
int ev_configure_notify(struct xsrv *xsrv, struct client *client,
                        struct window *event, struct window *window,
                        struct window *above_sibling, int16_t x, int16_t y,
                        uint16_t width, uint16_t height, uint16_t border_width,
                        uint8_t override_redirect);
int ev_configure_request(struct xsrv *xsrv, struct client *client,
                         uint8_t stack_mode, struct window *parent,
                         struct window *window, struct window *sibling,
                         int16_t x, int16_t y, uint16_t width, uint16_t height,
                         uint16_t border_width, uint16_t value_mask);
int ev_gravity_notify(struct xsrv *xsrv, struct client *client,
                      struct window *event, struct window *window,
                      int16_t x, int16_t y);
int ev_resize_request(struct xsrv *xsrv, struct client *client,
                      struct window *window, uint16_t width, uint16_t height);
int ev_circulate_notify(struct xsrv *xsrv, struct client *client,
                        struct window *event, struct window *window,
                        uint8_t place);
int ev_circulate_request(struct xsrv *xsrv, struct client *client,
                         struct window *parent, struct window *window,
                         uint8_t place);
int ev_property_notify(struct xsrv *xsrv, struct client *client,
                       struct window *window, struct atom *atom, uint32_t time,
                       uint8_t state);
int ev_selection_clear(struct xsrv *xsrv, struct client *client, uint32_t time,
                       struct window *owner, struct atom *selection);
int ev_selection_request(struct xsrv *xsrv, struct client *client,
                         uint32_t time, struct window *owner,
                         struct window *requestor, struct atom *selection,
                         struct atom *target, struct atom *property);
int ev_selection_notify(struct xsrv *xsrv, struct client *client,
                        uint32_t time, struct window *requestor,
                        struct atom *selection, struct atom *target,
                        struct atom *property);
int ev_colormap_notify(struct xsrv *xsrv, struct client *client,
                       struct window *window, struct colormap *colormap,
                       uint8_t new, uint8_t state);
int ev_client_message(struct xsrv *xsrv, struct client *client,
                      uint8_t format, struct window *window, struct atom *type,
                      uint8_t data_len, uint8_t *data);
int ev_mapping_notify(struct xsrv *xsrv, struct client *client,
                      uint8_t request, uint8_t first_keycode, uint8_t count);

typedef int (*req_handler_t)(struct xsrv *xsrv, struct client *client,
                             struct request *request);

extern req_handler_t g_req_handlers[256];

#endif
