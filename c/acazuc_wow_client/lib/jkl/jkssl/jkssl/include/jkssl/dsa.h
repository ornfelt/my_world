#ifndef JKSSL_DSA_H
#define JKSSL_DSA_H

#include "refcount.h"

#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>

struct bignum_gencb;
struct asn1_object;
struct bignum;

struct dsa_ctx
{
	refcount_t refcount;
	struct bignum *p;
	struct bignum *q;
	struct bignum *g;
	struct bignum *y;
	struct bignum *x;
};

struct dsa_sig
{
	struct bignum *r;
	struct bignum *s;
};

struct dsa_ctx *dsa_new(void);
void dsa_free(struct dsa_ctx *ctx);
int dsa_up_ref(struct dsa_ctx *ctx);
int dsa_generate_parameters(struct dsa_ctx *ctx, size_t bits,
                            struct bignum_gencb *gencb);
int dsa_generate_key(struct dsa_ctx *ctx);
int dsa_sign(struct dsa_ctx *ctx, const uint8_t *tbs, size_t tbslen,
             uint8_t *sig, size_t *siglen);
int dsa_verify(struct dsa_ctx *ctx, const uint8_t *tbs, size_t tbslen,
               const uint8_t *sig, size_t siglen);
ssize_t dsa_size(struct dsa_ctx *ctx);
ssize_t dsa_bits(struct dsa_ctx *ctx);

struct dsa_sig *dsa_sig_new(void);
void dsa_sig_free(struct dsa_sig *sig);
void dsa_sig_get0(const struct dsa_sig *sig, const struct bignum **r,
                  const struct bignum **s);
int dsa_sig_set0(struct dsa_sig *sig, struct bignum *r, struct bignum *s);

struct dsa_ctx *a2i_dsa_params(const struct asn1_object *object,
                                      struct dsa_ctx **ctx);
struct asn1_object *i2a_dsa_params(struct dsa_ctx *ctx);
struct dsa_ctx *a2i_dsa_private_key(const struct asn1_object *object,
                                    struct dsa_ctx **ctx);
struct asn1_object *i2a_dsa_private_key(struct dsa_ctx *ctx);
struct dsa_ctx *a2i_dsa_public_key(const struct asn1_object *object,
                                   struct dsa_ctx **ctx);
struct asn1_object *i2a_dsa_public_key(struct dsa_ctx *ctx);
struct dsa_ctx *a2i_dsa_pubkey(const struct asn1_object *object,
                               struct dsa_ctx **ctx);
struct asn1_object *i2a_dsa_pubkey(struct dsa_ctx *ctx);
struct dsa_sig *a2i_dsa_sig(const struct asn1_object *object,
                            struct dsa_sig **sig);
struct asn1_object *i2a_dsa_sig(struct dsa_sig *sig);

#endif
