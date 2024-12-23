#ifndef JKSSL_DH_H
#define JKSSL_DH_H

#include "refcount.h"

#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>

struct bignum_gencb;
struct asn1_object;
struct bignum;

struct dh_ctx
{
	refcount_t refcount;
	struct bignum *p;
	struct bignum *q;
	struct bignum *g;
	struct bignum *x;
	struct bignum *y;
};

struct dh_ctx *dh_new(void);
void dh_free(struct dh_ctx *ctx);
int dh_up_ref(struct dh_ctx *ctx);
int dh_generate_parameters(struct dh_ctx *ctx, uint32_t bits, size_t generator,
                           struct bignum_gencb *gencb);
int dh_generate_key(struct dh_ctx *ctx);
ssize_t dh_size(struct dh_ctx *ctx);
ssize_t dh_bits(struct dh_ctx *ctx);

struct dh_ctx *a2i_dh_params(const struct asn1_object *object,
                             struct dh_ctx **ctx);
struct asn1_object *i2a_dh_params(struct dh_ctx *ctx);

#endif
