#include "asn1/asn1.h"
#include "der/der.h"
#include "pem/pem.h"
#include "oid/oid.h"
#include "dsa/dsa.h"

#include <jkssl/dsa.h>
#include <jkssl/bio.h>

#include <stdlib.h>
#include <string.h>

ASN1_OBJ_ALLOC_DEF(dsa, struct dsa, dsa_new);
ASN1_OBJ_ALLOC_DEF(dsa_sig, struct dsa_sig, dsa_sig_new);

struct dsa *a2i_dsa_params(const struct asn1_object *object,
                           struct dsa **dsap)
{
	struct dsa *dsa = NULL;
	int allocated = 0;

	if (!get_dsa(&dsa, dsap, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 3
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_INTEGER))
		goto err;
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *q = (struct asn1_integer*)sequence->objects[1];
	struct asn1_integer *g = (struct asn1_integer*)sequence->objects[2];
	if (!asn1_integer_get_bignum(p, &dsa->p)
	 || !asn1_integer_get_bignum(q, &dsa->q)
	 || !asn1_integer_get_bignum(g, &dsa->g))
		goto err;
	if (dsap)
		*dsap = dsa;
	return dsa;

err:
	if (allocated)
		dsa_free(dsa);
	return NULL;
}

struct asn1_object *i2a_dsa_params(const struct dsa *dsa)
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
	 || !asn1_integer_set_bignum(p, dsa->p)
	 || !asn1_integer_set_bignum(q, dsa->q)
	 || !asn1_integer_set_bignum(g, dsa->g)
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

DER_DEF(dsa_params, dsa);
PEM_DEF_RPWC(dsa_params, dsa, "DSA PARAMETERS");

struct dsa *a2i_dsa_private_key(const struct asn1_object *object,
                                struct dsa **dsap)
{
	struct dsa *dsa = NULL;
	int allocated = 0;

	if (!get_dsa(&dsa, dsap, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 6
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[3], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[4], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[5], ASN1_INTEGER))
		goto err;
	struct asn1_integer *version = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[1];
	struct asn1_integer *q = (struct asn1_integer*)sequence->objects[2];
	struct asn1_integer *g = (struct asn1_integer*)sequence->objects[3];
	struct asn1_integer *y = (struct asn1_integer*)sequence->objects[4];
	struct asn1_integer *x = (struct asn1_integer*)sequence->objects[5];
	uint64_t v;
	if (!asn1_integer_get_uint64(version, &v)
	 || v
	 || !asn1_integer_get_bignum(p, &dsa->p)
	 || !asn1_integer_get_bignum(q, &dsa->q)
	 || !asn1_integer_get_bignum(g, &dsa->g)
	 || !asn1_integer_get_bignum(y, &dsa->y)
	 || !asn1_integer_get_bignum(x, &dsa->x))
		goto err;
	if (dsap)
		*dsap = dsa;
	return dsa;

err:
	if (allocated)
		dsa_free(dsa);
	return NULL;
}

struct asn1_object *i2a_dsa_private_key(const struct dsa *dsa)
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
	 || !asn1_integer_set_bignum(p, dsa->p)
	 || !asn1_integer_set_bignum(q, dsa->q)
	 || !asn1_integer_set_bignum(g, dsa->g)
	 || !asn1_integer_set_bignum(y, dsa->y)
	 || !asn1_integer_set_bignum(x, dsa->x)
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

DER_DEF(dsa_private_key, dsa);
PEM_DEF_RPWP(dsa_private_key, dsa, "DSA PRIVATE KEY");

struct dsa *a2i_dsa_public_key(const struct asn1_object *object,
                               struct dsa **dsap)
{
	struct dsa *dsa = NULL;
	int allocated = 0;

	if (!get_dsa(&dsa, dsap, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 5
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[3], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[4], ASN1_INTEGER))
		goto err;
	struct asn1_integer *version = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *y = (struct asn1_integer*)sequence->objects[1];
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[2];
	struct asn1_integer *q = (struct asn1_integer*)sequence->objects[3];
	struct asn1_integer *g = (struct asn1_integer*)sequence->objects[4];
	uint64_t v;
	if (!asn1_integer_get_uint64(version, &v)
	 || v
	 || !asn1_integer_get_bignum(y, &dsa->y)
	 || !asn1_integer_get_bignum(p, &dsa->p)
	 || !asn1_integer_get_bignum(q, &dsa->q)
	 || !asn1_integer_get_bignum(g, &dsa->g))
		goto err;
	if (dsap)
		*dsap = dsa;
	return dsa;

err:
	if (allocated)
		dsa_free(dsa);
	return NULL;
}

struct asn1_object *i2a_dsa_public_key(const struct dsa *dsa)
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
	 || !asn1_integer_set_bignum(y, dsa->y)
	 || !asn1_integer_set_bignum(p, dsa->p)
	 || !asn1_integer_set_bignum(q, dsa->q)
	 || !asn1_integer_set_bignum(g, dsa->g)
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

