#ifndef JKSSL_EC_EC_H
#define JKSSL_EC_EC_H

#include "refcount.h"

#include <jkssl/ec.h>

struct asn1_oid;
struct ec_point;
struct bignum;

struct ec_group
{
	struct asn1_oid *oid;
	struct ec_point *g;
	struct bignum *p;
	struct bignum *a;
	struct bignum *b;
	struct bignum *n;
	struct bignum *h;
	int asn1_flag;
	enum point_conversion_form conv_form;
};

struct ec_point
{
	int infinity;
	struct bignum *x;
	struct bignum *y;
};

struct ec_key
{
	struct ec_group *group;
	struct bignum *priv;
	struct ec_point *pub;
	refcount_t refcount;
};

struct gfp_curve
{
	const char *name;
	const uint32_t *oid_data;
	size_t oid_size;
	const uint8_t *p_data;
	size_t p_size;
	const uint8_t *a_data;
	size_t a_size;
	const uint8_t *b_data;
	size_t b_size;
	const uint8_t *n_data;
	size_t n_size;
	const uint8_t *h_data;
	size_t h_size;
	const uint8_t *gx_data;
	size_t gx_size;
	const uint8_t *gy_data;
	size_t gy_size;
};

struct ecdsa_sig
{
	struct bignum *r;
	struct bignum *s;
};

extern const struct gfp_curve prime192v1;
extern const struct gfp_curve prime192v2;
extern const struct gfp_curve prime192v3;
extern const struct gfp_curve prime239v1;
extern const struct gfp_curve prime239v2;
extern const struct gfp_curve prime239v3;
extern const struct gfp_curve prime256v1;
extern const struct gfp_curve secp112r1;
extern const struct gfp_curve secp112r2;
extern const struct gfp_curve secp128r1;
extern const struct gfp_curve secp128r2;
extern const struct gfp_curve secp160r1;
extern const struct gfp_curve secp160r2;
extern const struct gfp_curve secp160k1;
extern const struct gfp_curve secp192k1;
extern const struct gfp_curve secp224r1;
extern const struct gfp_curve secp224k1;
extern const struct gfp_curve secp256k1;
extern const struct gfp_curve secp384r1;
extern const struct gfp_curve secp521r1;
extern const struct gfp_curve frp256v1;
extern const struct gfp_curve sm2;
extern const struct gfp_curve gost3410_12_256;
extern const struct gfp_curve gost3410_12_512;
extern const struct gfp_curve brainpoolP160r1;
extern const struct gfp_curve brainpoolP160t1;
extern const struct gfp_curve brainpoolP192r1;
extern const struct gfp_curve brainpoolP192t1;
extern const struct gfp_curve brainpoolP224r1;
extern const struct gfp_curve brainpoolP224t1;
extern const struct gfp_curve brainpoolP256r1;
extern const struct gfp_curve brainpoolP256t1;
extern const struct gfp_curve brainpoolP320r1;
extern const struct gfp_curve brainpoolP320t1;
extern const struct gfp_curve brainpoolP384r1;
extern const struct gfp_curve brainpoolP384t1;
extern const struct gfp_curve brainpoolP512r1;
extern const struct gfp_curve brainpoolP512t1;

#endif
