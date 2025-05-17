#ifndef GLX_GLX_H
#define GLX_GLX_H

#include "xsrv.h"

int setup_glx(struct xsrv *xsrv);
int glx_req(struct xsrv *xsrv,
            struct client *client,
            struct request *request);

#endif
