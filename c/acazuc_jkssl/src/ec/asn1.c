#include "asn1/asn1.h"
#include "der/der.h"
#include "pem/pem.h"
#include "oid/oid.h"
#include "ec/ec.h"

#include <jkssl/bignum.h>
#include <jkssl/bio.h>
#include <jkssl/ec.h>

#include <stdlib.h>

ASN1_OBJ_ALLOC_DEF(ec_group, struct ec_group, ec_group_new);
ASN1_OBJ_ALLOC_DEF(ec_key, struct ec_key, ec_key_new);
ASN1_OBJ_ALLOC_DEF(ecdsa_sig, struct ecdsa_sig, ecdsa_sig_new);

static int get_ec_point(struct ec_point **dst, struct ec_point **src,
                        int *allocated, const struct ec_group *group)
{
	if (src)
		*dst = *src;
	if (!*dst)
	{
		*dst = ec_point_new(group);
		if (!*dst)
			return 0;
		*allocated = 1;
	}
	else
	{
		*allocated = 0;
	}
	return 1;
}

static struct ec_group *a2i_named_curve(const struct asn1_object *object,
                                        struct ec_group **groupp)
{
	struct asn1_oid *oid = (struct asn1_oid*)object;
	struct ec_group *group = NULL;
	int allocated = 0;

	if (!get_ec_group(&group, groupp, &allocated))
		return NULL;
	if (!ec_group_set_curve_by_name(group, oid)
	 || !ec_group_set_curve_name(group, oid))
		goto err;
	if (groupp)
		*groupp = group;
	return group;

err:
	if (allocated)
		ec_group_free(group);
	return NULL;
}

static struct ec_group *a2i_explicit_curve(const struct asn1_object *object,
                                           struct ec_group **groupp)
{
	struct ec_group *group = NULL;
	int allocated = 0;

	if (!get_ec_group(&group, groupp, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 6 /* XXX spec says the cofactor is optional */
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[3], ASN1_OCTET_STRING)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[4], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[5], ASN1_INTEGER))
		goto err;
	struct asn1_integer *version = (struct asn1_integer*)sequence->objects[0];
	struct asn1_constructed *field = (struct asn1_constructed*)sequence->objects[1];
	struct asn1_constructed *curve = (struct asn1_constructed*)sequence->objects[2];
	struct asn1_octet_string *base = (struct asn1_octet_string*)sequence->objects[3];
	struct asn1_integer *order = (struct asn1_integer*)sequence->objects[4];
	struct asn1_integer *cofactor = (struct asn1_integer*)sequence->objects[5];
	uint64_t v;
	if (!asn1_integer_get_uint64(version, &v)
	 || v != 1
	 || field->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(field->objects[0], ASN1_OBJECT_IDENTIFIER)
	 || !ASN1_IS_UNIVERSAL_TYPE(field->objects[1], ASN1_INTEGER) /* XXX only true for gfp */
	 || curve->objects_count < 2 /* XXX optional seed on third element */
	 || !ASN1_IS_UNIVERSAL_TYPE(curve->objects[0], ASN1_OCTET_STRING)
	 || !ASN1_IS_UNIVERSAL_TYPE(curve->objects[1], ASN1_OCTET_STRING))
		goto err;
	struct asn1_oid *type = (struct asn1_oid*)field->objects[0];
	struct asn1_integer *p = (struct asn1_integer*)field->objects[1];
	struct asn1_octet_string *a = (struct asn1_octet_string*)curve->objects[0];
	struct asn1_octet_string *b = (struct asn1_octet_string*)curve->objects[1];
	if (!group->g)
	{
		group->g = ec_point_new(group);
		if (!group->g)
			goto err;
	}
	struct bignum *bn_a = bignum_bin2bignum(a->data, a->size, group->a);
	struct bignum *bn_b = bignum_bin2bignum(b->data, b->size, group->b);
	struct bignum_ctx *bn_ctx = bignum_ctx_new();
	if (!bn_ctx
	 || asn1_oid_cmp_uint32(type, oid_x9_62_prime_field, sizeof(oid_x9_62_prime_field) / sizeof(*oid_x9_62_prime_field))
	 || !bn_a
	 || !bn_b
	 || !asn1_integer_get_bignum(p, &group->p)
	 || !asn1_integer_get_bignum(order, &group->n)
	 || !asn1_integer_get_bignum(cofactor, &group->h)
	 || !ec_point_oct2point(group, group->g, base->data, base->size, bn_ctx))
	{
		bignum_ctx_free(bn_ctx);
		goto err;
	}
	group->a = bn_a;
	group->b = bn_b;
	bignum_ctx_free(bn_ctx);
	if (groupp)
		*groupp = group;
	return group;

