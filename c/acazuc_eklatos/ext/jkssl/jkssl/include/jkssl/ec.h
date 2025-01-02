#ifndef JKSSL_EC_H
#define JKSSL_EC_H

#include <stdint.h>
#include <stddef.h>

#define JKSSL_EC_EXPLICIT_CURVE 0
#define JKSSL_EC_NAMED_CURVE    1

enum point_conversion_form
{
	POINT_CONVERSION_UNCOMPRESSED,
	POINT_CONVERSION_COMPRESSED,
	POINT_CONVERSION_HYBRID,
};

struct bignum_ctx;
struct ecdsa_dig;
struct ec_group;
struct ec_point;
struct asn1_oid;
struct bignum;
struct ec_key;

struct ec_point *ec_point_new(const struct ec_group *group);
void ec_point_free(struct ec_point *point);
int ec_point_copy(struct ec_point *dst, const struct ec_point *src);
struct ec_point *ec_point_dup(const struct ec_point *point);

int ec_point_add(const struct ec_group *group, struct ec_point *r,
                 const struct ec_point *a, const struct ec_point *b,
                 struct bignum_ctx *ctx);
int ec_point_dbl(const struct ec_group *group, struct ec_point *r,
                 const struct ec_point *a, struct bignum_ctx *ctx);
int ec_point_invert(const struct ec_group *group, struct ec_point *r,
                    const struct ec_point *a, struct bignum_ctx *ctx);
int ec_point_is_at_infinity(const struct ec_group *group,
                            const struct ec_point *p);
int ec_point_set_to_infinity(const struct ec_group *group, struct ec_point *p);
int ec_point_is_on_curve(const struct ec_group *group, const struct ec_point *p,
                         struct bignum_ctx *ctx);
int ec_point_cmp(const struct ec_group *group, const struct ec_point *a,
                 const struct ec_point *b, struct bignum_ctx *ctx);
int ec_point_mul(const struct ec_group *group, struct ec_point *r,
                 const struct bignum *n, const struct ec_point *p,
                 const struct bignum *m, struct bignum_ctx *ctx);
int ec_point_set_affine_coordinates(const struct ec_group *group,
                                    struct ec_point *point,
                                    const struct bignum *x,
                                    const struct bignum *y,
                                    struct bignum_ctx *bn_ctx);
int ec_point_get_affine_coordinates(const struct ec_group *group,
                                    const struct ec_point *point,
                                    struct bignum *x,
                                    struct bignum *y,
                                    struct bignum_ctx *bn_ctx);
size_t ec_point_point2oct(const struct ec_group *group,
                          const struct ec_point *point,
                          enum point_conversion_form form,
                          uint8_t *buf, size_t size,
                          struct bignum_ctx *bn_ctx);
size_t ec_point_point2buf(const struct ec_group *group,
                          const struct ec_point *point,
                          enum point_conversion_form form,
                          uint8_t **buf,
                          struct bignum_ctx *bn_ctx);
int ec_point_oct2point(const struct ec_group *group,
                       struct ec_point *point,
                       const uint8_t *buf, size_t size,
                       struct bignum_ctx *bn_ctx);

struct ec_group *ec_group_new(void);
struct ec_group *ec_group_new_curve_gfp(const struct bignum *p,
                                        const struct bignum *a,
                                        const struct bignum *b,
                                        struct bignum_ctx *bn_ctx);
void ec_group_free(struct ec_group *group);
int ec_group_copy(struct ec_group *dst, const struct ec_group *src);
struct ec_group *ec_group_dup(const struct ec_group *group);
int ec_group_set_curve(struct ec_group *group, const struct bignum *p,
                       const struct bignum *a, const struct bignum *b,
                       struct bignum_ctx *bn_ctx);
int ec_group_get_curve(const struct ec_group *group, struct bignum *p,
                       struct bignum *a, struct bignum *b,
                       struct bignum_ctx *bn_ctx);
int ec_group_set_curve_by_name(struct ec_group *group,
                               const struct asn1_oid *oid);
struct asn1_oid *ec_group_get_curve_name(const struct ec_group *group);
int ec_group_set_curve_name(struct ec_group *group, const struct asn1_oid *oid);
int ec_group_set_generator(struct ec_group *group, const struct ec_point *g,
                           const struct bignum *n, const struct bignum *h);
