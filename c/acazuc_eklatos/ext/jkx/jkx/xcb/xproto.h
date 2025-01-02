#ifndef XCB_XPROTO_H
#define XCB_XPROTO_H

#include <stdint.h>

typedef uint32_t xcb_window_t;
typedef uint32_t xcb_pixmap_t;
typedef uint32_t xcb_cursor_t;
typedef uint32_t xcb_font_t;
typedef uint32_t xcb_gcontext_t;
typedef uint32_t xcb_colormap_t;
typedef uint32_t xcb_drawable_t;
typedef uint32_t xcb_fontable_t;
typedef uint32_t xcb_atom_t;
typedef uint32_t xcb_visualid_t;
typedef uint32_t xcb_keysym_t;
typedef uint8_t xcb_keycode_t;
typedef uint8_t xcb_button_t;
typedef uint32_t xcb_timestamp_t;
typedef uint8_t xcb_bool_t;

typedef enum
{
	XCB_CREATE_WINDOW              = 1,
	XCB_CHANGE_WINDOW_ATTRIBUTES   = 2,
	XCB_GET_WINDOW_ATTRIBUTES      = 3,
	XCB_DESTROY_WINDOW             = 4,
	XCB_DESTROY_SUBWINDOWS         = 5,
	XCB_CHANGE_SAVE_SET            = 6,
	XCB_REPARENT_WINDOW            = 7,
	XCB_MAP_WINDOW                 = 8,
	XCB_MAP_SUBWINDOWS             = 9,
	XCB_UNMAP_WINDOW               = 10,
	XCB_UNMAP_SUBWINDOWS           = 11,
	XCB_CONFIGURE_WINDOW           = 12,
	XCB_CIRCULATE_WINDOW           = 13,
	XCB_GET_GEOMETRY               = 14,
	XCB_QUERY_TREE                 = 15,
	XCB_INTERN_ATOM                = 16,
	XCB_GET_ATOM_NAME              = 17,
	XCB_CHANGE_PROPERTY            = 18,
	XCB_DELETE_PROPERTY            = 19,
	XCB_GET_PROPERTY               = 20,
	XCB_LIST_PROPERTIES            = 21,
	XCB_SET_SELECTION_OWNER        = 22,
	XCB_GET_SELECTION_OWNER        = 23,
	XCB_CONVERT_SELECTION          = 24,
	XCB_SEND_EVENT                 = 25,
	XCB_GRAB_POINTER               = 26,
	XCB_UNGRAB_POINTER             = 27,
	XCB_GRAB_BUTTON                = 28,
	XCB_UNGRAB_BUTTON              = 29,
	XCB_CHANGE_ACTIVE_POINTER_GRAB = 30,
	XCB_GRAB_KEYBOARD              = 31,
	XCB_UNGRAB_KEYBOARD            = 32,
	XCB_GRAB_KEY                   = 33,
	XCB_UNGRAB_KEY                 = 34,
	XCB_ALLOW_EVENTS               = 35,
	XCB_GRAB_SERVER                = 36,
	XCB_UNGRAB_SERVER              = 37,
	XCB_QUERY_POINTER              = 38,
	XCB_GET_MOTION_EVENTS          = 39,
	XCB_TRANSLATE_COORDINATES      = 40,
	XCB_WARP_POINTER               = 41,
	XCB_SET_INPUT_FOCUS            = 42,
	XCB_GET_INPUT_FOCUS            = 43,
	XCB_QUERY_KEYMAP               = 44,
	XCB_OPEN_FONT                  = 45,
	XCB_CLOSE_FONT                 = 46,
	XCB_QUERY_FONT                 = 47,
	XCB_QUERY_TEXT_EXTENTS         = 48,
	XCB_LIST_FONTS                 = 49,
	XCB_LIST_FONTS_WITH_INFO       = 50,
	XCB_SET_FONT_PATH              = 51,
	XCB_GET_FONT_PATH              = 52,
	XCB_CREATE_PIXMAP              = 53,
	XCB_FREE_PIXMAP                = 54,
	XCB_CREATE_GC                  = 55,
	XCB_CHANGE_GC                  = 56,
	XCB_COPY_GC                    = 57,
	XCB_SET_DASHES                 = 58,
	XCB_SET_CLIP_RECTANGLES        = 59,
	XCB_FREE_GC                    = 60,
	XCB_CLEAR_AREA                 = 61,
	XCB_COPY_AREA                  = 62,
	XCB_COPY_PLANE                 = 63,
	XCB_POLY_POINT                 = 64,
	XCB_POLY_LINE                  = 65,
	XCB_POLY_SEGMENT               = 66,
	XCB_POLY_RECTANGLE             = 67,
	XCB_POLY_ARC                   = 68,
	XCB_FILL_POLY                  = 69,
	XCB_POLY_FILL_RECTANGLE        = 70,
	XCB_POLY_FILL_ARC              = 71,
	XCB_PUT_IMAGE                  = 72,
	XCB_GET_IMAGE                  = 73,
	XCB_POLY_TEXT_8                = 74,
	XCB_POLY_TEXT_16               = 75,
	XCB_IMAGE_TEXT_8               = 76,
	XCB_IMAGE_TEXT_16              = 77,
	XCB_CREATE_COLORMAP            = 78,
	XCB_FREE_COLORMAP              = 79,
	XCB_COPY_COLORMAP_AND_FREE     = 80,
	XCB_INSTALL_COLORMAP           = 81,
	XCB_UNINSTALL_COLORMAP         = 82,
	XCB_LIST_INSTALLED_COLORMAPS   = 83,
	XCB_ALLOC_COLOR                = 84,
	XCB_ALLOC_NAMED_COLOR          = 85,
	XCB_ALLOC_COLOR_CELLS          = 86,
	XCB_ALLOC_COLOR_PLANES         = 87,
	XCB_FREE_COLORS                = 88,
	XCB_STORE_COLORS               = 89,
	XCB_STORE_NAMED_COLOR          = 90,
	XCB_QUERY_COLORS               = 91,
	XCB_LOOKUP_COLOR               = 92,
	XCB_CREATE_CURSOR              = 93,
	XCB_CREATE_GLYPH_CURSOR        = 94,
	XCB_FREE_CURSOR                = 95,
	XCB_RECOLOR_CURSOR             = 96,
	XCB_QUERY_BEST_SIZE            = 97,
	XCB_QUERY_EXTENSION            = 98,
	XCB_LIST_EXTENSIONS            = 99,
	XCB_CHANGE_KEYBOARD_MAPPING    = 100,
	XCB_GET_KEYBOARD_MAPPING       = 101,
	XCB_CHANGE_KEYBOARD_CONTROL    = 102,
	XCB_GET_KEYBOARD_CONTROL       = 103,
	XCB_BELL                       = 104,
	XCB_CHANGE_POINTER_CONTROL     = 105,
	XCB_GET_POINTER_CONTROL        = 106,
	XCB_SET_SCREEN_SAVER           = 107,
	XCB_GET_SCREEN_SAVER           = 108,
	XCB_CHANGE_HOSTS               = 109,
	XCB_LIST_HOSTS                 = 110,
	XCB_SET_ACCESS_CONTROL         = 111,
	XCB_SET_CLOSE_DOWN_MODE        = 112,
	XCB_KILL_CLIENT                = 113,
	XCB_ROTATE_PROPERTIES          = 114,
	XCB_FORCE_SCREEN_SAVER         = 115,
	XCB_SET_POINTER_MAPPING        = 116,
	XCB_GET_POINTER_MAPPING        = 117,
	XCB_SET_MODIFIER_MAPPING       = 118,
	XCB_GET_MODIFIER_MAPPING       = 119,
	XCB_NO_OPERATION               = 127,
} xcb_request_t;

typedef enum
{
	XCB_EVENT_MASK_NO_EVENT              = 0,
	XCB_EVENT_MASK_KEY_PRESS             = (1 << 0),
	XCB_EVENT_MASK_KEY_RELEASE           = (1 << 1),
	XCB_EVENT_MASK_BUTTON_PRESS          = (1 << 2),
	XCB_EVENT_MASK_BUTTON_RELEASE        = (1 << 3),
	XCB_EVENT_MASK_ENTER_WINDOW          = (1 << 4),
	XCB_EVENT_MASK_LEAVE_WINDOW          = (1 << 5),
	XCB_EVENT_MASK_POINTER_MOTION        = (1 << 6),
	XCB_EVENT_MASK_POINTER_MOTION_HINT   = (1 << 7),
	XCB_EVENT_MASK_BUTTON_1_MOTION       = (1 << 8),
	XCB_EVENT_MASK_BUTTON_2_MOTION       = (1 << 9),
	XCB_EVENT_MASK_BUTTON_3_MOTION       = (1 << 10),
	XCB_EVENT_MASK_BUTTON_4_MOTION       = (1 << 11),
	XCB_EVENT_MASK_BUTTON_5_MOTION       = (1 << 12),
	XCB_EVENT_MASK_BUTTON_MOTION         = (1 << 13),
	XCB_EVENT_MASK_KEYMAP_STATE          = (1 << 14),
	XCB_EVENT_MASK_EXPOSURE              = (1 << 15),
	XCB_EVENT_MASK_VISIBILITY_CHANGE     = (1 << 16),
	XCB_EVENT_MASK_STRUCTURE_NOTIFY      = (1 << 17),
	XCB_EVENT_MASK_RESIZE_REDIRECT       = (1 << 18),
	XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY   = (1 << 19),
	XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT = (1 << 20),
	XCB_EVENT_MASK_FOCUS_CHANGE          = (1 << 21),
	XCB_EVENT_MASK_PROPERTY_CHANGE       = (1 << 22),
	XCB_EVENT_MASK_COLORMAP_CHANGE       = (1 << 23),
	XCB_EVENT_MASK_OWNER_GRAB_BUTTON     = (1 << 24),
} xcb_event_mask_t;

