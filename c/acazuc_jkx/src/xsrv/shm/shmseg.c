#include "shm/shm.h"
#include "xsrv.h"

#include <sys/shm.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static void shmseg_destroy(struct xsrv *xsrv, struct object *object);

static const struct object_def
shmseg_def =
{
	.name = "shmseg",
	.destroy = shmseg_destroy,
};

void
shmseg_register(struct xsrv *xsrv)
{
	xsrv->obj_shmseg = register_object_type(xsrv, &shmseg_def);
}

struct shmseg *
shmseg_new(struct xsrv *xsrv, struct client *client, uint32_t id, void *ptr)
{
	struct shmseg *shmseg;

	shmseg = malloc(sizeof(*shmseg));
	if (!shmseg)
	{
		fprintf(stderr, "%s: malloc: %s\n", xsrv->progname,
		        strerror(errno));
		return NULL;
	}
	object_init(xsrv, client, &shmseg->object, xsrv->obj_shmseg, id);
	shmseg->ptr = ptr;
	return shmseg;
}

static void
shmseg_destroy(struct xsrv *xsrv, struct object *object)
{
	struct shmseg *shmseg = (struct shmseg*)object;

	(void)xsrv;
	shmdt(shmseg->ptr);
}

struct shmseg *
shmseg_get(struct xsrv *xsrv, uint32_t id)
{
	return (struct shmseg*)object_get_typed(xsrv, id, xsrv->obj_shmseg);
}
