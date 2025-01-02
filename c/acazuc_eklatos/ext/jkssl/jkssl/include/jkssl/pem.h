#ifndef JKSSL_PEM_H
#define JKSSL_PEM_H

#include <stdint.h>
#include <stdio.h>

struct asn1_object;
struct evp_cipher;
struct evp_pkey;
struct ec_group;
struct bignum;
struct ec_key;
struct x509;
struct bio;
struct dsa;
struct rsa;
struct dh;

typedef int pem_password_cb(char *buf, int size, int rw, void *userdata);

#define PEM_DECL_READ(name, st) \
struct st *pem_read_bio_##name(struct bio *bio, struct st **v); \
struct st *pem_read_##name(FILE *fp, struct st **v)

#define PEM_DECL_READ_PASS(name, st) \
struct st *pem_read_bio_##name(struct bio *bio, \
                               struct st **v, \
                               pem_password_cb *cb, \
                               void *userdata); \
struct st *pem_read_##name(FILE *fp, \
                           struct st **v, \
                           pem_password_cb *cb, \
                           void *userdata)

#define PEM_DECL_WRITE(name, st) \
int pem_write_bio_##name(struct bio *bio, const struct st *v); \
int pem_write_##name(FILE *fp, const struct st *v)

#define PEM_DECL_WRITE_PASS(name, st) \
int pem_write_bio_##name(struct bio *bio, \
                         const struct st *v, \
                         const struct evp_cipher *cipher, \
                         pem_password_cb *cb, \
                         void *userdata); \
int pem_write_##name(FILE *fp, \
                     const struct st *v, \
                     const struct evp_cipher *cipher, \
                     pem_password_cb *cb, \
                     void *userdata)

#define PEM_DECL_RPWP(name, st) \
PEM_DECL_READ_PASS(name, st); \
PEM_DECL_WRITE_PASS(name, st)

#define PEM_DECL_RPWC(name, st) \
PEM_DECL_READ_PASS(name, st); \
PEM_DECL_WRITE(name, st)

#define PEM_DECL_RCWC(name, st) \
PEM_DECL_READ(name, st); \
PEM_DECL_WRITE(name, st)

PEM_DECL_RPWP(rsa_private_key, rsa);
PEM_DECL_RPWC(rsa_public_key, rsa);
PEM_DECL_RPWC(rsa_pubkey, rsa);
PEM_DECL_RPWC(dsa_params, dsa);
PEM_DECL_RPWP(dsa_private_key, dsa);
PEM_DECL_RPWC(dsa_public_key, dsa);
PEM_DECL_RPWC(dsa_pubkey, dsa);
PEM_DECL_RPWC(dh_params, dh);
PEM_DECL_RPWP(private_key, evp_pkey);
PEM_DECL_RPWP(private_key_traditional, evp_pkey);
PEM_DECL_RPWP(pkcs8_private_key, evp_pkey);
PEM_DECL_RCWC(pkcs8_private_key_info, evp_pkey);
PEM_DECL_RPWC(pubkey, evp_pkey);
PEM_DECL_RCWC(asn1_object, asn1_object);
PEM_DECL_RCWC(x509, x509);
PEM_DECL_RCWC(ecpk_parameters, ec_group);
PEM_DECL_RPWP(ec_private_key, ec_key);
PEM_DECL_RPWC(ec_pubkey, ec_key);

#endif