err:
	if (allocated)
		ec_group_free(group);
	return NULL;
}

struct ec_group *a2i_ecpk_parameters(const struct asn1_object *object,
                                     struct ec_group **group)
{
	if (ASN1_IS_UNIVERSAL_TYPE(object, ASN1_OBJECT_IDENTIFIER))
		return a2i_named_curve(object, group);
	return a2i_explicit_curve(object, group);
}

static struct asn1_object *i2a_named_curve(const struct ec_group *group)
{
	if (!group->oid)
		return NULL;
	return (struct asn1_object*)asn1_oid_dup(group->oid);
}

static struct asn1_object *i2a_explicit_curve(const struct ec_group *group)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *version = asn1_integer_new();
	struct asn1_constructed *field = asn1_sequence_new();
	struct asn1_constructed *curve = asn1_sequence_new();
	struct asn1_octet_string *base = asn1_octet_string_new();
	struct asn1_integer *order = asn1_integer_new();
	struct asn1_integer *cofactor = asn1_integer_new();
	struct asn1_oid *type = asn1_oid_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_octet_string *a = asn1_octet_string_new();
	struct asn1_octet_string *b = asn1_octet_string_new();
	uint8_t *a_data = NULL;
	size_t a_size;
	uint8_t *b_data = NULL;
	size_t b_size;
	uint8_t *g_data = NULL;
	size_t g_size;
	struct bignum_ctx *bn_ctx = bignum_ctx_new();
	struct asn1_object *ret = NULL;

	a_size = bignum_num_bytes(group->a);
	a_data = malloc(a_size);
	b_size = bignum_num_bytes(group->b);
	b_data = malloc(b_size);
	g_size = ec_point_point2buf(group, group->g, group->conv_form,
	                            &g_data, bn_ctx);
	if (!sequence
	 || !version
	 || !field
	 || !curve
	 || !base
	 || !order
	 || !cofactor
	 || !type
	 || !p
	 || !a
	 || !b
	 || !a_data
	 || !b_data
	 || !g_size
	 || !bn_ctx
	 || !bignum_bignum2bin(group->a, a_data)
	 || !bignum_bignum2bin(group->b, b_data)
	 || !asn1_integer_set_uint64(version, 1)
	 || !asn1_oid_set1(type, oid_x9_62_prime_field, sizeof(oid_x9_62_prime_field) / sizeof(*oid_x9_62_prime_field))
	 || !asn1_integer_set_bignum(p, group->p)
	 || !asn1_constructed_add1(field, (struct asn1_object*)type)
	 || !asn1_constructed_add1(field, (struct asn1_object*)p)
	 || !asn1_octet_string_set1(a, a_data, a_size)
	 || !asn1_octet_string_set1(b, b_data, b_size)
	 || !asn1_constructed_add1(curve, (struct asn1_object*)a)
	 || !asn1_constructed_add1(curve, (struct asn1_object*)b)
	 || !asn1_octet_string_set1(base, g_data, g_size)
	 || !asn1_integer_set_bignum(order, group->n)
	 || !asn1_integer_set_bignum(cofactor, group->h)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)version)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)field)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)curve)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)base)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)order)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)cofactor))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(version);
	asn1_constructed_free(field);
	asn1_constructed_free(curve);
	asn1_octet_string_free(base);
	asn1_integer_free(order);
	asn1_integer_free(cofactor);
	asn1_oid_free(type);
	asn1_integer_free(p);
	asn1_octet_string_free(a);
	asn1_octet_string_free(b);
	free(a_data);
	free(b_data);
	free(g_data);
	bignum_ctx_free(bn_ctx);
	return ret;
}

struct asn1_object *i2a_ecpk_parameters(const struct ec_group *group)
{
	switch (group->asn1_flag)
	{
		case JKSSL_EC_NAMED_CURVE:
			if (group->oid)
				return i2a_named_curve(group);
			return i2a_explicit_curve(group);
		case JKSSL_EC_EXPLICIT_CURVE:
			return i2a_explicit_curve(group);
		default:
			return NULL;
	}
}

DER_DEF(ecpk_parameters, ec_group);
PEM_DEF_RCWC(ecpk_parameters, ec_group, "EC PARAMETERS");

static struct ec_point *a2i_pubkey_bs(const struct ec_group *group,
                                      struct asn1_object *object,
                                      struct ec_point **pointp)
{
	struct ec_point *point = NULL;
	int allocated = 0;