typedef enum
{
	XCB_KEY_BUT_MASK_SHIFT    = (1 << 0),
	XCB_KEY_BUT_MASK_LOCK     = (1 << 1),
	XCB_KEY_BUT_MASK_CONTROL  = (1 << 2),
	XCB_KEY_BUT_MASK_MOD_1    = (1 << 3),
	XCB_KEY_BUT_MASK_MOD_2    = (1 << 4),
	XCB_KEY_BUT_MASK_MOD_3    = (1 << 5),
	XCB_KEY_BUT_MASK_MOD_4    = (1 << 6),
	XCB_KEY_BUT_MASK_MOD_5    = (1 << 7),
	XCB_KEY_BUT_MASK_BUTTON_1 = (1 << 8),
	XCB_KEY_BUT_MASK_BUTTON_2 = (1 << 9),
	XCB_KEY_BUT_MASK_BUTTON_3 = (1 << 10),
	XCB_KEY_BUT_MASK_BUTTON_4 = (1 << 11),
	XCB_KEY_BUT_MASK_BUTTON_5 = (1 << 12),
} xcb_key_but_mask_t;

typedef enum
{
	XCB_REQUEST        = 1,
	XCB_VALUE          = 2,
	XCB_WINDOW         = 3,
	XCB_PIXMAP         = 4,
	XCB_ATOM           = 5,
	XCB_CURSOR         = 6,
	XCB_FONT           = 7,
	XCB_MATCH          = 8,
	XCB_DRAWABLE       = 9,
	XCB_ACCESS         = 10,
	XCB_ALLOC          = 11,
	XCB_COLORMAP       = 12,
	XCB_G_CONTEXT      = 13,
	XCB_ID_CHOICE      = 14,
	XCB_NAME           = 15,
	XCB_LENGTH         = 16,
	XCB_IMPLEMENTATION = 17,
} xcb_error_code_t;

typedef enum
{
	XCB_KEY_PRESS         = 2,
	XCB_KEY_RELEASE       = 3,
	XCB_BUTTON_PRESS      = 4,
	XCB_BUTTON_RELEASE    = 5,
	XCB_MOTION_NOTIFY     = 6,
	XCB_ENTER_NOTIFY      = 7,
	XCB_LEAVE_NOTIFY      = 8,
	XCB_FOCUS_IN          = 9,
	XCB_FOCUS_OUT         = 10,
	XCB_KEYMAP_NOTIFY     = 11,
	XCB_EXPOSE            = 12,
	XCB_GRAPHICS_EXPOSURE = 13,
	XCB_NO_EXPOSURE       = 14,
	XCB_VISIBILITY_NOTIFY = 15,
	XCB_CREATE_NOTIFY     = 16,
	XCB_DESTROY_NOTIFY    = 17,
	XCB_UNMAP_NOTIFY      = 18,
	XCB_MAP_NOTIFY        = 19,
	XCB_MAP_REQUEST       = 20,
	XCB_REPARENT_NOTIFY   = 21,
	XCB_CONFIGURE_NOTIFY  = 22,
	XCB_CONFIGURE_REQUEST = 23,
	XCB_GRAVITY_NOTIFY    = 24,
	XCB_RESIZE_REQUEST    = 25,
	XCB_CIRCULATE_NOTIFY  = 26,
	XCB_CIRCULATE_REQUEST = 27,
	XCB_PROPERTY_NOTIFY   = 28,
	XCB_SELECTION_CLEAR   = 29,
	XCB_SELECTION_REQUEST = 30,
	XCB_SELECTION_NOTIFY  = 31,
	XCB_COLORMAP_NOTIFY   = 32,
	XCB_CLIENT_MESSAGE    = 33,
	XCB_MAPPING_NOTIFY    = 34,
} xcb_event_type_t;

typedef enum
{
	XCB_GRAVITY_BIT_FORGET = 0,
	XCB_GRAVITY_WIN_UNMAP  = 0,
	XCB_GRAVITY_NORTHWEST  = 1,
	XCB_GRAVITY_NORTH      = 2,
	XCB_GRAVITY_NORTHEAST  = 3,
	XCB_GRAVITY_WEST       = 4,
	XCB_GRAVITY_CENTER     = 5,
	XCB_GRAVITY_EAST       = 6,
	XCB_GRAVITY_SOUTHWEST  = 7,
	XCB_GRAVITY_SOUTH      = 8,
	XCB_GRAVITY_SOUTHEAST  = 9,
	XCB_GRAVITY_STATIC     = 10,
} xcb_gravity_t;

typedef enum
{
	XCB_CW_BACK_PIXMAP       = (1 << 0),
	XCB_CW_BACK_PIXEL        = (1 << 1),
	XCB_CW_BORDER_PIXMAP     = (1 << 2),
	XCB_CW_BORDER_PIXEL      = (1 << 3),
	XCB_CW_BIT_GRAVITY       = (1 << 4),
	XCB_CW_WIN_GRAVITY       = (1 << 5),
	XCB_CW_BACKING_STORE     = (1 << 6),
	XCB_CW_BACKING_PLANES    = (1 << 7),
	XCB_CW_BACKING_PIXEL     = (1 << 8),
	XCB_CW_OVERRIDE_REDIRECT = (1 << 9),
	XCB_CW_SAVE_UNDER        = (1 << 10),
	XCB_CW_EVENT_MASK        = (1 << 11),
	XCB_CW_DONT_PROPAGATE    = (1 << 12),
	XCB_CW_COLORMAP          = (1 << 13),
	XCB_CW_CURSOR            = (1 << 14),
} xcb_cw_t;

typedef enum
{
	XCB_CONFIG_WINDOW_X            = (1 << 0),
	XCB_CONFIG_WINDOW_Y            = (1 << 1),
	XCB_CONFIG_WINDOW_WIDTH        = (1 << 2),
	XCB_CONFIG_WINDOW_HEIGHT       = (1 << 3),
	XCB_CONFIG_WINDOW_BORDER_WIDTH = (1 << 4),
	XCB_CONFIG_WINDOW_SIBLING      = (1 << 5),
	XCB_CONFIG_WINDOW_STACK_MODE   = (1 << 6),
} xcb_config_window_t;

typedef enum
{
	XCB_CIRCULATE_RAISE_LOWEST  = 0,
	XCB_CIRCULATE_LOWER_HIGHEST = 1,
} xcb_circulate_t;

typedef enum
{
	XCB_PROP_MODE_REPLACE = 0,
	XCB_PROP_MODE_PREPEND = 1,
	XCB_PROP_MODE_APPEND  = 2,
} xcb_prop_mode_t;

typedef enum
{
	XCB_GC_FUNCTION              = (1 << 0),
	XCB_GC_PLANE_MASK            = (1 << 1),
	XCB_GC_FOREGROUND            = (1 << 2),
	XCB_GC_BACKGROUND            = (1 << 3),
	XCB_GC_LINE_WIDTH            = (1 << 4),
	XCB_GC_LINE_STYLE            = (1 << 5),
	XCB_GC_CAP_STYLE             = (1 << 6),
	XCB_GC_JOIN_STYLE            = (1 << 7),
	XCB_GC_FILL_STYLE            = (1 << 8),
	XCB_GC_FILL_RULE             = (1 << 9),
	XCB_GC_TILE                  = (1 << 10),
	XCB_GC_STIPPLE               = (1 << 11),
	XCB_GC_TILE_STIPPLE_ORIGIN_X = (1 << 12),
	XCB_GC_TILE_STIPPLE_ORIGIN_Y = (1 << 13),
	XCB_GC_FONT                  = (1 << 14),
	XCB_GC_SUBWINDOW_MODE        = (1 << 15),
	XCB_GC_GRAPHICS_EXPOSURES    = (1 << 16),
	XCB_GC_CLIP_ORIGIN_X         = (1 << 17),
	XCB_GC_CLIP_ORIGIN_Y         = (1 << 18),
	XCB_GC_CLIP_MASK             = (1 << 19),
	XCB_GC_DASH_OFFSET           = (1 << 20),
	XCB_GC_DASH_LIST             = (1 << 21),
	XCB_GC_ARC_MODE              = (1 << 22),
} xcb_gc_t;

