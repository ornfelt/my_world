#ifndef JKSSL_RSA_H
#define JKSSL_RSA_H

#include "refcount.h"

#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>

#define RSA_PKCS1_PADDING      1
#define RSA_PKCS1_OAEP_PADDING 2
#define RSA_NO_PADDING         3

struct bignum_gencb;
struct asn1_object;
struct evp_md;
struct bignum;

struct rsa_ctx
{
	refcount_t refcount;
	struct bignum *p;
	struct bignum *q;
	struct bignum *d;
	struct bignum *phi;
	struct bignum *e;
	struct bignum *n;
	struct bignum *dmq;
	struct bignum *dmp;
	struct bignum *coef;
};

struct rsa_ctx *rsa_new(void);
void rsa_free(struct rsa_ctx *ctx);
int rsa_up_ref(struct rsa_ctx *ctx);
int rsa_generate_key(struct rsa_ctx *ctx, size_t bits, size_t e,
                     struct bignum_gencb *gencb);
int rsa_enc(struct rsa_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t len,
            int padding);
int rsa_dec(struct rsa_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t len,
            int padding);
int rsa_sign(const struct evp_md *md, struct rsa_ctx *ctx,
             const uint8_t *tbs, size_t tbslen,
             uint8_t *sig, size_t *siglen);
int rsa_verify(const struct evp_md *md, struct rsa_ctx *ctx,
               const uint8_t *tbs, size_t tbslen,
               const uint8_t *sig, size_t siglen);
ssize_t rsa_size(struct rsa_ctx *ctx);
ssize_t rsa_bits(struct rsa_ctx *ctx);

int rsa_public_encrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                       struct rsa_ctx *ctx, int padding);
int rsa_private_decrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                        struct rsa_ctx *ctx, int padding);
int rsa_private_encrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                        struct rsa_ctx *ctx, int padding);
int rsa_public_decrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                       struct rsa_ctx *ctx, int padding);

struct rsa_ctx *a2i_rsa_private_key(const struct asn1_object *object,
                                    struct rsa_ctx **ctx);
struct asn1_object *i2a_rsa_private_key(struct rsa_ctx *ctx);
struct rsa_ctx *a2i_rsa_public_key(const struct asn1_object *object,
                                   struct rsa_ctx **ctx);
struct asn1_object *i2a_rsa_public_key(struct rsa_ctx *ctx);
struct rsa_ctx *a2i_rsa_pubkey(const struct asn1_object *object,
                               struct rsa_ctx **ctx);
struct asn1_object *i2a_rsa_pubkey(struct rsa_ctx *ctx);

#endif
