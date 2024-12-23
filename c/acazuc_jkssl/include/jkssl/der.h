#ifndef JKSSL_DER_H
#define JKSSL_DER_H

#include <jkssl/pem.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct asn1_object;
struct evp_cipher;
struct evp_pkey;
struct ec_group;
struct dsa_sig;
struct bignum;
struct ec_key;
struct x509;
struct bio;
struct dsa;
struct rsa;
struct dh;

#define DECL_D2I(name, st) \
struct st *d2i_##name(struct st **v, uint8_t **data, size_t len); \
struct st *d2i_##name##_fp(FILE *fp, struct st **v); \
struct st *d2i_##name##_bio(struct bio *bio, struct st **v)

#define DECL_D2I_PASS(name, st) \
struct st *d2i_##name(struct st **v, \
                      uint8_t **data, \
                      size_t len, \
                      pem_password_cb *cb, \
                      void *userdata); \
struct st *d2i_##name##_fp(FILE *fp, \
                           struct st **v, \
                           pem_password_cb *cb, \
                           void *userdata); \
struct st *d2i_##name##_bio(struct bio *bio, \
                            struct st **v, \
                            pem_password_cb *cb, \
                            void *userdata)

#define DECL_I2D(name, st) \
int i2d_##name(const struct st *v, uint8_t **dst); \
int i2d_##name##_fp(FILE *fp, const struct st *v); \
int i2d_##name##_bio(struct bio *bio, const struct st *v)

#define DECL_I2D_PASS(name, st) \
int i2d_##name(const struct st *v, \
               uint8_t **dst, \
               const struct evp_cipher *cipher, \
               pem_password_cb *cb, \
               void *userdata); \
int i2d_##name##_fp(FILE *fp, \
                    const struct st *v, \
                    const struct evp_cipher *cipher, \
                    pem_password_cb *cb, \
                    void *userdata); \
int i2d_##name##_bio(struct bio *bio, \
                     const struct st *v, \
                     const struct evp_cipher *cipher, \
                     pem_password_cb *cb, \
                     void *userdata)

#define DER_DECL_RCWC(name, st) \
DECL_D2I(name, st); \
DECL_I2D(name, st)

#define DER_DECL_RPWP(name, st) \
DECL_D2I_PASS(name, st); \
DECL_I2D_PASS(name, st)

DER_DECL_RCWC(rsa_private_key, rsa);
DER_DECL_RCWC(rsa_public_key, rsa);
DER_DECL_RCWC(rsa_pubkey, rsa);
DER_DECL_RCWC(dsa_params, dsa);
DER_DECL_RCWC(dsa_private_key, dsa);
DER_DECL_RCWC(dsa_public_key, dsa);
DER_DECL_RCWC(dsa_pubkey, dsa);
DER_DECL_RCWC(dsa_sig, dsa_sig);
DER_DECL_RCWC(dh_params, dh);
DER_DECL_RCWC(private_key, evp_pkey);
DER_DECL_RCWC(private_key_traditional, evp_pkey);
DER_DECL_RPWP(pkcs8_private_key, evp_pkey);
DER_DECL_RCWC(pkcs8_private_key_info, evp_pkey);
DER_DECL_RCWC(pubkey, evp_pkey);
DER_DECL_RCWC(asn1_object, asn1_object);
DER_DECL_RCWC(x509, x509);
DER_DECL_RCWC(x509_algor, x509_algor);
DER_DECL_RCWC(x509_name_entry, x509_name_entry);
DER_DECL_RCWC(x509_name, x509_name);
DER_DECL_RCWC(x509_extension, x509_extension);
DER_DECL_RCWC(ecpk_parameters, ec_group);
DER_DECL_RCWC(ec_private_key, ec_key);
DER_DECL_RCWC(ecdsa_sig, ecdsa_sig);
DER_DECL_RCWC(ec_pubkey, ec_key);

#endif