typedef enum
{
	XCB_COORD_MODE_ORIGIN   = 0,
	XCB_COORD_MODE_PREVIOUS = 1,
} xcb_coord_mode_t;

typedef enum
{
	XCB_IMAGE_FORMAT_XY_BITMAP = 0,
	XCB_IMAGE_FORMAT_XY_PIXMAP = 1,
	XCB_IMAGE_FORMAT_Z_PIXMAP  = 2,
} xcb_image_format_t;

typedef enum
{
	XCB_NOTIFY_DETAIL_ANCESTOR = 0,
	XCB_NOTIFY_DETAIL_VIRTUAL  = 1,
	XCB_NOTIFY_DETAIL_INFERIOR = 2,
	XCB_NOTIFY_DETAIL_NONLINEAR = 3,
	XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL = 4,
} xcb_notify_detail_t;

typedef enum
{
	XCB_NOTIFY_MODE_NORMAL = 0,
	XCB_NOTIFY_MODE_GRAB   = 1,
	XCB_NOTIFY_MODE_UNGRAB = 2,
} xcb_notify_mode_t;

typedef enum
{
	XCB_STACK_MODE_ABOVE     = 0,
	XCB_STACK_MODE_BELOW     = 1,
	XCB_STACK_MODE_TOP_IF    = 2,
	XCB_STACK_MODE_BOTTOM_IF = 3,
	XCB_STACK_MODE_OPPOSITE  = 4,
} xcb_stack_mode_t;

typedef enum
{
	XCB_PLACE_ON_TOP    = 0,
	XCB_PLACE_ON_BOTTOM = 1,
} xcb_place_t;

typedef enum
{
	XCB_PROPERTY_NEW_VALUE = 0,
	XCB_PROPERTY_DELETE    = 1,
} xcb_property_t;

typedef enum
{
	XCB_COLORMAP_UNINSTALLED = 0,
	XCB_COLORMAP_INSTALLED   = 1,
} xcb_colormap_state_t;

typedef enum
{
	XCB_MAPPING_MODIFIER = 0,
	XCB_MAPPING_KEYBOARD = 1,
	XCB_MAPPING_POINTER  = 2,
} xcb_mapping_t;

typedef struct
{
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
} xcb_segment_t;

typedef struct
{
	int16_t x;
	int16_t y;
} xcb_point_t;

typedef struct
{
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
} xcb_rectangle_t;

typedef struct
{
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	int16_t angle1;
	int16_t angle2;
} xcb_arc_t;

typedef struct
{
	uint8_t depth;
	uint8_t bpp;
	uint8_t scanline_pad;
	uint8_t pad0;
	uint32_t pad1;
} xcb_format_t;

typedef struct
{
	xcb_format_t *data;
	int rem;
	int index;
} xcb_format_iterator_t;

typedef struct
{
	xcb_visualid_t visual_id;
	uint8_t _class;
	uint8_t bits_per_rgb_value;
	uint16_t colormap_entries;
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t pad0;
} xcb_visualtype_t;

typedef struct
{
	xcb_visualtype_t *data;
	int rem;
	int index;
} xcb_visualtype_iterator_t;

typedef struct
{
	uint8_t depth;
	uint8_t pad0;
	uint16_t visuals_len;
	uint32_t pad1;
} xcb_depth_t;

typedef struct
{
	xcb_depth_t *data;
	int rem;
	int index;
} xcb_depth_iterator_t;

typedef struct
{
	xcb_window_t root;
	xcb_colormap_t default_colormap;
	uint32_t white_pixel;
	uint32_t black_pixel;
	uint32_t current_input_mask;
	uint16_t width_in_pixels;
	uint16_t height_in_pixels;
	uint16_t width_in_millimeters;
	uint16_t height_in_millimeters;
	uint16_t min_installed_maps;
	uint16_t max_installed_maps;
	xcb_visualid_t root_visual;
	uint8_t backing_stores;
	uint8_t save_unders;
	uint8_t root_depth;
	uint8_t allowed_depths_len;
} xcb_screen_t;

typedef struct
{
	xcb_screen_t *data;
	int rem;
	int index;
} xcb_screen_iterator_t;

typedef struct
{
	uint8_t status;
	uint8_t pad0;
	uint16_t protocol_major_version;
	uint16_t protocol_minor_version;
	uint16_t length;
	uint32_t release_number;
	uint32_t resource_id_base;
	uint32_t resource_id_mask;
	uint32_t motion_buffer_size;
	uint16_t vendor_len;
	uint16_t maximum_request_length;
	uint8_t roots_len;
	uint8_t pixmap_formats_len;
	uint8_t image_byte_order;
	uint8_t bitmap_format_bit_order;
	uint8_t bitmap_format_scanline_unit;
	uint8_t bitmap_format_scanline_pad;
	xcb_keycode_t min_keycode;
	xcb_keycode_t max_keycode;
	uint32_t pad1;
} xcb_setup_t;

typedef struct
{
	uint8_t name_len;
} xcb_str_t;

typedef struct
{
	xcb_str_t *data;
	int rem;
	int index;
} xcb_str_iterator_t;

typedef struct
{
	uint8_t response_type;
	xcb_keycode_t detail;
	uint16_t sequence;
	xcb_timestamp_t time;
	xcb_window_t root;
	xcb_window_t event;
	xcb_window_t child;
	int16_t root_x;
	int16_t root_y;
	int16_t event_x;
	int16_t event_y;
	uint16_t state;
	uint8_t same_screen;
} xcb_key_press_event_t;

typedef xcb_key_press_event_t xcb_key_relase_event_t;

typedef struct
{
	uint8_t response_type;
	xcb_button_t detail;
	uint16_t sequence;
	xcb_timestamp_t time;
	xcb_window_t root;
	xcb_window_t event;
	xcb_window_t child;
	int16_t root_x;
	int16_t root_y;
	int16_t event_x;
	int16_t event_y;
	uint16_t state;
	uint8_t same_screen;
} xcb_button_press_event_t;

typedef xcb_button_press_event_t xcb_button_release_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t detail;
	uint16_t sequence;
	xcb_timestamp_t time;
	xcb_window_t root;
	xcb_window_t event;
	xcb_window_t child;
	int16_t root_x;
	int16_t root_y;
	int16_t event_x;
	int16_t event_y;
	uint16_t state;
	uint8_t same_screen;
} xcb_motion_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t detail;
	uint16_t sequence;
	xcb_timestamp_t time;
	xcb_window_t root;
	xcb_window_t event;
	xcb_window_t child;
	int16_t root_x;
	int16_t root_y;
	int16_t event_x;
	int16_t event_y;
	uint16_t state;
	uint8_t mode;
	uint8_t same_screen_focus;
} xcb_enter_notify_event_t;

typedef xcb_enter_notify_event_t xcb_leave_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t detail;
	uint16_t sequence;
	xcb_window_t event;
	uint8_t mode;
} xcb_focus_in_event_t;

typedef xcb_focus_in_event_t xcb_focus_out_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t keys[31];
} xcb_keymap_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t window;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t count;
} xcb_expose_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_drawable_t drawable;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t minor_opcode;
	uint16_t count;
	uint8_t major_opcode;
} xcb_graphics_exposure_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_drawable_t drawable;
	uint16_t minor_opcode;
	uint8_t major_opcode;
} xcb_no_exposure_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t window;
	uint8_t state;
} xcb_visibility_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t parent;
	xcb_window_t window;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t border_width;
	xcb_bool_t override_redirect;
} xcb_create_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t event;
	xcb_window_t window;
} xcb_destroy_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t event;
	xcb_window_t window;
	xcb_bool_t from_configure;
} xcb_unmap_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t event;
	xcb_window_t window;
	xcb_bool_t override_redirect;
} xcb_map_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t parent;
	xcb_window_t window;
} xcb_map_request_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t event;
	xcb_window_t window;
	xcb_window_t parent;
	int16_t x;
	int16_t y;
	xcb_bool_t override_redirect;
} xcb_reparent_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t event;
	xcb_window_t window;
	xcb_window_t above_sibling;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t border_width;
	xcb_bool_t override_redirect;
} xcb_configure_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t stack_mode;
	uint16_t sequence;
	xcb_window_t parent;
	xcb_window_t window;
	xcb_window_t sibling;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t border_width;
	uint16_t value_mask;
} xcb_configure_request_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t event;
	xcb_window_t window;
	int16_t x;
	int16_t y;
} xcb_gravity_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t window;
	uint16_t width;
	uint16_t height;
} xcb_resize_request_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t event;
	xcb_window_t window;
	uint32_t pad1;
	uint8_t place;
} xcb_circulate_notify_event_t;