	if (!get_ec_point(&point, pointp, &allocated, group))
		return NULL;
	if (!group
	 || !ASN1_IS_UNIVERSAL_TYPE(object, ASN1_BIT_STRING))
		goto err;
	struct asn1_bit_string *pubkey = (struct asn1_bit_string*)object;
	struct bignum_ctx *bn_ctx = bignum_ctx_new();
	if (pubkey->bits % 8
	 || !point
	 || !bn_ctx
	 || !ec_point_oct2point(group, point, pubkey->data,
	                        pubkey->bits / 8, bn_ctx))
	{
		bignum_ctx_free(bn_ctx);
		goto err;
	}
	bignum_ctx_free(bn_ctx);
	if (pointp)
		*pointp = point;
	return point;

err:
	if (allocated)
		ec_point_free(point);
	return NULL;
}

struct ec_key *a2i_ec_private_key(const struct asn1_object *object,
                                  struct ec_key **keyp)
{
	struct ec_key *key = NULL;
	int allocated = 0;

	if (!get_ec_key(&key, keyp, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	const struct asn1_constructed *sequence = (const struct asn1_constructed*)object;
	if (sequence->objects_count < 2
	 || sequence->objects_count > 4
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_OCTET_STRING))
		goto err;
	const struct asn1_octet_string *priv = (const struct asn1_octet_string*)sequence->objects[1];
	uint64_t v;
	if (!asn1_integer_get_uint64((const struct asn1_integer*)sequence->objects[0], &v)
	 || v != 1)
		goto err;
	struct bignum *bn_priv = bignum_bin2bignum(priv->data, priv->size, key->priv);
	if (!bn_priv)
		goto err;
	key->priv = bn_priv;
	if (sequence->objects_count >= 3)
	{
		if (!ASN1_IS_CONTEXT_TYPE(sequence->objects[2], ASN1_CONSTRUCTED, 0))
			goto err;
		struct asn1_constructed *ecpk_parameters = (struct asn1_constructed*)sequence->objects[2];
		if (ecpk_parameters->objects_count != 1)
			goto err;
		struct ec_group *group = a2i_ecpk_parameters(ecpk_parameters->objects[0], &key->group);
		if (!group)
			goto err;
		key->group = group;
	}
	else
	{
		ec_group_free(key->group);
		key->group = NULL;
	}
	if (sequence->objects_count >= 4)
	{
		if (!ASN1_IS_CONTEXT_TYPE(sequence->objects[3], ASN1_CONSTRUCTED, 1))
			goto err;
		struct asn1_constructed *pubkey = (struct asn1_constructed*)sequence->objects[3];
		if (pubkey->objects_count != 1)
			goto err;
		struct ec_point *point = a2i_pubkey_bs(key->group, pubkey->objects[0], &key->pub);
		if (!point)
			goto err;
		key->pub = point;
	}
	else
	{
		ec_point_free(key->pub);
		key->pub = NULL;
	}
	if (keyp)
		*keyp = key;
	return key;

err:
	if (allocated)
		ec_key_free(key);
	return NULL;
}

static struct asn1_object *i2a_pubkey_bs(const struct ec_group *group,
                                         const struct ec_point *point)
{
	struct asn1_bit_string *pubkey = asn1_bit_string_new();
	struct asn1_object *ret = NULL;
	struct bignum_ctx *bn_ctx = bignum_ctx_new();
	uint8_t *pubkey_data = NULL;
	size_t size;

	if (!bn_ctx)
		goto end;
	size = ec_point_point2buf(group, point, group->conv_form, &pubkey_data, bn_ctx);
	if (!size
	 || !asn1_bit_string_set1(pubkey, pubkey_data, size * 8))
		goto end;
	ret = (struct asn1_object*)pubkey;

end:
	if (!ret)
		asn1_bit_string_free(pubkey);
	bignum_ctx_free(bn_ctx);
	free(pubkey_data);
	return ret;
}

struct asn1_object *i2a_ec_private_key(const struct ec_key *key)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *version = asn1_integer_new();
	struct asn1_octet_string *private_key = asn1_octet_string_new();
	struct asn1_object *ecpk_parameters = NULL;
	struct asn1_constructed *ec_group = NULL;
	struct asn1_object *public_key = NULL;
	struct asn1_constructed *pubkey = NULL;
	struct asn1_object *ret = NULL;
	uint8_t *pkey_data = NULL;
	size_t pkey_size;

