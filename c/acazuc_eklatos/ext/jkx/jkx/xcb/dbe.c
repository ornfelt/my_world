#include <xcb/xcbext.h>
#include <xcb/int.h>
#include <xcb/dbe.h>

#include <stdlib.h>

xcb_extension_t xcb_dbe_id =
{
	"DOUBLE-BUFFER",
	0,
};

xcb_dbe_query_version_cookie_t xcb_dbe_query_version(xcb_connection_t *conn,
                                                     uint16_t major_version,
                                                     uint16_t minor_version)
{
	EXT_GET(dbe);
	uint16_t length = 2;
	REQ_INIT(dbe_query_version, extension->major_opcode, length);
	r->minor_opcode = XCB_DBE_QUERY_VERSION;
	r->major_version = major_version;
	r->minor_version = minor_version;
	xcb_dbe_query_version_cookie_t cookie;
	cookie.sequence = ++conn->sequence;
	return cookie;
}

xcb_dbe_query_version_reply_t *xcb_dbe_query_version_reply(xcb_connection_t *conn,
                                                           xcb_dbe_query_version_cookie_t cookie,
                                                           xcb_generic_error_t **error)
{
	REPLY_ANSWER(dbe_query_version, 0);
	reply->response_type = answer->reply.response_type;
	reply->pad0 = answer->reply.pad0;
	reply->sequence = answer->reply.sequence;
	reply->length = answer->reply.length;
	reply->major_version = answer->reply.data16[0];
	reply->minor_version = answer->reply.data16[1];
	return reply;
}