typedef xcb_circulate_notify_event_t xcb_circulate_request_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t window;
	xcb_atom_t atom;
	xcb_timestamp_t time;
	uint8_t state;
} xcb_property_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_timestamp_t time;
	xcb_window_t owner;
	xcb_atom_t selection;
} xcb_selection_clear_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_timestamp_t time;
	xcb_window_t owner;
	xcb_window_t requestor;
	xcb_atom_t selection;
	xcb_atom_t target;
	xcb_atom_t property;
} xcb_selection_request_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_timestamp_t time;
	xcb_window_t requestor;
	xcb_atom_t selection;
	xcb_atom_t target;
	xcb_atom_t property;
} xcb_selection_notify_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	xcb_window_t window;
	xcb_colormap_t colormap;
	xcb_bool_t _new;
	uint8_t state;
} xcb_colormap_notify_event_t;

typedef struct
{
	uint8_t data8[20];
	uint16_t data16[10];
	uint32_t data32[5];
} xcb_client_message_data_t;

typedef struct
{
	uint8_t response_type;
	uint8_t format;
	uint16_t sequence;
	xcb_window_t window;
	xcb_atom_t type;
	xcb_client_message_data_t data;
} xcb_client_message_event_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint8_t request;
	xcb_keycode_t first_keycode;
	uint8_t count;
} xcb_mapping_notify_event_t;

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_list_extensions_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_list_extensions_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t names_len;
	uint16_t sequence;
	uint32_t length;
	uint8_t pad0[24];
} xcb_list_extensions_reply_t;

xcb_list_extensions_cookie_t xcb_list_extensions(xcb_connection_t *conn);

xcb_list_extensions_reply_t *xcb_list_extensions_reply(xcb_connection_t *conn,
                                                       xcb_list_extensions_cookie_t cookie,
                                                       xcb_generic_error_t **error);

int xcb_list_extensions_names_length(const xcb_list_extensions_reply_t *reply);

xcb_str_iterator_t xcb_list_extensions_names_iterator(const xcb_list_extensions_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	uint16_t name_len;
	uint16_t pad1;
} xcb_query_extension_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_query_extension_cookie_t;

typedef struct xcb_query_extension_reply_t
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint8_t present;
	uint8_t major_opcode;
	uint8_t first_event;
	uint8_t first_error;
} xcb_query_extension_reply_t;

xcb_query_extension_cookie_t xcb_query_extension(xcb_connection_t *conn,
                                                 uint16_t name_len,
                                                 const char *name);

xcb_query_extension_reply_t *xcb_query_extension_reply(xcb_connection_t *conn,
                                                       xcb_query_extension_cookie_t cookie,
                                                       xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t depth;
	uint16_t length;
	xcb_window_t wid;
	xcb_window_t parent;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t border_width;
	uint16_t _class;
	xcb_visualid_t visual;
	uint32_t value_mask;
} xcb_create_window_request_t;

xcb_void_cookie_t xcb_create_window(xcb_connection_t *conn,
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
                                    const uint32_t *value_list);

xcb_void_cookie_t xcb_create_window_checked(xcb_connection_t *conn,
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
                                            const uint32_t *value_list);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_destroy_window_request_t;

xcb_void_cookie_t xcb_destroy_window(xcb_connection_t *conn,
                                     xcb_window_t window);

xcb_void_cookie_t xcb_destroy_window_checked(xcb_connection_t *conn,
                                             xcb_window_t window);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_destroy_subwindows_request_t;

xcb_void_cookie_t xcb_destroy_subwindows(xcb_connection_t *conn,
                                         xcb_window_t window);

xcb_void_cookie_t xcb_destroy_subwindows_checked(xcb_connection_t *conn,
                                                 xcb_window_t window);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_map_window_request_t;

xcb_void_cookie_t xcb_map_window(xcb_connection_t *conn,
                                 xcb_window_t window);

xcb_void_cookie_t xcb_map_window_checked(xcb_connection_t *conn,
                                         xcb_window_t window);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_unmap_window_request_t;

xcb_void_cookie_t xcb_unmap_window(xcb_connection_t *conn,
                                   xcb_window_t window);

xcb_void_cookie_t xcb_unmap_window_checked(xcb_connection_t *conn,
                                           xcb_window_t window);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_map_subwindows_request_t;

xcb_void_cookie_t xcb_map_subwindows(xcb_connection_t *conn,
                                     xcb_window_t window);

xcb_void_cookie_t xcb_map_subwindows_checked(xcb_connection_t *conn,
                                             xcb_window_t window);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_unmap_subwindows_request_t;

xcb_void_cookie_t xcb_unmap_subwindows(xcb_connection_t *conn,
                                       xcb_window_t window);

xcb_void_cookie_t xcb_unmap_subwindows_checked(xcb_connection_t *conn,
                                               xcb_window_t window);

typedef struct
{
	uint8_t major_opcode;
	uint8_t only_if_exists;
	uint16_t length;
	uint16_t name_len;
	uint8_t pad0[2];
} xcb_intern_atom_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_intern_atom_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	xcb_atom_t atom;
} xcb_intern_atom_reply_t;

xcb_intern_atom_cookie_t xcb_intern_atom(xcb_connection_t *conn,
                                         uint8_t only_if_exists,
                                         uint16_t name_len,
                                         const char *name);

xcb_intern_atom_reply_t *xcb_intern_atom_reply(xcb_connection_t *conn,
                                               xcb_intern_atom_cookie_t cookie,
                                               xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_atom_t atom;
} xcb_get_atom_name_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_atom_name_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint16_t name_len;
} xcb_get_atom_name_reply_t;

xcb_get_atom_name_cookie_t xcb_get_atom_name(xcb_connection_t *conn,
                                             xcb_atom_t atom);

xcb_get_atom_name_reply_t *xcb_get_atom_name_reply(xcb_connection_t *conn,
                                                   xcb_get_atom_name_cookie_t cookie,
                                                   xcb_generic_error_t **error);

char *xcb_get_atom_name_name(const xcb_get_atom_name_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t mode;
	uint16_t length;
	xcb_window_t window;
	xcb_atom_t property;
	xcb_atom_t type;
	uint8_t format;
	uint8_t pad0[3];
	uint32_t data_len;
} xcb_change_property_request_t;

xcb_void_cookie_t xcb_change_property(xcb_connection_t *conn,
                                      xcb_prop_mode_t mode,
                                      xcb_window_t window,
                                      xcb_atom_t property,
                                      xcb_atom_t type,
                                      uint8_t format,
                                      uint32_t data_length,
                                      const void *data);

xcb_void_cookie_t xcb_change_property_checked(xcb_connection_t *conn,
                                              xcb_prop_mode_t mode,
                                              xcb_window_t window,
                                              xcb_atom_t property,
                                              xcb_atom_t type,
                                              uint8_t format,
                                              uint32_t data_length,
                                              const void *data);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
	uint16_t value_mask;
	uint16_t pad1;
} xcb_configure_window_request_t;

xcb_void_cookie_t xcb_configure_window(xcb_connection_t *conn,
                                       xcb_window_t window,
                                       uint16_t value_mask,
                                       uint32_t *value_list);

xcb_void_cookie_t xcb_configure_window_checked(xcb_connection_t *conn,
                                               xcb_window_t window,
                                               uint16_t value_mask,
                                               uint32_t *value_list);

typedef struct
{
	uint8_t major_opcode;
	uint8_t direction;
	uint16_t length;
	xcb_window_t window;
} xcb_circulate_window_request_t;

xcb_void_cookie_t xcb_circulate_window(xcb_connection_t *conn,
                                       uint8_t direction,
                                       xcb_window_t window);

xcb_void_cookie_t xcb_circulate_window_checked(xcb_connection_t *conn,
                                               uint8_t direction,
                                               xcb_window_t window);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
	uint32_t value_mask;
} xcb_change_window_attributes_request_t;

xcb_void_cookie_t xcb_change_window_attributes(xcb_connection_t *conn,
                                               xcb_window_t window,
                                               uint32_t value_mask,
                                               uint32_t *values);

xcb_void_cookie_t xcb_change_window_attributes_checked(xcb_connection_t *conn,
                                                       xcb_window_t window,
                                                       uint32_t value_mask,
                                                       uint32_t *values);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_get_window_attributes_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_window_attributes_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t backing_store;
	uint16_t sequence;
	uint32_t length;
	xcb_visualid_t visual;
	uint16_t _class;
	uint8_t bit_gravity;
	uint8_t win_gravity;
	uint32_t backing_planes;
	uint32_t backing_pixel;
	uint8_t save_under;
	uint8_t map_is_installed;
	uint8_t map_state;
	uint8_t override_redirect;
	xcb_colormap_t colormap;
	uint32_t all_event_masks;
	uint32_t your_event_mask;
	uint16_t do_not_propagate_mask;
	uint16_t pad0;
} xcb_get_window_attributes_reply_t;

xcb_get_window_attributes_cookie_t xcb_get_window_attributes(xcb_connection_t *conn,
                                                             xcb_window_t window);

