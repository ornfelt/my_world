#ifndef XCB_RENDER_H
#define XCB_RENDER_H

#include "xcb.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XCB_RENDER_MAJOR_VERSION 0
#define XCB_RENDER_MINOR_VERSION 11

#define XCB_RENDER_QUERY_VERSION      0
#define XCB_RENDER_QUERY_PICT_FORMATS 1
#define XCB_RENDER_CREATE_PICTURE     4
#define XCB_RENDER_COMPOSITE          8
#define XCB_RENDER_FILL_RECTANGLES    26

typedef uint32_t xcb_render_picture_t;
typedef uint32_t xcb_render_pictformat_t;
typedef uint32_t xcb_render_fixed_t;
typedef uint32_t xcb_render_glyphset_t;
typedef uint32_t xcb_render_glyph_t;

typedef enum
{
	XCB_RENDER_PICT_TYPE_INDEXED = 0,
	XCB_RENDER_PICT_TYPE_DIRECT  = 1,
} xcb_render_pict_type_t;

typedef enum
{
	XCB_RENDER_PICT_OP_CLEAR                 = 0,
	XCB_RENDER_PICT_OP_SRC                   = 1,
	XCB_RENDER_PICT_OP_DST                   = 2,
	XCB_RENDER_PICT_OP_OVER                  = 3,
	XCB_RENDER_PICT_OP_OVER_RESERVE          = 4,
	XCB_RENDER_PICT_OP_IN                    = 5,
	XCB_RENDER_PICT_OP_IN_REVERSE            = 6,
	XCB_RENDER_PICT_OP_OUT                   = 7,
	XCB_RENDER_PICT_OP_OUT_REVERSE           = 8,
	XCB_RENDER_PICT_OP_ATOP                  = 9,
	XCB_RENDER_PICT_OP_ATOP_REVERSE          = 10,
	XCB_RENDER_PICT_OP_XOR                   = 11,
	XCB_RENDER_PICT_OP_ADD                   = 12,
	XCB_RENDER_PICT_OP_SATURATE              = 13,
	XCB_RENDER_PICT_OP_DISJOINT_CLEAR        = 16,
	XCB_RENDER_PICT_OP_DISJOINT_SRC          = 17,
	XCB_RENDER_PICT_OP_DISJOINT_DST          = 18,
	XCB_RENDER_PICT_OP_DISJOINT_OVER         = 19,
	XCB_RENDER_PICT_OP_DISJOINT_OVER_REVERSE = 20,
	XCB_RENDER_PICT_OP_DISJOINT_IN           = 21,
	XCB_RENDER_PICT_OP_DISJOINT_IN_REVERSE   = 22,
	XCB_RENDER_PICT_OP_DISJOINT_OUT          = 23,
	XCB_RENDER_PICT_OP_DISJOINT_OUT_REVERSE  = 24,
	XCB_RENDER_PICT_OP_DISJOINT_ATOP         = 25,
	XCB_RENDER_PICT_OP_DISJOINT_ATOP_REVERSE = 26,
	XCB_RENDER_PICT_OP_DISJOINT_XOR          = 27,
	XCB_RENDER_PICT_OP_CONJOINT_CLEAR        = 32,
	XCB_RENDER_PICT_OP_CONJOINT_SRC          = 33,
	XCB_RENDER_PICT_OP_CONJOINT_DST          = 34,
	XCB_RENDER_PICT_OP_CONJOINT_OVER         = 35,
	XCB_RENDER_PICT_OP_CONJOINT_OVER_REVERSE = 36,
	XCB_RENDER_PICT_OP_CONJOINT_IN           = 37,
	XCB_RENDER_PICT_OP_CONJOINT_IN_REVERSE   = 38,
	XCB_RENDER_PICT_OP_CONJOINT_OUT          = 39,
	XCB_RENDER_PICT_OP_CONJOINT_OUT_REVERSE  = 40,
	XCB_RENDER_PICT_OP_CONJOINT_ATOP         = 41,
	XCB_RENDER_PICT_OP_CONJOINT_ATOP_REVERSE = 42,
	XCB_RENDER_PICT_OP_CONJOINT_XOR          = 43,
	XCB_RENDER_PICT_OP_MULTIPLY              = 48,
	XCB_RENDER_PICT_OP_SCREEN                = 49,
	XCB_RENDER_PICT_OP_OVERLAY               = 50,
	XCB_RENDER_PICT_OP_DARKEN                = 51,
	XCB_RENDER_PICT_OP_LIGHTEN               = 52,
	XCB_RENDER_PICT_OP_COLOR_DODGE           = 53,
	XCB_RENDER_PICT_OP_COLOR_BURN            = 54,
	XCB_RENDER_PICT_OP_HARD_LIGHT            = 55,
	XCB_RENDER_PICT_OP_SOFT_LIGHT            = 56,
	XCB_RENDER_PICT_OP_DIFFERENCE            = 57,
	XCB_RENDER_PICT_OP_EXCLUSION             = 58,
	XCB_RENDER_PICT_OP_HSL_HUE               = 59,
	XCB_RENDER_PICT_OP_HSL_SATURATION        = 60,
	XCB_RENDER_PICT_OP_HSL_COLOR             = 61,
	XCB_RENDER_PICT_OP_HSL_LUMINOSITY        = 62,
} xcb_render_pict_op_t;

