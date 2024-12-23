#ifndef JKSSL_EVP_PKEY_H
#define JKSSL_EVP_PKEY_H

#include <stddef.h>
#include <stdint.h>

struct evp_pkey;
struct rsa_ctx;
struct dsa_ctx;
struct dh_ctx;
struct evp_md;

struct evp_pkey *evp_pkey_new(void);
void evp_pkey_free(struct evp_pkey *pkey);
int evp_pkey_up_ref(struct evp_pkey *pkey);

int evp_pkey_assign_rsa(struct evp_pkey *pkey, struct rsa_ctx *rsa);
int evp_pkey_set1_rsa(struct evp_pkey *pkey, struct rsa_ctx *rsa);
int evp_pkey_assign_dsa(struct evp_pkey *pkey, struct dsa_ctx *dsa);
int evp_pkey_set1_dsa(struct evp_pkey *pkey, struct dsa_ctx *dsa);
int evp_pkey_assign_dh(struct evp_pkey *pkey, struct dh_ctx *dh);
int evp_pkey_set1_dh(struct evp_pkey *pkey, struct dh_ctx *dh);

struct rsa_ctx *evp_pkey_get0_rsa(struct evp_pkey *pkey);
struct rsa_ctx *evp_pkey_get1_rsa(struct evp_pkey *pkey);
struct dsa_ctx *evp_pkey_get0_dsa(struct evp_pkey *pkey);
struct dsa_ctx *evp_pkey_get1_dsa(struct evp_pkey *pkey);
struct dh_ctx *evp_pkey_get0_dh(struct evp_pkey *pkey);
struct dh_ctx *evp_pkey_get1_dh(struct evp_pkey *pkey);

struct evp_pkey_ctx *evp_pkey_ctx_new(struct evp_pkey *pkey);
void evp_pkey_ctx_free(struct evp_pkey_ctx *ctx);

int evp_pkey_ctx_set_rsa_padding(struct evp_pkey_ctx *ctx, int padding);
int evp_pkey_ctx_set_signature_md(struct evp_pkey_ctx *ctx,
                                  const struct evp_md *md);

int evp_pkey_sign_init(struct evp_pkey_ctx *ctx);
int evp_pkey_sign(struct evp_pkey_ctx *ctx, uint8_t *sig, size_t *siglen,
                  const uint8_t *tbs, size_t tbslen);
int evp_pkey_verify_init(struct evp_pkey_ctx *ctx);
int evp_pkey_verify(struct evp_pkey_ctx *ctx, const uint8_t *sig, size_t siglen,
                    const uint8_t *tbs, size_t tbslen);
int evp_pkey_encrypt_init(struct evp_pkey_ctx *ctx);
int evp_pkey_encrypt(struct evp_pkey_ctx *ctx, uint8_t *out, size_t *outlen,
                     const uint8_t *in, size_t inlen);
int evp_pkey_decrypt_init(struct evp_pkey_ctx *ctx);
int evp_pkey_decrypt(struct evp_pkey_ctx *ctx, uint8_t *out, size_t *outlen,
                     const uint8_t *in, size_t inlen);
int evp_pkey_derive_init(struct evp_pkey_ctx *ctx);
int evp_pkey_derive(struct evp_pkey_ctx *ctx, uint8_t *key, size_t *keylen);

#endif