xcb_get_window_attributes_reply_t *xcb_get_window_attributes_reply(xcb_connection_t *conn,
                                                                   xcb_get_window_attributes_cookie_t cookie,
                                                                   xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_get_input_focus_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_input_focus_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t revert_to;
	uint16_t sequence;
	uint32_t length;
	xcb_window_t focus;
} xcb_get_input_focus_reply_t;

xcb_get_input_focus_cookie_t xcb_get_input_focus(xcb_connection_t *conn);

xcb_get_input_focus_reply_t *xcb_get_input_focus_reply(xcb_connection_t *conn,
                                                       xcb_get_input_focus_cookie_t cookie,
                                                       xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t revert_to;
	uint16_t length;
	xcb_window_t focus;
	xcb_timestamp_t time;
} xcb_set_input_focus_request_t;

xcb_void_cookie_t xcb_set_input_focus(xcb_connection_t *conn,
                                      uint8_t revert_to,
                                      xcb_window_t focus,
                                      xcb_timestamp_t time);

xcb_void_cookie_t xcb_set_input_focus_checked(xcb_connection_t *conn,
                                              uint8_t revert_to,
                                              xcb_window_t focus,
                                              xcb_timestamp_t time);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_gcontext_t cid;
	xcb_drawable_t drawable;
	uint32_t value_mask;
} xcb_create_gc_request_t;

xcb_void_cookie_t xcb_create_gc(xcb_connection_t *conn,
                                xcb_gcontext_t cid,
                                xcb_drawable_t drawable,
                                uint32_t value_mask,
                                const uint32_t *values);

xcb_void_cookie_t xcb_create_gc_checked(xcb_connection_t *conn,
                                        xcb_gcontext_t cid,
                                        xcb_drawable_t drawable,
                                        uint32_t value_mask,
                                        const uint32_t *values);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_gcontext_t src_gc;
	xcb_gcontext_t dst_gc;
	uint32_t value_mask;
} xcb_copy_gc_request_t;

xcb_void_cookie_t xcb_copy_gc(xcb_connection_t *conn, 
                              xcb_gcontext_t src_gc,
                              xcb_gcontext_t dst_gc,
                              uint32_t value_mask);

xcb_void_cookie_t xcb_copy_gc_checked(xcb_connection_t *conn, 
                                      xcb_gcontext_t src_gc,
                                      xcb_gcontext_t dst_gc,
                                      uint32_t value_mask);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_gcontext_t gc;
} xcb_free_gc_request_t;

xcb_void_cookie_t xcb_free_gc(xcb_connection_t *conn,
                              xcb_gcontext_t gc);

xcb_void_cookie_t xcb_free_gc_checked(xcb_connection_t *conn,
                                      xcb_gcontext_t gc);

typedef struct
{
	uint8_t major_opcode;
	uint8_t coordinate_mode;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
} xcb_poly_point_request_t;

xcb_void_cookie_t xcb_poly_point(xcb_connection_t *conn,
                                 uint8_t coordinate_mode,
                                 xcb_drawable_t drawable,
                                 xcb_gcontext_t gc,
                                 uint32_t points_len,
                                 const xcb_point_t *points);

xcb_void_cookie_t xcb_poly_point_checked(xcb_connection_t *conn,
                                         uint8_t coordinate_mode,
                                         xcb_drawable_t drawable,
                                         xcb_gcontext_t gc,
                                         uint32_t points_len,
                                         const xcb_point_t *points);

typedef struct
{
	uint8_t major_opcode;
	uint8_t coordinate_mode;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
} xcb_poly_line_request_t;

xcb_void_cookie_t xcb_poly_line(xcb_connection_t *conn,
                                uint8_t coordinate_mode,
                                xcb_drawable_t drawable, xcb_gcontext_t gc,
                                uint32_t points_len,
                                const xcb_point_t *points);

xcb_void_cookie_t xcb_poly_line_checked(xcb_connection_t *conn,
                                        uint8_t coordinate_mode,
                                        xcb_drawable_t drawable,
                                        xcb_gcontext_t gc,
                                        uint32_t points_len,
                                        const xcb_point_t *points);
typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
} xcb_poly_segment_request_t;

xcb_void_cookie_t xcb_poly_segment(xcb_connection_t *conn,
                                   xcb_drawable_t drawable,
                                   xcb_gcontext_t gc,
                                   uint32_t segments_len,
                                   const xcb_segment_t *segments);

xcb_void_cookie_t xcb_poly_segment_checked(xcb_connection_t *conn,
                                           xcb_drawable_t drawable,
                                           xcb_gcontext_t gc,
                                           uint32_t segments_len,
                                           const xcb_segment_t *segments);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
} xcb_poly_rectangle_request_t;

xcb_void_cookie_t xcb_poly_rectangle(xcb_connection_t *conn,
                                     xcb_drawable_t drawable,
                                     xcb_gcontext_t gc,
                                     uint32_t rectangles_len,
                                     const xcb_rectangle_t *rectangles);

xcb_void_cookie_t xcb_poly_rectangle_checked(xcb_connection_t *conn,
                                           xcb_drawable_t drawable,
                                           xcb_gcontext_t gc,
                                           uint32_t rectangles_len,
                                           const xcb_rectangle_t *rectangles);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
} xcb_poly_arc_request_t;

xcb_void_cookie_t xcb_poly_arc(xcb_connection_t *conn,
                               xcb_drawable_t drawable,
                               xcb_gcontext_t gc,
                               uint32_t arcs_len,
                               const xcb_arc_t *arcs);

xcb_void_cookie_t xcb_poly_arc_checked(xcb_connection_t *conn,
                                       xcb_drawable_t drawable,
                                       xcb_gcontext_t gc,
                                       uint32_t arcs_len,
                                       const xcb_arc_t *arcs);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
	uint8_t shape;
	uint8_t coordinate_mode;
	uint16_t pad1;
} xcb_fill_poly_request_t;

xcb_void_cookie_t xcb_fill_poly(xcb_connection_t *conn,
                                xcb_drawable_t drawable,
                                xcb_gcontext_t gc,
                                uint8_t shape,
                                uint8_t coordinate_mode,
                                uint32_t points_len,
                                const xcb_point_t *points);

xcb_void_cookie_t xcb_fill_poly_checked(xcb_connection_t *conn,
                                        xcb_drawable_t drawable,
                                        xcb_gcontext_t gc,
                                        uint8_t shape,
                                        uint8_t coordinate_mode,
                                        uint32_t points_len,
                                        const xcb_point_t *points);
typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
} xcb_poly_fill_rectangle_request_t;

xcb_void_cookie_t xcb_poly_fill_rectangle(xcb_connection_t *conn,
                                          xcb_drawable_t drawable,
                                          xcb_gcontext_t gc,
                                          uint32_t rectangles_len,
                                          const xcb_rectangle_t *rectangles);

xcb_void_cookie_t xcb_poly_fill_rectangle_checked(xcb_connection_t *conn,
                                                  xcb_drawable_t drawable,
                                                  xcb_gcontext_t gc,
                                                  uint32_t rectangles_len,
                                                  const xcb_rectangle_t *rectangles);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
} xcb_poly_fill_arc_request_t;

xcb_void_cookie_t xcb_poly_fill_arc(xcb_connection_t *conn,
                                    xcb_drawable_t drawable,
                                    xcb_gcontext_t gc,
                                    uint32_t arcs_len,
                                    const xcb_arc_t *arcs);

xcb_void_cookie_t xcb_poly_fill_arc_checked(xcb_connection_t *conn,
                                            xcb_drawable_t drawable,
                                            xcb_gcontext_t gc,
                                            uint32_t arcs_len,
                                            const xcb_arc_t *arcs);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_font_t fid;
	uint16_t name_len;
	uint16_t pad1;
} xcb_open_font_request_t;

xcb_void_cookie_t xcb_open_font(xcb_connection_t *conn,
                                xcb_font_t fid,
                                uint16_t name_len,
                                const char *name);
xcb_void_cookie_t xcb_open_font_checked(xcb_connection_t *conn,
                                        xcb_font_t fid,
                                        uint16_t name_len,
                                        const char *name);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_font_t font;
} xcb_close_font_request_t;

xcb_void_cookie_t xcb_close_font(xcb_connection_t *conn,
                                 xcb_font_t font);
xcb_void_cookie_t xcb_close_font_checked(xcb_connection_t *conn,
                                         xcb_font_t font);

typedef struct
{
	uint8_t major_opcode;
	uint8_t n;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
	int16_t x;
	int16_t y;
} xcb_image_text_8_request_t;

xcb_void_cookie_t xcb_image_text_8(xcb_connection_t *conn,
                                   xcb_drawable_t drawable,
                                   xcb_gcontext_t gc,
                                   int16_t x,
                                   int16_t y,
                                   const char *string);
xcb_void_cookie_t xcb_image_text_8_checked(xcb_connection_t *conn,
                                           xcb_drawable_t drawable,
                                           xcb_gcontext_t gc,
                                           int16_t x,
                                           int16_t y,
                                           const char *string);

