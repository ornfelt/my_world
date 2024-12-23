#ifndef JKSSL_RSA_H
#define JKSSL_RSA_H

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
struct rsa;

struct rsa *rsa_new(void);
void rsa_free(struct rsa *rsa);
int rsa_up_ref(struct rsa *rsa);
int rsa_generate_key(struct rsa *rsa, size_t bits, size_t e,
                     struct bignum_gencb *gencb);
int rsa_enc(struct rsa *rsa, uint8_t *dst, const uint8_t *src, size_t len,
            int padding);
int rsa_dec(struct rsa *rsa, uint8_t *dst, const uint8_t *src, size_t len,
            int padding);
int rsa_sign(const struct evp_md *md, struct rsa *rsa,
             const uint8_t *tbs, size_t tbslen,
             uint8_t *sig, size_t *siglen);
int rsa_verify(const struct evp_md *md, struct rsa *rsa,
               const uint8_t *tbs, size_t tbslen,
               const uint8_t *sig, size_t siglen);
ssize_t rsa_size(struct rsa *rsa);
ssize_t rsa_bits(struct rsa *rsa);
void rsa_get0_key(const struct rsa *rsa, const struct bignum **n,
                  const struct bignum **e, const struct bignum **d);
int rsa_set0_key(struct rsa *rsa, struct bignum *n,
                 struct bignum *e, struct bignum *d);
void rsa_get0_factors(const struct rsa *rsa, const struct bignum **p,
                      const struct bignum **q);
int rsa_set0_factors(struct rsa *rsa, struct bignum *p, struct bignum *q);
void rsa_get0_crt_params(const struct rsa *rsa, const struct bignum **dmp,
                         const struct bignum **dmq, const struct bignum **coef);
int rsa_set0_crt_params(struct rsa *rsa, struct bignum *dmp,
                        struct bignum *dmq, struct bignum *coef);
const struct bignum *rsa_get0_n(const struct rsa *rsa);
const struct bignum *rsa_get0_e(const struct rsa *rsa);
const struct bignum *rsa_get0_d(const struct rsa *rsa);
const struct bignum *rsa_get0_p(const struct rsa *rsa);
const struct bignum *rsa_get0_q(const struct rsa *rsa);
const struct bignum *rsa_get0_dmp1(const struct rsa *rsa);
const struct bignum *rsa_get0_dmq1(const struct rsa *rsa);
const struct bignum *rsa_get0_iqmp(const struct rsa *rsa);
int rsa_check_key(const struct rsa *rsa);

int rsa_public_encrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                       struct rsa *rsa, int padding);
int rsa_private_decrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                        struct rsa *rsa, int padding);
int rsa_private_encrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                        struct rsa *rsa, int padding);
int rsa_public_decrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                       struct rsa *rsa, int padding);

struct rsa *a2i_rsa_private_key(const struct asn1_object *object,
                                struct rsa **rsap);
struct asn1_object *i2a_rsa_private_key(const struct rsa *rsa);
struct rsa *a2i_rsa_public_key(const struct asn1_object *object,
                               struct rsa **rsap);
struct asn1_object *i2a_rsa_public_key(const struct rsa *rsa);
struct rsa *a2i_rsa_pubkey(const struct asn1_object *object,
                           struct rsa **rsap);
struct asn1_object *i2a_rsa_pubkey(const struct rsa *rsa);

#endif