DER_DEF(dsa_public_key, dsa);
PEM_DEF_RPWC(dsa_public_key, dsa, "DSA PUBLIC KEY");

static int a2i_pubkey_key(struct dsa *dsa, uint8_t *data, size_t size)
{
	struct asn1_object *object = d2i_asn1_object(&object, &data, size);
	int ret = 0;

	if (!object)
		return 0;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_INTEGER))
		goto end;
	struct asn1_integer *y = (struct asn1_integer*)object;
	if (!asn1_integer_get_bignum(y, &dsa->y))
		goto end;
	ret = 1;

end:
	asn1_object_free(object);
	return ret;
}

struct dsa *a2i_dsa_pubkey(const struct asn1_object *object,
                           struct dsa **dsap)
{
	struct dsa *dsa = NULL;
	int allocated = 0;

	if (!get_dsa(&dsa, dsap, &allocated))
		return NULL;
	struct asn1_oid *algorithm;
	struct asn1_object *parameters;
	struct asn1_bit_string *key_data;
	if (!asn1_decode_spki(object, &algorithm, &parameters, &key_data)
	 || !ASN1_IS_UNIVERSAL_TYPE(parameters, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *params_seq = (struct asn1_constructed*)parameters;
	if (asn1_oid_cmp_uint32(algorithm, oid_dsa,
	                        sizeof(oid_dsa) / sizeof(*oid_dsa))
	 || params_seq->objects_count != 3
	 || !ASN1_IS_UNIVERSAL_TYPE(params_seq->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(params_seq->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(params_seq->objects[2], ASN1_INTEGER))
		goto err;
	struct asn1_integer *p = (struct asn1_integer*)params_seq->objects[0];
	struct asn1_integer *q = (struct asn1_integer*)params_seq->objects[1];
	struct asn1_integer *g = (struct asn1_integer*)params_seq->objects[2];
	if (!asn1_integer_get_bignum(p, &dsa->p)
	 || !asn1_integer_get_bignum(q, &dsa->q)
	 || !asn1_integer_get_bignum(g, &dsa->g)
	 || !a2i_pubkey_key(dsa, key_data->data, key_data->bits / 8))
		goto err;
	if (dsap)
		*dsap = dsa;
	return dsa;

err:
	if (allocated)
		dsa_free(dsa);
	return NULL;
}

static int i2a_pubkey_key(const struct dsa *dsa, uint8_t **data)
{
	struct asn1_integer *y = asn1_integer_new();
	int ret = -1;

	if (!y
	 || !asn1_integer_set_bignum(y, dsa->y))
		goto end;
	ret = i2d_asn1_object((struct asn1_object*)y, data);

end:
	asn1_integer_free(y);
	return ret;
}

struct asn1_object *i2a_dsa_pubkey(const struct dsa *dsa)
{
	uint8_t *pubkey_data = NULL;
	struct asn1_oid *algorithm = asn1_oid_new();
	struct asn1_constructed *parameters = asn1_sequence_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_integer *q = asn1_integer_new();
	struct asn1_integer *g = asn1_integer_new();
	struct asn1_bit_string *pubkey = asn1_bit_string_new();
	struct asn1_object *ret = NULL;
	int key_len = i2a_pubkey_key(dsa, &pubkey_data);

	if (key_len == -1
	 || !algorithm
	 || !parameters
	 || !p
	 || !q
	 || !g
	 || !pubkey
	 || !asn1_oid_set1(algorithm, oid_dsa, sizeof(oid_dsa) / sizeof(*oid_dsa))
	 || !asn1_integer_set_bignum(p, dsa->p)
	 || !asn1_integer_set_bignum(q, dsa->q)
	 || !asn1_integer_set_bignum(g, dsa->g)
	 || !asn1_constructed_add1(parameters, (struct asn1_object*)p)
	 || !asn1_constructed_add1(parameters, (struct asn1_object*)q)
	 || !asn1_constructed_add1(parameters, (struct asn1_object*)g)
	 || !asn1_bit_string_set1(pubkey, pubkey_data, key_len * 8))
		goto end;
	ret = asn1_encode_spki(algorithm, (struct asn1_object*)parameters,
	                       pubkey);

end:
	asn1_oid_free(algorithm);
	asn1_constructed_free(parameters);
	asn1_integer_free(p);
	asn1_integer_free(q);
	asn1_integer_free(g);
	asn1_bit_string_free(pubkey);
	free(pubkey_data);
	return ret;
}

DER_DEF(dsa_pubkey, dsa);
PEM_DEF_RPWC(dsa_pubkey, dsa, "PUBLIC KEY");

struct dsa_sig *a2i_dsa_sig(const struct asn1_object *object,
                            struct dsa_sig **sigp)
{
	struct dsa_sig *sig = NULL;
	int allocated = 0;

	if (!get_dsa_sig(&sig, sigp, &allocated))
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
		dsa_sig_free(sig);
	return NULL;
}

struct asn1_object *i2a_dsa_sig(const struct dsa_sig *sig)
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
