#include "asn1/asn1.h"
#include "der/der.h"
#include "pem/pem.h"
#include "oid/oid.h"

#include <jkssl/dsa.h>
#include <jkssl/bio.h>

#include <stdlib.h>
#include <string.h>

struct dsa_ctx *a2i_dsa_params(const struct asn1_object *object,
                               struct dsa_ctx **ctx)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 3
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_INTEGER))
		return NULL;
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *q = (struct asn1_integer*)sequence->objects[1];
	struct asn1_integer *g = (struct asn1_integer*)sequence->objects[2];
	if (!asn1_integer_get_bignum(p, &(*ctx)->p)
	 || !asn1_integer_get_bignum(q, &(*ctx)->q)
	 || !asn1_integer_get_bignum(g, &(*ctx)->g))
		return NULL;
	return *ctx;
}

struct asn1_object *i2a_dsa_params(struct dsa_ctx *ctx)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_integer *q = asn1_integer_new();
	struct asn1_integer *g = asn1_integer_new();
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !p
	 || !q
	 || !g
	 || !asn1_integer_set_bignum(p, ctx->p)
	 || !asn1_integer_set_bignum(q, ctx->q)
	 || !asn1_integer_set_bignum(g, ctx->g)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)p)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)q)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)g))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(p);
	asn1_integer_free(q);
	asn1_integer_free(g);
	return ret;
}

DER_DEF(dsa_params, dsa_ctx);
PEM_DEF_RPWC(dsa_params, dsa_ctx, "DSA PARAMETERS");

struct dsa_ctx *a2i_dsa_private_key(const struct asn1_object *object,
                                    struct dsa_ctx **ctx)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 6
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[3], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[4], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[5], ASN1_INTEGER))
		return NULL;
	struct asn1_integer *version = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[1];
	struct asn1_integer *q = (struct asn1_integer*)sequence->objects[2];
	struct asn1_integer *g = (struct asn1_integer*)sequence->objects[3];
	struct asn1_integer *y = (struct asn1_integer*)sequence->objects[4];
	struct asn1_integer *x = (struct asn1_integer*)sequence->objects[5];
	uint64_t v;
	if (!asn1_integer_get_uint64(version, &v)
	 || v
	 || !asn1_integer_get_bignum(p, &(*ctx)->p)
	 || !asn1_integer_get_bignum(q, &(*ctx)->q)
	 || !asn1_integer_get_bignum(g, &(*ctx)->g)
	 || !asn1_integer_get_bignum(y, &(*ctx)->y)
	 || !asn1_integer_get_bignum(x, &(*ctx)->x))
		return NULL;
	return *ctx;
}

struct asn1_object *i2a_dsa_private_key(struct dsa_ctx *ctx)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *version = asn1_integer_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_integer *q = asn1_integer_new();
	struct asn1_integer *g = asn1_integer_new();
	struct asn1_integer *y = asn1_integer_new();
	struct asn1_integer *x = asn1_integer_new();
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !version
	 || !p
	 || !q
	 || !g
	 || !y
	 || !x
	 || !asn1_integer_set_uint64(version, 0)
	 || !asn1_integer_set_bignum(p, ctx->p)
	 || !asn1_integer_set_bignum(q, ctx->q)
	 || !asn1_integer_set_bignum(g, ctx->g)
	 || !asn1_integer_set_bignum(y, ctx->y)
	 || !asn1_integer_set_bignum(x, ctx->x)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)version)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)p)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)q)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)g)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)y)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)x))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(version);
	asn1_integer_free(p);
	asn1_integer_free(q);
	asn1_integer_free(g);
	asn1_integer_free(y);
	asn1_integer_free(x);
	return ret;
}

DER_DEF(dsa_private_key, dsa_ctx);
PEM_DEF_RPWP(dsa_private_key, dsa_ctx, "DSA PRIVATE KEY");

struct dsa_ctx *a2i_dsa_public_key(const struct asn1_object *object,
                                   struct dsa_ctx **ctx)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 5
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[3], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[4], ASN1_INTEGER))
		return NULL;
	struct asn1_integer *version = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *y = (struct asn1_integer*)sequence->objects[1];
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[2];
	struct asn1_integer *q = (struct asn1_integer*)sequence->objects[3];
	struct asn1_integer *g = (struct asn1_integer*)sequence->objects[4];
	uint64_t v;
	if (!asn1_integer_get_uint64(version, &v)
	 || v
	 || !asn1_integer_get_bignum(y, &(*ctx)->y)
	 || !asn1_integer_get_bignum(p, &(*ctx)->p)
	 || !asn1_integer_get_bignum(q, &(*ctx)->q)
	 || !asn1_integer_get_bignum(g, &(*ctx)->g))
		return NULL;
	return *ctx;
}

struct asn1_object *i2a_dsa_public_key(struct dsa_ctx *ctx)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *version = asn1_integer_new();
	struct asn1_integer *y = asn1_integer_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_integer *q = asn1_integer_new();
	struct asn1_integer *g = asn1_integer_new();
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !version
	 || !y
	 || !p
	 || !q
	 || !g
	 || !asn1_integer_set_uint64(version, 0)
	 || !asn1_integer_set_bignum(y, ctx->y)
	 || !asn1_integer_set_bignum(p, ctx->p)
	 || !asn1_integer_set_bignum(q, ctx->q)
	 || !asn1_integer_set_bignum(g, ctx->g)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)version)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)y)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)p)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)q)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)g))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(version);
	asn1_integer_free(y);
	asn1_integer_free(p);
	asn1_integer_free(q);
	asn1_integer_free(g);
	return ret;
}