typedef enum
{
	XCB_RENDER_SUB_PIXEL_UNKNOWN        = 0,
	XCB_RENDER_SUB_PIXEL_HORIZNOTAL_RGB = 1,
	XCB_RENDER_SUB_PIXEL_HORIZONTAL_BGR = 2,
	XCB_RENDER_SUB_PIXEL_VERTICAL_RGB   = 3,
	XCB_RENDER_SUB_PIXEL_VERTICAL_BGR   = 4,
	XCB_RENDER_SUB_PIXEL_NONE           = 5,
} xcb_render_sub_pixel_t;

typedef struct
{
	uint16_t reg;
	uint16_t green;
	uint16_t blue;
	uint16_t alpha;
} xcb_render_color_t;

typedef struct
{
	uint16_t shift;
	uint16_t mask;
} xcb_render_channel_mask_t;

typedef struct
{
	uint16_t red_shift;
	uint16_t red_mask;
	uint16_t green_shift;
	uint16_t green_mask;
	uint16_t blue_shift;
	uint16_t blue_mask;
	uint16_t alpha_shift;
	uint16_t alpha_mask;
} xcb_render_directformat_t;

typedef struct
{
	uint32_t pixel;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
	uint16_t alpha;
} xcb_render_indexvalue_t;

typedef struct
{
	xcb_render_pictformat_t id;
	uint8_t type;
	uint8_t depth;
	uint8_t pad0[2];
	xcb_render_directformat_t direct;
	xcb_colormap_t colormap;
} xcb_render_pictforminfo_t;

typedef struct
{
	xcb_visualid_t visual;
	xcb_render_pictformat_t format;
} xcb_render_pictvisual_t;

typedef struct
{
	uint8_t depth;
} xcb_render_pictdepth_t;

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	uint32_t client_major_version;
	uint32_t client_minor_version;
} xcb_render_query_version_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_render_query_version_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint32_t major_version;
	uint32_t minor_version;
} xcb_render_query_version_reply_t;

xcb_render_query_version_cookie_t xcb_render_query_version(xcb_connection_t *conn,
                                                           uint32_t client_major_version,
                                                           uint32_t client_minor_version);
xcb_render_query_version_reply_t *xcb_render_query_version_reply(xcb_connection_t *conn,
                                                                 xcb_render_query_version_cookie_t cookie,
                                                                 xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
} xcb_render_query_pict_formats_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_render_query_pict_formats_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t pad0;
	uint16_t sequence;
	uint32_t length;
	uint32_t num_formats;
	uint32_t num_screens;
	uint32_t num_depths;
	uint32_t num_visuals;
	uint32_t num_subpixels;
	uint32_t pad1;
} xcb_render_query_pict_formats_reply_t;

typedef struct
{
	xcb_render_query_pict_formats_reply_t reply;
	xcb_render_pictforminfo_t *formats;
} xcb_priv_render_query_pict_formats_reply_t;

xcb_render_query_pict_formats_cookie_t xcb_render_query_pict_formats(xcb_connection_t *conn);
xcb_render_query_pict_formats_reply_t *xcb_render_query_pict_formats_reply(xcb_connection_t *conn,
                                                                           xcb_render_query_pict_formats_cookie_t cookie,
                                                                           xcb_generic_error_t **error);
xcb_render_pictforminfo_t *xcb_render_query_pict_formats_formats(xcb_render_query_pict_formats_reply_t *reply);
int xcb_render_query_pict_formats_formats_length(xcb_render_query_pict_formats_reply_t *reply);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_render_picture_t pid;
	xcb_drawable_t drawable;
	xcb_render_pictformat_t format;
	uint32_t value_mask;
} xcb_render_create_picture_request_t;

xcb_void_cookie_t xcb_render_create_picture(xcb_connection_t *conn,
                                            xcb_render_picture_t pid,
                                            xcb_drawable_t drawable,
                                            xcb_render_pictformat_t format,
                                            uint32_t value_mask,
                                            const uint32_t *values);
xcb_void_cookie_t xcb_render_create_picture_checked(xcb_connection_t *conn,
                                                    xcb_render_picture_t pid,
                                                    xcb_drawable_t drawable,
                                                    xcb_render_pictformat_t format,
                                                    uint32_t value_mask,
                                                    const uint32_t *values);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	uint8_t op;
	uint8_t pad0[3];
	xcb_render_picture_t dst;
	xcb_render_color_t color;
} xcb_render_fill_rectangles_request_t;

xcb_void_cookie_t xcb_render_fill_rectangles(xcb_connection_t *conn,
                                             uint8_t op,
                                             xcb_render_picture_t dst,
                                             xcb_render_color_t color,
                                             uint32_t rectangles_len,
                                             const xcb_rectangle_t *rectangles);
xcb_void_cookie_t xcb_render_fill_rectangles_checked(xcb_connection_t *conn,
                                                     uint8_t op,
                                                     xcb_render_picture_t dst,
                                                     xcb_render_color_t color,
                                                     uint32_t rectangles_len,
                                                     const xcb_rectangle_t *rectangles);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	uint8_t op;
	uint8_t pad0[3];
	xcb_render_picture_t src;
	xcb_render_picture_t mask;
	xcb_render_picture_t dst;
	int16_t src_x;
	int16_t src_y;
	int16_t mask_x;
	int16_t mask_y;
	int16_t dst_x;
	int16_t dst_y;
	uint16_t width;
	uint16_t height;
} xcb_render_composite_request_t;

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
                                       uint16_t height);
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
                                               uint16_t height);

extern xcb_extension_t xcb_render_id;

#ifdef __cplusplus
}
#endif

#endif
