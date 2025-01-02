#ifndef JKSSL_DH_H
#define JKSSL_DH_H

#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>

#define DH_CHECK_P_NOT_PRIME         (1 << 0)
#define DH_NOT_SUITABLE_GENERATOR    (1 << 1)
#define DH_MODULUS_TOO_SMALL         (1 << 2)
#define DH_MODULUS_TOO_LARGE         (1 << 3)
#define DH_CHECK_P_NOT_SAFE_PRIME    (1 << 4)
#define DH_UNABLE_TO_CHECK_GENERATOR (1 << 5)
#define DH_CHECK_Q_NOT_PRIME         (1 << 6)
#define DH_CHECK_INVALID_Q_VALUE     (1 << 7)
#define DH_CHECK_INVALID_J_VALUE     (1 << 8)

struct bignum_gencb;
struct asn1_object;
struct bignum;
struct dh;

struct dh *dh_new(void);
void dh_free(struct dh *dh);
int dh_up_ref(struct dh *dh);
int dh_generate_parameters(struct dh *dh, uint32_t bits, size_t generator,
                           struct bignum_gencb *gencb);
int dh_generate_key(struct dh *dh);
int dh_compute_key(uint8_t *key, const struct bignum *pub, struct dh *dh);
int dh_check(struct dh *dh, int *codes);
int dh_check_params(struct dh *dh, int *codes);
ssize_t dh_size(struct dh *dh);
ssize_t dh_bits(struct dh *dh);
void dh_get0_pqg(const struct dh *dh, const struct bignum **p,
                 const struct bignum **q, const struct bignum **g);
int dh_set0_pqg(struct dh *dh, struct bignum *p,
                struct bignum *q, struct bignum *g);
void dh_get0_key(const struct dh *dh, const struct bignum **y,
                 struct bignum **x);
int dh_set0_key(struct dh *dh, struct bignum *y, struct bignum *x);
const struct bignum *dh_get0_p(const struct dh *dh);
const struct bignum *dh_get0_q(const struct dh *dh);
const struct bignum *dh_get0_g(const struct dh *dh);
const struct bignum *dh_get0_priv_key(const struct dh *dh);
const struct bignum *dh_get0_pub_key(const struct dh *dh);

struct dh *a2i_dh_params(const struct asn1_object *object,
                             struct dh **dhp);
struct asn1_object *i2a_dh_params(const struct dh *dh);

#endif
