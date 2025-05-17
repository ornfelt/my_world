#include "glx/glx.h"

int
setup_glx(struct xsrv *xsrv)
{
	struct extension *extension;

	extension = register_extension(xsrv, "GLX");
	if (!extension)
		return 1;
	xsrv->req_handlers[extension->major_opcode] = glx_req;
	return 0;
}
