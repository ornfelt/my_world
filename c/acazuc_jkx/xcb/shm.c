#include <xcb/xcbext.h>
#include <xcb/int.h>
#include <xcb/shm.h>

#include <stdlib.h>

xcb_extension_t xcb_shm_id =
{
	"MIT-SHM",
	0,
};

xcb_shm_query_version_cookie_t xcb_shm_query_version(xcb_connection_t *conn)
{
	EXT_GET(shm);
	uint16_t length = 1;
	REQ_INIT(shm_query_version, extension->major_opcode, length);
	r->minor_opcode = XCB_SHM_QUERY_VERSION;
	xcb_shm_query_version_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_shm_query_version_reply_t *xcb_shm_query_version_reply(xcb_connection_t *conn,
                                                           xcb_shm_query_version_cookie_t cookie,
                                                           xcb_generic_error_t **error)
{
	REPLY_ANSWER(shm_query_version, 0);
	reply->response_type = answer->reply.response_type;
	reply->shared_pixmaps = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->major_version = answer->reply.data16[0];
	reply->minor_version = answer->reply.data16[1];
	reply->uid = answer->reply.data16[2];
	reply->gid = answer->reply.data16[3];
	reply->pixmap_format = answer->reply.data32[2];
	return reply;
}

xcb_void_cookie_t xcb_shm_attach(xcb_connection_t *conn,
                                 xcb_shm_seg_t shmseg,
                                 uint32_t shmid,
                                 uint8_t read_only)
{
	EXT_GET(shm);
	uint16_t length = 4;
	REQ_INIT(shm_attach, extension->major_opcode, length);
	r->minor_opcode = XCB_SHM_ATTACH;
	r->shmseg = shmseg;
	r->shmid = shmid;
	r->read_only = read_only;
	r->pad0[0] = 0;
	r->pad0[1] = 0;
	r->pad0[2] = 0;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_shm_attach_checked(xcb_connection_t *conn,
                                         xcb_shm_seg_t shmseg,
                                         uint32_t shmid,
                                         uint8_t read_only)
{
	return checked_request(conn,
		xcb_shm_attach(conn,
		               shmseg,
		               shmid,
		               read_only));
}

xcb_void_cookie_t xcb_shm_detach(xcb_connection_t *conn,
                                 xcb_shm_seg_t shmseg)
{
	EXT_GET(shm);
	uint16_t length = 2;
	REQ_INIT(shm_detach, extension->major_opcode, length);
	r->minor_opcode = XCB_SHM_DETACH;
	r->shmseg = shmseg;
	return xcb_build_void_cookie(conn);
}

xcb_void_cookie_t xcb_shm_detach_checked(xcb_connection_t *conn,
                                         xcb_shm_seg_t shmseg)
{
	return checked_request(conn,
		xcb_shm_detach(conn,
		               shmseg));
}

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
                                    uint32_t offset)
{
	EXT_GET(shm);
	uint16_t length = 10;
	REQ_INIT(shm_put_image, extension->major_opcode, length);
	r->minor_opcode = XCB_SHM_PUT_IMAGE;
	r->drawable = drawable;
	r->gc = gc;
	r->total_width = total_width;
	r->total_height = total_height;
	r->src_x = src_x;
	r->src_y = src_y;
	r->src_width = src_width;
	r->src_height = src_height;
	r->dst_x = dst_x;
	r->dst_y = dst_y;
	r->depth = depth;
	r->format = format;
	r->send_event = send_event;
	r->pad0 = 0;
	r->shmseg = shmseg;
	r->offset = offset;
	return xcb_build_void_cookie(conn);
}

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
                                            uint32_t offset)
{
	return checked_request(conn,
		xcb_shm_put_image(conn,
		                  drawable,
		                  gc,
		                  total_width,
		                  total_height,
		                  src_x,
		                  src_y,
		                  src_width,
		                  src_height,
		                  dst_x,
		                  dst_y,
		                  depth,
		                  format,
		                  send_event,
		                  shmseg,
		                  offset));
}

xcb_shm_get_image_cookie_t xcb_shm_get_image(xcb_connection_t *conn,
                                             xcb_drawable_t drawable,
                                             int16_t x,
                                             int16_t y,
                                             uint16_t width,
                                             uint16_t height,
                                             uint32_t plane_mask,
                                             uint8_t format,
                                             xcb_shm_seg_t shmseg,
                                             uint32_t offset)
{
	EXT_GET(shm);
	uint16_t length = 8;
	REQ_INIT(shm_get_image, extension->major_opcode, length);
	r->minor_opcode = XCB_SHM_GET_IMAGE;
	r->drawable = drawable;
	r->x = x;
	r->y = y;
	r->width = width;
	r->height = height;
	r->plane_mask = plane_mask;
	r->format = format;
	r->pad0[0] = 0;
	r->pad0[1] = 0;
	r->pad0[2] = 0;
	r->shmseg = shmseg;
	r->offset = offset;
	xcb_shm_get_image_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_shm_get_image_reply_t *xcb_shm_get_image_reply(xcb_connection_t *conn,
                                                   xcb_shm_get_image_cookie_t cookie,
                                                   xcb_generic_error_t **error)
{
	REPLY_ANSWER(shm_get_image, 0);
	reply->response_type = answer->reply.response_type;
	reply->depth = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->visual = answer->reply.data32[0];
	reply->size = answer->reply.data32[1];
	return reply;
}
