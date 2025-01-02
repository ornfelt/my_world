#ifndef XCB_SHM_H
#define XCB_SHM_H

#include <xcb/xcb.h>

#define XCB_SHM_MAJOR_VERSION 1
#define XCB_SHM_MINOR_VERSION 2

#define XCB_SHM_QUERY_VERSION 0
#define XCB_SHM_ATTACH        1
#define XCB_SHM_DETACH        2
#define XCB_SHM_PUT_IMAGE     3
#define XCB_SHM_GET_IMAGE     4

typedef uint32_t xcb_shm_seg_t;

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
} xcb_shm_query_version_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_shm_query_version_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t shared_pixmaps;
	uint16_t sequence;
	uint32_t length;
	uint16_t major_version;
	uint16_t minor_version;
	uint16_t uid;
	uint16_t gid;
	uint8_t pixmap_format;
} xcb_shm_query_version_reply_t;

xcb_shm_query_version_cookie_t xcb_shm_query_version(xcb_connection_t *conn);
xcb_shm_query_version_reply_t *xcb_shm_query_version_reply(xcb_connection_t *conn,
                                                           xcb_shm_query_version_cookie_t cookie,
                                                           xcb_generic_error_t **error);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_shm_seg_t shmseg;
	uint32_t shmid;
	uint8_t read_only;
	uint8_t pad0[3];
} xcb_shm_attach_request_t;

xcb_void_cookie_t xcb_shm_attach(xcb_connection_t *conn,
                                 xcb_shm_seg_t shmseg,
                                 uint32_t shmid,
                                 uint8_t read_only);

xcb_void_cookie_t xcb_shm_attach_checked(xcb_connection_t *conn,
                                         xcb_shm_seg_t shmseg,
                                         uint32_t shmid,
                                         uint8_t read_only);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_shm_seg_t shmseg;
} xcb_shm_detach_request_t;

xcb_void_cookie_t xcb_shm_detach(xcb_connection_t *conn,
                                 xcb_shm_seg_t shmseg);

xcb_void_cookie_t xcb_shm_detach_checked(xcb_connection_t *conn,
                                         xcb_shm_seg_t shmseg);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_drawable_t drawable;
	xcb_gcontext_t gc;
	uint16_t total_width;
	uint16_t total_height;
	uint16_t src_x;
	uint16_t src_y;
	uint16_t src_width;
	uint16_t src_height;
	int16_t dst_x;
	int16_t dst_y;
	uint8_t depth;
	uint8_t format;
	uint8_t send_event;
	uint8_t pad0;
	xcb_shm_seg_t shmseg;
	uint32_t offset;
} xcb_shm_put_image_request_t;

xcb_void_cookie_t xcb_shm_put_image(xcb_connection_t *conn,
                                    xcb_drawable_t drawable,
                                    xcb_gcontext_t gc,
                                    uint16_t total_width,
                                    uint16_t total_height,
                                    uint16_t src_x,
                                    uint16_t src_y,
                                    uint16_t src_width,
                                    uint16_t src_height,
                                    int16_t dst_x,
                                    int16_t dst_y,
                                    uint8_t depth,
                                    uint8_t format,
                                    uint8_t send_event,
                                    xcb_shm_seg_t shmseg,
                                    uint32_t offset);

xcb_void_cookie_t xcb_shm_put_image_checked(xcb_connection_t *conn,
                                            xcb_drawable_t drawable,
                                            xcb_gcontext_t gc,
                                            uint16_t total_width,
                                            uint16_t total_height,
                                            uint16_t src_x,
                                            uint16_t src_y,
                                            uint16_t src_width,
                                            uint16_t src_height,
                                            int16_t dst_x,
                                            int16_t dst_y,
                                            uint8_t depth,
                                            uint8_t format,
                                            uint8_t send_event,
                                            xcb_shm_seg_t shmseg,
                                            uint32_t offset);

typedef struct
{
	uint8_t major_opcode;
	uint8_t minor_opcode;
	uint16_t length;
	xcb_drawable_t drawable;
	int16_t x;
	int16_t y;
	uint16_t width;
	uint16_t height;
	uint32_t plane_mask;
	uint8_t format;
	uint8_t pad0[3];
	xcb_shm_seg_t shmseg;
	uint32_t offset;
} xcb_shm_get_image_request_t;

typedef struct
{
	uint32_t sequence;
} xcb_shm_get_image_cookie_t;

typedef struct
{
	uint8_t response_type;
	uint8_t depth;
	uint16_t sequence;
	uint32_t length;
	xcb_visualid_t visual;
	uint32_t size;
} xcb_shm_get_image_reply_t;

xcb_shm_get_image_cookie_t xcb_shm_get_image(xcb_connection_t *conn,
                                             xcb_drawable_t drawable,
                                             int16_t x,
                                             int16_t y,
                                             uint16_t width,
                                             uint16_t height,
                                             uint32_t plane_mask,
                                             uint8_t format,
                                             xcb_shm_seg_t shmseg,
                                             uint32_t offset);

xcb_shm_get_image_reply_t *xcb_shm_get_image_reply(xcb_connection_t *conn,
                                                   xcb_shm_get_image_cookie_t cookie,
                                                   xcb_generic_error_t **error);

extern xcb_extension_t xcb_shm_id;

#endif
