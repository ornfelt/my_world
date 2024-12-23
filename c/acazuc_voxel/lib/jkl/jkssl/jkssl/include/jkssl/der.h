#ifndef JKSSL_DER_H
#define JKSSL_DER_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct asn1_object;
struct evp_pkey;
struct rsa_ctx;
struct dsa_ctx;
struct dsa_sig;
struct dh_ctx;
struct bignum;
struct x509;
struct bio;

#define DECL_D2I(name, st) \
struct st *d2i_##name(struct st **v, uint8_t **data, size_t len); \
struct st *d2i_##name##_fp(FILE *fp, struct st **v); \
struct st *d2i_##name##_bio(struct bio *bio, struct st **v)

#define DECL_I2D(name, st) \
int i2d_##name(struct st *ctx, uint8_t **dst); \
int i2d_##name##_fp(FILE *fp, struct st *v); \
int i2d_##name##_bio(struct bio *bio, struct st *v)

#define DER_DECL(name, st) \
DECL_D2I(name, st); \
DECL_I2D(name, st)

DER_DECL(rsa_private_key, rsa_ctx);
DER_DECL(rsa_public_key, rsa_ctx);
DER_DECL(rsa_pubkey, rsa_ctx);
DER_DECL(dsa_params, dsa_ctx);
DER_DECL(dsa_private_key, dsa_ctx);
DER_DECL(dsa_public_key, dsa_ctx);
DER_DECL(dsa_pubkey, dsa_ctx);
DER_DECL(dsa_sig, dsa_sig);
DER_DECL(dh_params, dh_ctx);
DER_DECL(private_key, evp_pkey);
DER_DECL(pkcs8_private_key, evp_pkey);
DER_DECL(pubkey, evp_pkey);
DER_DECL(asn1_object, asn1_object);
DER_DECL(x509, x509);
DER_DECL(x509_algor, x509_algor);
DER_DECL(x509_name_entry, x509_name_entry);
DER_DECL(x509_name, x509_name);
DER_DECL(x509_extension, x509_extension);
DER_DECL(x509_pubkey, x509_pubkey);

#endif
