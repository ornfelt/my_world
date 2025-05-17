#include "render/render.h"

int
setup_render(struct xsrv *xsrv)
{
	struct extension *extension;

	extension = register_extension(xsrv, "RENDER");
	if (!extension)
		return 1;
	picture_register(xsrv);
	glyphset_register(xsrv);
	xsrv->req_handlers[extension->major_opcode] = render_req;
	return 0;
}

struct pictfmt *
render_get_format(struct xsrv *xsrv, uint32_t id)
{
	return NULL;
}