typedef struct
{
	uint8_t major_opcode;
	uint8_t odd;
	uint16_t length;
	xcb_fontable_t fontable;
} xcb_query_text_extents_request_t;

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_cursor_t cid;
	xcb_font_t source_font;
	xcb_font_t mask_font;
	uint16_t source_char;
	uint16_t mask_char;
	uint16_t fore_red;
	uint16_t fore_green;
	uint16_t fore_blue;
	uint16_t back_red;
	uint16_t back_green;
	uint16_t back_blue;
} xcb_create_glyph_cursor_request_t;

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
                                          uint16_t back_blue);

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
                                                  uint16_t back_blue);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_cursor_t cid;
	xcb_pixmap_t source;
	xcb_pixmap_t mask;
	uint16_t fore_red;
	uint16_t fore_green;
	uint16_t fore_blue;
	uint16_t back_red;
	uint16_t back_green;
	uint16_t back_blue;
	uint16_t x;
	uint16_t y;
} xcb_create_cursor_request_t;

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
                                    uint16_t y);

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
                                            uint16_t y);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_cursor_t cursor;
	uint16_t fore_red;
	uint16_t fore_green;
	uint16_t fore_blue;
	uint16_t back_red;
	uint16_t back_green;
	uint16_t back_blue;
} xcb_recolor_cursor_request_t;

xcb_void_cookie_t xcb_recolor_cursor(xcb_connection_t *conn,
                                     xcb_cursor_t cursor,
                                     uint16_t fore_red,
                                     uint16_t fore_green,
                                     uint16_t fore_blue,
                                     uint16_t back_red,
                                     uint16_t back_green,
                                     uint16_t back_blue);

xcb_void_cookie_t xcb_recolor_cursor_checked(xcb_connection_t *conn,
                                             xcb_cursor_t cursor,
                                             uint16_t fore_red,
                                             uint16_t fore_green,
                                             uint16_t fore_blue,
                                             uint16_t back_red,
                                             uint16_t back_green,
                                             uint16_t back_blue);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_cursor_t cursor;
} xcb_free_cursor_request_t;

xcb_void_cookie_t xcb_free_cursor(xcb_connection_t *conn,
                                  xcb_cursor_t cursor);

xcb_void_cookie_t xcb_free_cursor_checked(xcb_connection_t *conn,
                                          xcb_cursor_t cursor);

typedef struct
{
	uint8_t major_opcode;
	uint8_t depth;
	uint16_t length;
	xcb_pixmap_t pid;
	xcb_drawable_t drawable;
	uint16_t width;
	uint16_t height;
} xcb_create_pixmap_request_t;

xcb_void_cookie_t xcb_create_pixmap(xcb_connection_t *conn,
                                    uint8_t depth,
                                    xcb_pixmap_t pid,
                                    xcb_drawable_t drawable,
                                    uint16_t width,
                                    uint16_t height);

xcb_void_cookie_t xcb_create_pixmap_checked(xcb_connection_t *conn,
                                            uint8_t depth,
                                            xcb_pixmap_t pid,
                                            xcb_drawable_t drawable,
                                            uint16_t width,
                                            uint16_t height);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_pixmap_t pixmap;
} xcb_free_pixmap_request_t;

xcb_void_cookie_t xcb_free_pixmap(xcb_connection_t *conn,
                                  xcb_pixmap_t pixmap);
xcb_void_cookie_t xcb_free_pixmap_checked(xcb_connection_t *conn,
                                          xcb_pixmap_t pixmap);

typedef struct
{
	uint8_t major_opcode;
	uint8_t format;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
	uint16_t width;
	uint16_t height;
	int16_t dst_x;
	int16_t dst_y;
	uint8_t left_pad;
	uint8_t depth;
	uint16_t pad0;
} xcb_put_image_request_t;

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
                                const uint8_t *data);

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
                                        const uint8_t *data);

typedef struct
{
	uint8_t major_opcode;
	uint8_t format;
	uint16_t length;
	xcb_drawable_t drawable;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	uint32_t plane_mask;
} xcb_get_image_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_image_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t depth;
	uint16_t sequence;
	uint32_t length;
	xcb_visualid_t visual;
} xcb_get_image_reply_t;

xcb_get_image_cookie_t xcb_get_image(xcb_connection_t *conn,
                                     uint8_t format,
                                     xcb_drawable_t drawable,
                                     int16_t x,
                                     int16_t y,
                                     uint16_t width,
                                     uint16_t height,
                                     uint32_t plane_mask);

xcb_get_image_reply_t *xcb_get_image_reply(xcb_connection_t *conn,
                                           xcb_get_image_cookie_t cookie,
                                           xcb_generic_error_t **error);

uint8_t *xcb_get_image_data(const xcb_get_image_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_drawable_t src_drawable;
	xcb_drawable_t dst_drawable;
	xcb_gcontext_t gc;
	int16_t src_x;
	int16_t src_y;
	int16_t dst_x;
	int16_t dst_y;
	uint16_t width;
	uint16_t height;
} xcb_copy_area_request_t;

xcb_void_cookie_t xcb_copy_area(xcb_connection_t *conn,
                                xcb_drawable_t src_drawable,
                                xcb_drawable_t dst_drawable,
                                xcb_gcontext_t gc,
                                int16_t src_x,
                                int16_t src_y,
                                int16_t dst_x,
                                int16_t dst_y,
                                uint16_t width,
                                uint16_t height);

xcb_void_cookie_t xcb_copy_area_checked(xcb_connection_t *conn,
                                        xcb_drawable_t src_drawable,
                                        xcb_drawable_t dst_drawable,
                                        xcb_gcontext_t gc,
                                        int16_t src_x,
                                        int16_t src_y,
                                        int16_t dst_x,
                                        int16_t dst_y,
                                        uint16_t width,
                                        uint16_t height);

typedef struct
{
	uint8_t major_opcode;
	uint8_t exposures;
	uint16_t length;
	xcb_window_t window;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
} xcb_clear_area_request_t;

xcb_void_cookie_t xcb_clear_area(xcb_connection_t *conn,
                                 uint8_t exposures,
                                 xcb_window_t window,
                                 int16_t x,
                                 int16_t y,
                                 uint16_t width,
                                 uint16_t height);

xcb_void_cookie_t xcb_clear_area_checked(xcb_connection_t *conn,
                                         uint8_t exposures,
                                         xcb_window_t window,
                                         int16_t x,
                                         int16_t y,
                                         uint16_t width,
                                         uint16_t height);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	uint16_t max_names;
	uint16_t pattern_len;
} xcb_list_fonts_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_list_fonts_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint16_t names_len;
	uint8_t pad1[22];
} xcb_list_fonts_reply_t;

xcb_list_fonts_cookie_t xcb_list_fonts(xcb_connection_t *conn,
                                       uint16_t max_names,
                                       uint16_t pattern_len,
                                       const char *pattern);

xcb_list_fonts_reply_t *xcb_list_fonts_reply(xcb_connection_t *conn,
                                             xcb_list_fonts_cookie_t cookie,
                                             xcb_generic_error_t **error);

int xcb_list_fonts_names_length(const xcb_list_fonts_reply_t *reply);

xcb_str_iterator_t xcb_list_fonts_names_iterator(const xcb_list_fonts_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t keycode_count;
	uint16_t length;
	uint8_t first_keycode;
	uint8_t keysyms_per_keycode;
	uint16_t pad0;
} xcb_change_keyboard_mapping_request_t;

xcb_void_cookie_t xcb_change_keyboard_mapping(xcb_connection_t *conn,
                                              uint8_t keycode_count,
                                              xcb_keycode_t first_keycode,
                                              uint8_t keysyms_per_keycode,
                                              const xcb_keysym_t *keysyms);

xcb_void_cookie_t xcb_change_keyboard_mapping_checked(xcb_connection_t *conn,
                                                      uint8_t keycode_count,
                                                      xcb_keycode_t first_keycode,
                                                      uint8_t keysyms_per_keycode,
                                                      const xcb_keysym_t *keysyms);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_keycode_t first_keycode;
	uint8_t count;
	uint16_t pad1;
} xcb_get_keyboard_mapping_request_t;

typedef struct
{
	uint32_t sequence;
	uint8_t count;
} xcb_get_keyboard_mapping_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t keysyms_per_keycode;
	uint16_t sequence;
	uint32_t length;
	uint8_t pad1[23];
} xcb_get_keyboard_mapping_reply_t;

xcb_get_keyboard_mapping_cookie_t xcb_get_keyboard_mapping(xcb_connection_t *conn,
                                                           xcb_keycode_t first_keycode,
                                                           uint8_t count);

xcb_get_keyboard_mapping_reply_t *xcb_get_keyboard_mapping_reply(xcb_connection_t *conn,
                                                                 xcb_get_keyboard_mapping_cookie_t cookie,
                                                                 xcb_generic_error_t **error);