	pkey_size = bignum_num_bytes(key->priv);
	pkey_data = malloc(pkey_size);
	if (!pkey_data)
		goto end;
	bignum_bignum2bin(key->priv, pkey_data);
	if (!sequence
	 || !version
	 || !private_key
	 || !asn1_integer_set_uint64(version, 1)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)version)
	 || !asn1_octet_string_set0(private_key, pkey_data, pkey_size)
	 || (pkey_data = NULL)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)private_key))
		goto end;
	if (key->group)
	{
		ec_group = asn1_constructed_new(ASN1_CONTEXT, 0);
		ecpk_parameters = i2a_ecpk_parameters(key->group);
		if (!ec_group
		 || !ecpk_parameters
		 || !asn1_constructed_add1(ec_group, ecpk_parameters)
		 || !asn1_constructed_add1(sequence, (struct asn1_object*)ec_group))
			goto end;
	}
	if (key->pub)
	{
		pubkey = asn1_constructed_new(ASN1_CONTEXT, 1);
		if (!pubkey)
			goto end;
		public_key = i2a_pubkey_bs(key->group, key->pub);
		if (!public_key
		 || !asn1_constructed_add1(pubkey, public_key)
		 || !asn1_constructed_add1(sequence, (struct asn1_object*)pubkey))
			goto end;
	}
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(version);
	asn1_octet_string_free(private_key);
	asn1_object_free(ecpk_parameters);
	asn1_constructed_free(ec_group);
	asn1_constructed_free(pubkey);
	asn1_object_free(public_key);
	free(pkey_data);
	return ret;
}

DER_DEF(ec_private_key, ec_key);
PEM_DEF_RPWP(ec_private_key, ec_key, "EC PRIVATE KEY");

struct ecdsa_sig *a2i_ecdsa_sig(const struct asn1_object *object,
                                struct ecdsa_sig **sigp)
{
	struct ecdsa_sig *sig = NULL;
	int allocated = 0;

	if (!get_ecdsa_sig(&sig, sigp, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER))
		goto err;
	struct asn1_integer *r = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *s = (struct asn1_integer*)sequence->objects[1];
	if (!asn1_integer_get_bignum(r, &sig->r)
	 || !asn1_integer_get_bignum(s, &sig->s))
		goto err;
	if (sigp)
		*sigp = sig;
	return sig;

err:
	if (allocated)
		ecdsa_sig_free(sig);
	return NULL;
}

struct asn1_object *i2a_ecdsa_sig(const struct ecdsa_sig *sig)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *r = asn1_integer_new();
	struct asn1_integer *s = asn1_integer_new();
	struct asn1_object *ret = NULL;

	if (!sequence
	 || !r
	 || !s
	 || !asn1_integer_set_bignum(r, sig->r)
	 || !asn1_integer_set_bignum(s, sig->s)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)r)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)s))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(r);
	asn1_integer_free(s);
	return ret;
}

DER_DEF(ecdsa_sig, ecdsa_sig);

struct ec_key *a2i_ec_pubkey(const struct asn1_object *object,
                             struct ec_key **keyp)
{
	struct asn1_oid *algorithm;
	struct asn1_object *parameters;
	struct asn1_bit_string *key_data;
	struct ec_key *key = NULL;
	int allocated = 0;

	if (!get_ec_key(&key, keyp, &allocated)
	 || !asn1_decode_spki(object, &algorithm, &parameters, &key_data)
	 || !ASN1_IS_UNIVERSAL_TYPE(parameters, ASN1_OBJECT_IDENTIFIER)
	 || asn1_oid_cmp_uint32(algorithm, oid_ecPublicKey,
	                        sizeof(oid_ecPublicKey) / sizeof(*oid_ecPublicKey)))
		goto err;
	struct ec_group *group = a2i_ecpk_parameters(parameters, &key->group);
	if (!group)
		goto err;
	key->group = group;
	struct ec_point *point = a2i_pubkey_bs(group, (struct asn1_object*)key_data, &key->pub);
	if (!point)
		goto err;
	key->pub = point;
	if (keyp)
		*keyp = key;
	return key;

err:
	if (allocated)
		ec_key_free(key);
	return NULL;
}

struct asn1_object *i2a_ec_pubkey(const struct ec_key *key)
{
	uint8_t *pubkey_data = NULL;
	struct asn1_oid *algorithm = asn1_oid_new();
	struct asn1_object *parameters = i2a_ecpk_parameters(key->group);
	struct asn1_object *pubkey = i2a_pubkey_bs(key->group, key->pub);
	struct asn1_object *ret = NULL;

	if (!algorithm
	 || !parameters
	 || !pubkey
	 || !asn1_oid_set1(algorithm, oid_ecPublicKey,
	                   sizeof(oid_ecPublicKey) / sizeof(*oid_ecPublicKey)))
		goto end;
	ret = asn1_encode_spki(algorithm, (struct asn1_object*)parameters,
	                       (struct asn1_bit_string*)pubkey);

end:
	asn1_oid_free(algorithm);
	asn1_object_free(parameters);
	asn1_object_free(pubkey);
	free(pubkey_data);
	return ret;
}

DER_DEF(ec_pubkey, ec_key);
PEM_DEF_RPWC(ec_pubkey, ec_key, "PUBLIC KEY");