const struct bignum *ec_group_get0_cofactor(const struct ec_group *group);
const struct bignum *ec_group_get0_field(const struct ec_group *group);
const struct bignum *ec_group_get0_order(const struct ec_group *group);
const struct ec_point *ec_group_get0_generator(const struct ec_group *group);
int ec_group_get_degree(const struct ec_group *group);
void ec_group_set_asn1_flag(struct ec_group *group, int flag);
int ec_group_get_asn1_flag(const struct ec_group *group);
void ec_group_set_point_conversion_form(struct ec_group *group,
                                        enum point_conversion_form form);
enum point_conversion_form ec_group_get_point_conversion_form(const struct ec_group *group);
int ec_group_check(const struct ec_group *group, struct bignum_ctx *bn_ctx);

struct ec_key *ec_key_new(void);
void ec_key_free(struct ec_key *key);
int ec_key_up_ref(struct ec_key *key);
const struct ec_group *ec_key_get0_group(const struct ec_key *key);
int ec_key_set_group(struct ec_key *key, const struct ec_group *group);
const struct bignum *ec_key_get0_private_key(const struct ec_key *key);
int ec_key_set_private_key(struct ec_key *key, const struct bignum *pkey);
const struct ec_point *ec_key_get0_public_key(const struct ec_key *key);
int ec_key_set_public_key(struct ec_key *key, const struct ec_point *pkey);
int ec_key_generate_key(struct ec_key *key);
int ec_key_check_key(const struct ec_key *key);
void ec_key_set_asn1_flag(struct ec_key *key, int flag);
int ec_key_get_asn1_flag(struct ec_key *key);
void ec_key_set_conv_form(struct ec_key *key, enum point_conversion_form form);
enum point_conversion_form ec_key_get_conv_form(const struct ec_key *key);

struct ecdsa_sig *ecdsa_sig_new(void);
void ecdsa_sig_free(struct ecdsa_sig *sig);
void ecdsa_sig_get0(const struct ecdsa_sig *sig,
                    const struct bignum **r,
                    const struct bignum **s);
const struct bignum *ecdsa_sig_get0_r(const struct ecdsa_sig *sig);
const struct bignum *ecdsa_sig_get0_s(const struct ecdsa_sig *sig);
int ecdsa_sig_set0(struct ecdsa_sig *sig, struct bignum *r, struct bignum *s);

int ecdsa_size(const struct ec_key *key);
struct ecdsa_sig *ecdsa_do_sign(const uint8_t *tbs, size_t tbslen,
                                struct ec_key *key);
int ecdsa_sign(int type, const uint8_t *tbs, size_t tbslen,
               uint8_t *sig, size_t *siglen, struct ec_key *key);
int ecdsa_do_verify(const uint8_t *tbs, size_t tbslen,
                    const struct ecdsa_sig *ecdsa_sig, struct ec_key *key);
int ecdsa_verify(int type, const uint8_t *tbs, size_t tbslen,
                 const uint8_t *sig, size_t siglen, struct ec_key *key);

void ec_foreach_curve(int (*cb)(const char *name, const uint32_t *oid,
                                size_t oid_size, void *data),
                      void *data);
struct ec_group *ec_get_curvebyname(const char *name);

struct ec_group *a2i_ecpk_parameters(const struct asn1_object *object,
                                     struct ec_group **group);
struct asn1_object *i2a_ecpk_parameters(const struct ec_group *group);
struct ec_key *a2i_ec_private_key(const struct asn1_object *object,
                                  struct ec_key **key);
struct asn1_object *i2a_ec_private_key(const struct ec_key *key);
struct ecdsa_sig *a2i_ecdsa_sig(const struct asn1_object *object,
                                struct ecdsa_sig **sig);
struct asn1_object *i2a_ecdsa_sig(const struct ecdsa_sig *sig);
struct ec_key *a2i_ec_pubkey(const struct asn1_object *object,
                             struct ec_key **key);
struct asn1_object *i2a_ec_pubkey(const struct ec_key *key);

#endif
