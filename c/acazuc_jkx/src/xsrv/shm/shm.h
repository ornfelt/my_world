#ifndef SHM_SHM_H
#define SHM_SHM_H

#include "xsrv.h"

struct shmseg
{
	struct object object;
	void *ptr;
};

void shmseg_register(struct xsrv *xsrv);

int shm_req(struct xsrv *xsrv, struct client *client,
            struct request *request);

struct shmseg *shmseg_new(struct xsrv *xsrv, struct client *client, uint32_t id,
                          void *ptr);
struct shmseg *shmseg_get(struct xsrv *xsrv, uint32_t id);

int setup_shm(struct xsrv *xsrv);

#endif
