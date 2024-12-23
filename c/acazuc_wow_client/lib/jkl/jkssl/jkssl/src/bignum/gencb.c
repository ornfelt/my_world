#include <jkssl/bignum.h>

#include <stdlib.h>

struct bignum_gencb *bignum_gencb_new(void)
{
	return calloc(sizeof(struct bignum_gencb), 1);
}

void bignum_gencb_free(struct bignum_gencb *gencb)
{
	if (!gencb)
		return;
	free(gencb);
}

void bignum_gencb_set(struct bignum_gencb *gencb,
                      bignum_gencb_cb_t callback,
                      void *arg)
{
	gencb->callback = callback;
	gencb->arg = arg;
}

void *bignum_gencb_get_arg(struct bignum_gencb *gencb)
{
	return gencb->arg;
}

int bignum_gencb_call(struct bignum_gencb *gencb, int a, int b)
{
	if (!gencb->callback)
		return 1;
	return gencb->callback(a, b, gencb);
}