int xcb_get_keyboard_mapping_keysyms_length(const xcb_get_keyboard_mapping_reply_t *reply);

xcb_keysym_t *xcb_get_keyboard_mapping_keysyms(const xcb_get_keyboard_mapping_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_gcontext_t gc;
	uint32_t value_mask;
} xcb_change_gc_request_t;

xcb_void_cookie_t xcb_change_gc(xcb_connection_t *conn,
                                xcb_gcontext_t gc,
                                uint32_t value_mask,
                                const uint32_t *values);

xcb_void_cookie_t xcb_change_gc_checked(xcb_connection_t *conn,
                                        xcb_gcontext_t gc,
                                        uint32_t value_mask,
                                        const uint32_t *values);

typedef struct
{
	uint8_t major_opcode;
	int8_t percent;
	uint16_t length;
} xcb_bell_request_t;

xcb_void_cookie_t xcb_bell(xcb_connection_t *conn,
                           int8_t percent);

xcb_void_cookie_t xcb_bell_checked(xcb_connection_t *conn,
                                   int8_t percent);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_query_pointer_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_query_pointer_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t same_screen;
	uint16_t sequence;
	uint32_t length;
	xcb_window_t root;
	xcb_window_t child;
	int16_t root_x;
	int16_t root_y;
	int16_t win_x;
	int16_t win_y;
	uint16_t mask;
} xcb_query_pointer_reply_t;

xcb_query_pointer_cookie_t xcb_query_pointer(xcb_connection_t *conn,
                                             xcb_window_t window);

xcb_query_pointer_reply_t *xcb_query_pointer_reply(xcb_connection_t *conn,
                                                   xcb_query_pointer_cookie_t cookie,
                                                   xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t src_window;
	xcb_window_t dst_window;
	int16_t src_x;
	int16_t src_y;
	uint16_t src_width;
	uint16_t src_height;
	int16_t dst_x;
	int16_t dst_y;
} xcb_warp_pointer_request_t;

xcb_void_cookie_t xcb_warp_pointer(xcb_connection_t *conn,
                                   xcb_window_t src_window,
                                   xcb_window_t dst_window,
                                   int16_t src_x,
                                   int16_t src_y,
                                   uint16_t src_width,
                                   uint16_t src_height,
                                   int16_t dst_x,
                                   int16_t dst_y);

xcb_void_cookie_t xcb_warp_pointer_checked(xcb_connection_t *conn,
                                           xcb_window_t src_window,
                                           xcb_window_t dst_window,
                                           int16_t src_x,
                                           int16_t src_y,
                                           uint16_t src_width,
                                           uint16_t src_height,
                                           int16_t dst_x,
                                           int16_t dst_y);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_grab_server_request_t;

xcb_void_cookie_t xcb_grab_server(xcb_connection_t *conn);

xcb_void_cookie_t xcb_grab_server_checked(xcb_connection_t *conn);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_ungrab_server_request_t;

xcb_void_cookie_t xcb_ungrab_server(xcb_connection_t *conn);

xcb_void_cookie_t xcb_ungrab_server_checked(xcb_connection_t *conn);

typedef struct
{
	uint8_t major_opcode;
	uint8_t keycodes_per_modifier;
	uint16_t length;
} xcb_set_modifier_mapping_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_set_modifier_mapping_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t status;
	uint16_t sequence;
	uint32_t length;
} xcb_set_modifier_mapping_reply_t;

xcb_set_modifier_mapping_cookie_t xcb_set_modifier_mapping(xcb_connection_t *conn,
                                                           uint8_t keycodes_per_modifier,
                                                           const xcb_keycode_t *keycodes);

xcb_set_modifier_mapping_reply_t *xcb_set_modifier_mapping_reply(xcb_connection_t *conn,
                                                                 xcb_set_modifier_mapping_cookie_t cookie,
                                                                 xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_get_modifier_mapping_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_modifier_mapping_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t keycodes_per_modifier;
	uint16_t sequence;
	uint32_t length;
} xcb_get_modifier_mapping_reply_t;

xcb_get_modifier_mapping_cookie_t xcb_get_modifier_mapping(xcb_connection_t *conn);

xcb_get_modifier_mapping_reply_t *xcb_get_modifier_mapping_reply(xcb_connection_t *conn,
                                                                 xcb_get_modifier_mapping_cookie_t cookie,
                                                                 xcb_generic_error_t **error);

xcb_keycode_t *xcb_get_modifier_mapping_keycodes(xcb_get_modifier_mapping_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_no_operation_request_t;

xcb_void_cookie_t xcb_no_operation(xcb_connection_t *conn);

xcb_void_cookie_t xcb_no_operation_checked(xcb_connection_t *conn);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
	xcb_window_t parent;
	int16_t x;
	int16_t y;
} xcb_reparent_window_request_t;

xcb_void_cookie_t xcb_reparent_window(xcb_connection_t *conn,
                                      xcb_window_t window,
                                      xcb_window_t parent,
                                      int16_t x,
                                      int16_t y);

xcb_void_cookie_t xcb_reparent_window_checked(xcb_connection_t *conn,
                                              xcb_window_t window,
                                              xcb_window_t parent,
                                              int16_t x,
                                              int16_t y);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_drawable_t drawable;
} xcb_get_geometry_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_geometry_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t depth;
	uint16_t sequence;
	uint32_t length;
	xcb_window_t root;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t border_width;
} xcb_get_geometry_reply_t;

xcb_get_geometry_cookie_t xcb_get_geometry(xcb_connection_t *conn,
                                           xcb_drawable_t drawable);

xcb_get_geometry_reply_t *xcb_get_geometry_reply(xcb_connection_t *conn,
                                                 xcb_get_geometry_cookie_t cookie,
                                                 xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_query_tree_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_query_tree_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	xcb_window_t root;
	xcb_window_t parent;
	uint16_t children_len;
} xcb_query_tree_reply_t;

xcb_query_tree_cookie_t xcb_query_tree(xcb_connection_t *conn,
                                       xcb_window_t window);

xcb_query_tree_reply_t *xcb_query_tree_reply(xcb_connection_t *conn,
                                             xcb_query_tree_cookie_t cookie,
                                             xcb_generic_error_t **error);

xcb_window_t *xcb_query_tree_children(const xcb_query_tree_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t _delete;
	uint16_t length;
	xcb_window_t window;
	xcb_atom_t property;
	xcb_atom_t type;
	uint32_t long_offset;
	uint32_t long_length;
} xcb_get_property_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_property_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t format;
	uint16_t sequence;
	uint32_t length;
	xcb_atom_t type;
	uint32_t bytes_after;
	uint32_t value_len;
} xcb_get_property_reply_t;

xcb_get_property_cookie_t xcb_get_property(xcb_connection_t *conn,
                                           uint8_t _delete,
                                           xcb_window_t window,
                                           xcb_atom_t property,
                                           xcb_atom_t type,
                                           uint32_t long_offset,
                                           uint32_t long_length);

xcb_get_property_reply_t *xcb_get_property_reply(xcb_connection_t *conn,
                                                 xcb_get_property_cookie_t cookie,
                                                 xcb_generic_error_t **error);

void *xcb_get_property_value(const xcb_get_property_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
} xcb_list_properties_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_list_properties_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint16_t atoms_len;
} xcb_list_properties_reply_t;

xcb_list_properties_cookie_t xcb_list_properties(xcb_connection_t *conn,
                                                 xcb_window_t window);

xcb_list_properties_reply_t *xcb_list_properties_reply(xcb_connection_t *conn,
                                                       xcb_list_properties_cookie_t cookie,
                                                       xcb_generic_error_t **error);

xcb_atom_t *xcb_list_properties_atoms(const xcb_list_properties_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t owner_events;
	uint16_t length;
	xcb_window_t grab_window;
	uint16_t event_mask;
	uint8_t pointer_mode;
	uint8_t keyboard_mode;
	xcb_window_t confine_to;
	xcb_cursor_t cursor;
	xcb_button_t button;
	uint8_t pad0;
	uint16_t modifiers;
} xcb_grab_button_request_t;

xcb_void_cookie_t xcb_grab_button(xcb_connection_t *conn,
                                  uint8_t owner_events,
                                  xcb_window_t grab_window,
                                  uint16_t event_mask,
                                  uint8_t pointer_mode,
                                  uint8_t keyboard_mode,
                                  xcb_window_t confine_to,
                                  xcb_cursor_t cursor,
                                  xcb_button_t button,
                                  uint16_t modifiers);

xcb_void_cookie_t xcb_grab_button_checked(xcb_connection_t *conn,
                                          uint8_t owner_events,
                                          xcb_window_t grab_window,
                                          uint16_t event_mask,
                                          uint8_t pointer_mode,
                                          uint8_t keyboard_mode,
                                          xcb_window_t confine_to,
                                          xcb_cursor_t cursor,
                                          xcb_button_t button,
                                          uint16_t modifiers);

