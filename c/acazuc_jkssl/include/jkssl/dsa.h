#ifndef JKSSL_DSA_H
#define JKSSL_DSA_H

#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>

struct bignum_gencb;
struct asn1_object;
struct dsa_sig;
struct bignum;
struct dsa;

struct dsa *dsa_new(void);
void dsa_free(struct dsa *dsa);
int dsa_up_ref(struct dsa *dsa);
int dsa_generate_parameters(struct dsa *dsa, size_t bits,
                            struct bignum_gencb *gencb);
int dsa_generate_key(struct dsa *dsa);
int dsa_sign(struct dsa *dsa, const uint8_t *tbs, size_t tbslen,
             uint8_t *sig, size_t *siglen);
struct dsa_sig *dsa_do_sign(const uint8_t *tbs, size_t tbslen,
                            struct dsa *dsa);
int dsa_verify(struct dsa *dsa, const uint8_t *tbs, size_t tbslen,
               const uint8_t *sig, size_t siglen);
int dsa_do_verify(const uint8_t *tbs, size_t tbslen, struct dsa_sig *sig,
                  struct dsa *dsa);
ssize_t dsa_size(struct dsa *dsa);
ssize_t dsa_bits(struct dsa *dsa);
void dsa_get0_pqg(const struct dsa *dsa, const struct bignum **p,
                  const struct bignum **q, const struct bignum **g);
int dsa_set0_pqg(struct dsa *dsa, struct bignum *p, struct bignum *q,
                 struct bignum *g);
void dsa_get0_key(const struct dsa *dsa, const struct bignum **y,
                  const struct bignum **x);
int dsa_set0_key(struct dsa *dsa, struct bignum *y, struct bignum *x);

const struct bignum *dsa_get0_p(const struct dsa *dsa);
const struct bignum *dsa_get0_q(const struct dsa *dsa);
const struct bignum *dsa_get0_g(const struct dsa *dsa);
const struct bignum *dsa_get0_pub_key(const struct dsa *dsa);
const struct bignum *dsa_get0_priv_key(const struct dsa *dsa);

struct dsa_sig *dsa_sig_new(void);
void dsa_sig_free(struct dsa_sig *sig);
void dsa_sig_get0(const struct dsa_sig *sig, const struct bignum **r,
                  const struct bignum **s);
int dsa_sig_set0(struct dsa_sig *sig, struct bignum *r, struct bignum *s);

struct dsa *a2i_dsa_params(const struct asn1_object *object,
                           struct dsa **dsap);
struct asn1_object *i2a_dsa_params(const struct dsa *dsa);
struct dsa *a2i_dsa_private_key(const struct asn1_object *object,
                                struct dsa **dsap);
struct asn1_object *i2a_dsa_private_key(const struct dsa *dsa);
struct dsa *a2i_dsa_public_key(const struct asn1_object *object,
                               struct dsa **dsap);
struct asn1_object *i2a_dsa_public_key(const struct dsa *dsa);
struct dsa *a2i_dsa_pubkey(const struct asn1_object *object,
                           struct dsa **dsap);
struct asn1_object *i2a_dsa_pubkey(const struct dsa *dsa);
struct dsa_sig *a2i_dsa_sig(const struct asn1_object *object,
                            struct dsa_sig **sig);
struct asn1_object *i2a_dsa_sig(const struct dsa_sig *sig);

#endif
