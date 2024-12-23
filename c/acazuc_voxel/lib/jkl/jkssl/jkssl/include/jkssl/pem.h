#ifndef JKSSL_PEM_H
#define JKSSL_PEM_H

#include <stdint.h>
#include <stdio.h>

struct evp_cipher_ctx;
struct pem_write_ctx;
struct b64_write_ctx;
struct pem_read_ctx;
struct asn1_object;
struct evp_cipher;
struct evp_pkey;
struct rsa_ctx;
struct dsa_ctx;
struct dh_ctx;
struct bignum;
struct x509;
struct bio;

int pem_get_key(const struct evp_cipher *evp_cipher, uint8_t *key,
                const uint8_t *salt_iv, const char *password);
int pem_write(struct pem_write_ctx *ctx);
int pem_read(struct pem_read_ctx *ctx);

#define PEM_DECL_READ(name, st) \
struct st *pem_read_bio_##name(struct bio *bio, struct st **v); \
struct st *pem_read_##name(FILE *fp, struct st **v)

#define PEM_DECL_READ_PASS(name, st) \
struct st *pem_read_bio_##name(struct bio *bio, struct st **v, \
                               const char *pass); \
struct st *pem_read_##name(FILE *fp, struct st **v, const char *pass)

#define PEM_DECL_WRITE(name, st) \
int pem_write_bio_##name(struct bio *bio, struct st *v); \
int pem_write_##name(FILE *fp, struct st *v)

#define PEM_DECL_WRITE_PASS(name, st) \
int pem_write_bio_##name(struct bio *bio, struct st *v, \
                         const struct evp_cipher *cipher, \
                         const char *pass); \
int pem_write_##name(FILE *fp, struct st *v, \
                     const struct evp_cipher *cipher, \
                     const char *pass)

#define PEM_DECL_RPWP(name, st) \
PEM_DECL_READ_PASS(name, st); \
PEM_DECL_WRITE_PASS(name, st)

#define PEM_DECL_RPWC(name, st) \
PEM_DECL_READ_PASS(name, st); \
PEM_DECL_WRITE(name, st)

#define PEM_DECL_RCWC(name, st) \
PEM_DECL_READ(name, st); \
PEM_DECL_WRITE(name, st)

PEM_DECL_RPWP(rsa_private_key, rsa_ctx);
PEM_DECL_RPWC(rsa_public_key, rsa_ctx);
PEM_DECL_RPWC(rsa_pubkey, rsa_ctx);
PEM_DECL_RPWC(dsa_params, dsa_ctx);
PEM_DECL_RPWP(dsa_private_key, dsa_ctx);
PEM_DECL_RPWC(dsa_public_key, dsa_ctx);
PEM_DECL_RPWC(dsa_pubkey, dsa_ctx);
PEM_DECL_RPWC(dh_params, dh_ctx);
PEM_DECL_RPWP(private_key, evp_pkey);
PEM_DECL_RPWP(pkcs8_private_key, evp_pkey);
PEM_DECL_RPWC(pubkey, evp_pkey);
PEM_DECL_RCWC(asn1_object, asn1_object);
PEM_DECL_RCWC(x509, x509);

#endif