typedef struct
{
	uint8_t major_opcode;
	xcb_button_t button;
	uint16_t length;
	xcb_window_t grab_window;
	uint16_t modifiers;
	uint16_t pad0;
} xcb_ungrab_button_request_t;

xcb_void_cookie_t xcb_ungrab_button(xcb_connection_t *conn,
                                    xcb_button_t button,
                                    xcb_window_t grab_window,
                                    uint16_t modifiers);

xcb_void_cookie_t xcb_ungrab_button_checked(xcb_connection_t *conn,
                                            xcb_button_t button,
                                            xcb_window_t grab_window,
                                            uint16_t modifiers);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
	xcb_atom_t property;
} xcb_delete_property_request_t;

xcb_void_cookie_t xcb_delete_property(xcb_connection_t *conn,
                                      xcb_window_t window,
                                      xcb_property_t property);

xcb_void_cookie_t xcb_delete_property_checked(xcb_connection_t *conn,
                                              xcb_window_t window,
                                              xcb_property_t property);

typedef struct
{
	uint8_t major_opcode;
	uint8_t owner_events;
	uint16_t length;
	xcb_window_t grab_window;
	uint16_t event_mask;
	uint8_t pointer_mode;
	uint8_t keyboard_mode;
	xcb_window_t confine_to;
	xcb_cursor_t cursor;
	xcb_timestamp_t time;
} xcb_grab_pointer_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_grab_pointer_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t status;
	uint16_t sequence;
	uint32_t length;
} xcb_grab_pointer_reply_t;

xcb_grab_pointer_cookie_t xcb_grab_pointer(xcb_connection_t *conn,
                                           uint8_t owner_events,
                                           xcb_window_t grab_window,
                                           uint16_t event_mask,
                                           uint8_t pointer_mode,
                                           uint8_t keyboard_mode,
                                           xcb_window_t confine_to,
                                           xcb_cursor_t cursor,
                                           xcb_timestamp_t time);

xcb_grab_pointer_reply_t *xcb_grab_pointer_reply(xcb_connection_t *conn,
                                                 xcb_grab_pointer_cookie_t cookie,
                                                 xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_timestamp_t time;
} xcb_ungrab_pointer_request_t;

xcb_void_cookie_t xcb_ungrab_pointer(xcb_connection_t *conn,
                                     xcb_timestamp_t time);

xcb_void_cookie_t xcb_ungrab_pointer_checked(xcb_connection_t *conn,
                                             xcb_timestamp_t time);

typedef struct
{
	uint8_t major_opcode;
	uint8_t map_len;
	uint16_t length;
} xcb_set_pointer_mapping_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_set_pointer_mapping_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t status;
	uint16_t sequence;
	uint32_t length;
} xcb_set_pointer_mapping_reply_t;

xcb_set_pointer_mapping_cookie_t xcb_set_pointer_mapping(xcb_connection_t *conn,
                                                         uint8_t map_len,
                                                         const uint8_t *map);

xcb_set_pointer_mapping_reply_t *xcb_set_pointer_mapping_reply(xcb_connection_t *conn,
                                                               xcb_set_pointer_mapping_cookie_t cookie,
                                                               xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_get_pointer_mapping_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_pointer_mapping_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t map_len;
	uint16_t sequence;
	uint32_t length;
} xcb_get_pointer_mapping_reply_t;

xcb_get_pointer_mapping_cookie_t xcb_get_pointer_mapping(xcb_connection_t *conn);

xcb_get_pointer_mapping_reply_t *xcb_get_pointer_mapping_reply(xcb_connection_t *conn,
                                                               xcb_get_pointer_mapping_cookie_t cookie,
                                                               xcb_generic_error_t **error);

uint8_t *xcb_get_pointer_mapping_map(const xcb_get_pointer_mapping_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	int16_t acceleration_numerator;
	int16_t acceleration_denominator;
	int16_t threshold;
	uint8_t do_acceleration;
	uint8_t do_threshold;
} xcb_change_pointer_control_request_t;

xcb_void_cookie_t xcb_change_pointer_control(xcb_connection_t *conn,
                                             int16_t acceleration_numerator,
                                             int16_t acceleration_denominator,
                                             int16_t threshold,
                                             uint8_t do_acceleration,
                                             uint8_t do_threshold);

xcb_void_cookie_t xcb_change_pointer_control_checked(xcb_connection_t *conn,
                                                     int16_t acceleration_numerator,
                                                     int16_t acceleration_denominator,
                                                     int16_t threshold,
                                                     uint8_t do_acceleration,
                                                     uint8_t do_threshold);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_get_pointer_control_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_pointer_control_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	int16_t acceleration_numerator;
	int16_t acceleration_denominator;
	int16_t threshold;
} xcb_get_pointer_control_reply_t;

xcb_get_pointer_control_cookie_t xcb_get_pointer_control(xcb_connection_t *conn);

xcb_get_pointer_control_reply_t *xcb_get_pointer_control_reply(xcb_connection_t *conn,
                                                               xcb_get_pointer_control_cookie_t cookie,
                                                               xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t class;
	uint16_t length;
	xcb_drawable_t drawable;
	uint16_t width;
	uint16_t height;
} xcb_query_best_size_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_query_best_size_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint16_t width;
	uint16_t height;
} xcb_query_best_size_reply_t;

xcb_query_best_size_cookie_t xcb_query_best_size(xcb_connection_t *conn,
                                                 uint8_t class,
                                                 xcb_drawable_t drawable,
                                                 uint16_t width,
                                                 uint16_t height);

xcb_query_best_size_reply_t *xcb_query_best_size_reply(xcb_connection_t *conn,
                                                       xcb_query_best_size_cookie_t cookie,
                                                       xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	uint32_t value_mask;
} xcb_change_keyboard_control_request_t;

xcb_void_cookie_t xcb_change_keyboard_control(xcb_connection_t *conn,
                                              uint32_t value_mask,
                                              const void *values);

xcb_void_cookie_t xcb_change_keyboard_control_checked(xcb_connection_t *conn,
                                                      uint32_t value_mask,
                                                      const void *values);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
} xcb_get_keyboard_control_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_get_keyboard_control_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t global_auto_repeat;
	uint16_t sequence;
	uint32_t length;
	uint32_t led_mask;
	uint8_t key_click_percent;
	uint8_t bell_percent;
	uint16_t bell_pitch;
	uint16_t bell_duration;
	uint16_t pad0;
	uint8_t auto_repeats[32];
} xcb_get_keyboard_control_reply_t;

xcb_get_keyboard_control_cookie_t xcb_get_keyboard_control(xcb_connection_t *conn);

xcb_get_keyboard_control_reply_t *xcb_get_keyboard_control_reply(xcb_connection_t *conn,
                                                                 xcb_get_keyboard_control_cookie_t cookie,
                                                                 xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_gcontext_t gc;
	uint16_t dash_offset;
	uint16_t dashes_len;
} xcb_set_dashes_request_t;

xcb_void_cookie_t xcb_set_dashes(xcb_connection_t *conn,
                                 xcb_gcontext_t gc,
                                 uint16_t dash_offset,
                                 uint16_t dashes_len,
                                 const uint8_t *dashes);

xcb_void_cookie_t xcb_set_dashes_checked(xcb_connection_t *conn,
                                         xcb_gcontext_t gc,
                                         uint16_t dash_offset,
                                         uint16_t dashes_len,
                                         const uint8_t *dashes);

typedef struct
{
	uint8_t major_opcode;
	uint8_t ordering;
	uint16_t length;
	xcb_gcontext_t gc;
	int16_t clip_x_origin;
	int16_t clip_y_origin;
} xcb_set_clip_rectangles_request_t;

xcb_void_cookie_t xcb_set_clip_rectangles(xcb_connection_t *conn,
                                          uint8_t ordering,
                                          xcb_gcontext_t gc,
                                          int16_t clip_x_origin,
                                          int16_t clip_y_origin,
                                          uint16_t rectangles_len,
                                          const xcb_rectangle_t *rectangles);

xcb_void_cookie_t xcb_set_clip_rectangles_checked(xcb_connection_t *conn,
                                                  uint8_t ordering,
                                                  xcb_gcontext_t gc,
                                                  int16_t clip_x_origin,
                                                  int16_t clip_y_origin,
                                                  uint16_t rectangles_len,
                                                  const xcb_rectangle_t *rectangles);

typedef struct
{
	uint8_t major_opcode;
	uint8_t pad0;
	uint16_t length;
	xcb_window_t window;
	uint16_t atoms_len;
	int16_t delta;
} xcb_rotate_properties_request_t;

xcb_void_cookie_t xcb_rotate_properties(xcb_connection_t *conn,
                                        xcb_window_t window,
                                        uint16_t atoms_len,
                                        int16_t delta,
                                        const xcb_atom_t *atoms);

xcb_void_cookie_t xcb_rotate_properties_checked(xcb_connection_t *conn,
                                                xcb_window_t window,
                                                uint16_t atoms_len,
                                                int16_t delta,
                                                const xcb_atom_t *atoms);

#endif