DER_DEF(dsa_public_key, dsa_ctx);
PEM_DEF_RPWC(dsa_public_key, dsa_ctx, "DSA PUBLIC KEY");

static int a2i_pubkey_key(struct dsa_ctx *ctx, uint8_t *data, size_t size)
{
	struct asn1_object *object = d2i_asn1_object(&object, &data, size);
	int ret = 0;
	if (!object)
		return 0;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_INTEGER))
		goto end;
	struct asn1_integer *y = (struct asn1_integer*)object;
	if (!asn1_integer_get_bignum(y, &ctx->y))
		goto end;
	ret = 1;

end:
	asn1_object_free(object);
	return ret;
}

struct dsa_ctx *a2i_dsa_pubkey(const struct asn1_object *object,
                               struct dsa_ctx **ctx)
{
	struct asn1_oid *algorithm;
	struct asn1_object *parameters;
	struct asn1_bit_string *key_data;
	if (!asn1_decode_spki(object, &algorithm, &parameters, &key_data)
	 || !ASN1_IS_UNIVERSAL_TYPE(parameters, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *params_seq = (struct asn1_constructed*)parameters;
	if (asn1_oid_cmp_uint32(algorithm, oid_dsa,
	                        sizeof(oid_dsa) / sizeof(*oid_dsa))
	 || params_seq->objects_count != 3
	 || !ASN1_IS_UNIVERSAL_TYPE(params_seq->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(params_seq->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(params_seq->objects[2], ASN1_INTEGER))
		return NULL;
	struct asn1_integer *p = (struct asn1_integer*)params_seq->objects[0];
	struct asn1_integer *q = (struct asn1_integer*)params_seq->objects[1];
	struct asn1_integer *g = (struct asn1_integer*)params_seq->objects[2];
	if (!asn1_integer_get_bignum(p, &(*ctx)->p)
	 || !asn1_integer_get_bignum(q, &(*ctx)->q)
	 || !asn1_integer_get_bignum(g, &(*ctx)->g)
	 || !a2i_pubkey_key(*ctx, key_data->data, key_data->bits / 8))
		return NULL;
	return *ctx;
}

static int i2a_pubkey_key(struct dsa_ctx *ctx, uint8_t **data)
{
	struct asn1_integer *y = asn1_integer_new();
	int ret = -1;
	if (!y
	 || !asn1_integer_set_bignum(y, ctx->y))
		goto end;
	ret = i2d_asn1_object((struct asn1_object*)y, data);

end:
	asn1_integer_free(y);
	return ret;
}

struct asn1_object *i2a_dsa_pubkey(struct dsa_ctx *ctx)
{
	uint8_t *pubkey_data = NULL;
	int key_len = i2a_pubkey_key(ctx, &pubkey_data);
	if (key_len == -1)
		return NULL;
	struct asn1_bit_string *key_data = asn1_bit_string_new();
	if (!key_data)
	{
		free(pubkey_data);
		return NULL;
	}
	if (!asn1_bit_string_set0(key_data, pubkey_data, key_len * 8))
	{
		asn1_bit_string_free(key_data);
		free(pubkey_data);
		return NULL;
	}
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_constructed *alg_seq = asn1_sequence_new();
	struct asn1_oid *alg = asn1_oid_new();
	struct asn1_constructed *params_seq = asn1_sequence_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_integer *q = asn1_integer_new();
	struct asn1_integer *g = asn1_integer_new();
	struct asn1_object *ret = NULL;
	if (!sequence
	 || !alg_seq
	 || !alg
	 || !params_seq
	 || !p
	 || !q
	 || !g
	 || !key_data
	 || !asn1_integer_set_bignum(p, ctx->p)
	 || !asn1_integer_set_bignum(q, ctx->q)
	 || !asn1_integer_set_bignum(g, ctx->g)
	 || !asn1_oid_set1(alg, oid_dsa, sizeof(oid_dsa) / sizeof(*oid_dsa))
	 || !asn1_constructed_add1(params_seq, (struct asn1_object*)p)
	 || !asn1_constructed_add1(params_seq, (struct asn1_object*)q)
	 || !asn1_constructed_add1(params_seq, (struct asn1_object*)g)
	 || !asn1_constructed_add1(alg_seq, (struct asn1_object*)alg)
	 || !asn1_constructed_add1(alg_seq, (struct asn1_object*)params_seq)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)alg_seq)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)key_data))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_constructed_free(alg_seq);
	asn1_oid_free(alg);
	asn1_constructed_free(params_seq);
	asn1_integer_free(p);
	asn1_integer_free(q);
	asn1_integer_free(g);
	return ret;
}

DER_DEF(dsa_pubkey, dsa_ctx);
PEM_DEF_RPWC(dsa_pubkey, dsa_ctx, "PUBLIC KEY");

struct dsa_sig *a2i_dsa_sig(const struct asn1_object *object,
                            struct dsa_sig **sig)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER))
		return NULL;
	struct asn1_integer *r = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *s = (struct asn1_integer*)sequence->objects[1];
	if (!asn1_integer_get_bignum(r, &(*sig)->r)
	 || !asn1_integer_get_bignum(s, &(*sig)->s))
		return NULL;
	return *sig;
}

struct asn1_object *i2a_dsa_sig(struct dsa_sig *sig)
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

DER_DEF(dsa_sig, dsa_sig);
