#include "shm/shm.h"

int setup_shm(struct xsrv *xsrv)
{
	struct extension *extension = register_extension(xsrv, "MIT-SHM");
	if (!extension)
		return 1;
	shmseg_register(xsrv);
	g_req_handlers[extension->major_opcode] = shm_req;
	return 0;
}
