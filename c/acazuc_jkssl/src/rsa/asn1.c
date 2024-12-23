#include "asn1/asn1.h"
#include "der/der.h"
#include "pem/pem.h"
#include "oid/oid.h"
#include "rsa/rsa.h"

#include <jkssl/rsa.h>
#include <jkssl/bio.h>

#include <stdlib.h>
#include <string.h>

ASN1_OBJ_ALLOC_DEF(rsa, struct rsa, rsa_new);

struct rsa *a2i_rsa_private_key(const struct asn1_object *object,
                                struct rsa **rsap)
{
	struct rsa *rsa = NULL;
	int allocated = 0;

	if (!get_rsa(&rsa, rsap, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 9
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[3], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[4], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[5], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[6], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[7], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[8], ASN1_INTEGER))
		goto err;
	struct asn1_integer *version = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *n = (struct asn1_integer*)sequence->objects[1];
	struct asn1_integer *e = (struct asn1_integer*)sequence->objects[2];
	struct asn1_integer *d = (struct asn1_integer*)sequence->objects[3];
	struct asn1_integer *p = (struct asn1_integer*)sequence->objects[4];
	struct asn1_integer *q = (struct asn1_integer*)sequence->objects[5];
	struct asn1_integer *dmp = (struct asn1_integer*)sequence->objects[6];
	struct asn1_integer *dmq = (struct asn1_integer*)sequence->objects[7];
	struct asn1_integer *coef = (struct asn1_integer*)sequence->objects[8];
	uint64_t v;
	if (!asn1_integer_get_uint64(version, &v)
	 || v
	 || !asn1_integer_get_bignum(n, &rsa->n)
	 || !asn1_integer_get_bignum(e, &rsa->e)
	 || !asn1_integer_get_bignum(d, &rsa->d)
	 || !asn1_integer_get_bignum(p, &rsa->p)
	 || !asn1_integer_get_bignum(q, &rsa->q)
	 || !asn1_integer_get_bignum(dmp, &rsa->dmp)
	 || !asn1_integer_get_bignum(dmq, &rsa->dmq)
	 || !asn1_integer_get_bignum(coef, &rsa->coef))
		goto err;
	if (rsap)
		*rsap = rsa;
	return rsa;

err:
	if (allocated)
		rsa_free(rsa);
	return NULL;
}

struct asn1_object *i2a_rsa_private_key(const struct rsa *rsa)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *version = asn1_integer_new();
	struct asn1_integer *n = asn1_integer_new();
	struct asn1_integer *e = asn1_integer_new();
	struct asn1_integer *d = asn1_integer_new();
	struct asn1_integer *p = asn1_integer_new();
	struct asn1_integer *q = asn1_integer_new();
	struct asn1_integer *dmp = asn1_integer_new();
	struct asn1_integer *dmq = asn1_integer_new();
	struct asn1_integer *coef = asn1_integer_new();
	struct asn1_object *ret = NULL;

	if (!sequence
	 || !version
	 || !n
	 || !e
	 || !d
	 || !p
	 || !q
	 || !dmp
	 || !dmq
	 || !coef
	 || !asn1_integer_set_uint64(version, 0)
	 || !asn1_integer_set_bignum(n, rsa->n)
	 || !asn1_integer_set_bignum(e, rsa->e)
	 || !asn1_integer_set_bignum(d, rsa->d)
	 || !asn1_integer_set_bignum(p, rsa->p)
	 || !asn1_integer_set_bignum(q, rsa->q)
	 || !asn1_integer_set_bignum(dmp, rsa->dmp)
	 || !asn1_integer_set_bignum(dmq, rsa->dmq)
	 || !asn1_integer_set_bignum(coef, rsa->coef)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)version)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)n)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)e)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)d)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)p)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)q)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)dmp)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)dmq)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)coef))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(version);
	asn1_integer_free(n);
	asn1_integer_free(e);
	asn1_integer_free(d);
	asn1_integer_free(p);
	asn1_integer_free(q);
	asn1_integer_free(dmp);
	asn1_integer_free(dmq);
	asn1_integer_free(coef);
	return ret;
}

DER_DEF(rsa_private_key, rsa);
PEM_DEF_RPWP(rsa_private_key, rsa, "RSA PRIVATE KEY");

struct rsa *a2i_rsa_public_key(const struct asn1_object *object,
                               struct rsa **rsap)
{
	struct rsa *rsa = NULL;
	int allocated = 0;

	if (!get_rsa(&rsa, rsap, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER))
		goto err;
	struct asn1_integer *n = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *e = (struct asn1_integer*)sequence->objects[1];
	if (!asn1_integer_get_bignum(n, &rsa->n)
	 || !asn1_integer_get_bignum(e, &rsa->e))
		goto err;
	if (rsap)
		*rsap = rsa;
	return rsa;

err:
	if (allocated)
		rsa_free(rsa);
	return NULL;
}

struct asn1_object *i2a_rsa_public_key(const struct rsa *rsa)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *n = asn1_integer_new();
	struct asn1_integer *e = asn1_integer_new();
	struct asn1_object *ret = NULL;

	if (!sequence
	 || !n
	 || !e
	 || !asn1_integer_set_bignum(n, rsa->n)
	 || !asn1_integer_set_bignum(e, rsa->e)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)n)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)e))
		goto end;
	ret = (struct asn1_object*)sequence;;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(n);
	asn1_integer_free(e);
	return ret;
}

DER_DEF(rsa_public_key, rsa);
PEM_DEF_RPWC(rsa_public_key, rsa, "RSA PUBLIC KEY");

static struct rsa *a2i_pubkey_key(struct rsa **rsap, uint8_t *data, size_t size)
{
	struct asn1_object *object = d2i_asn1_object(&object, &data, size);
	struct rsa *rsa = NULL;
	int allocated = 0;

	if (!get_rsa(&rsa, rsap, &allocated))
		goto err;
	if (!object)
		goto err;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_INTEGER))
		goto err;
	struct asn1_integer *n = (struct asn1_integer*)sequence->objects[0];
	struct asn1_integer *e = (struct asn1_integer*)sequence->objects[1];
	if (!asn1_integer_get_bignum(n, &rsa->n)
	 || !asn1_integer_get_bignum(e, &rsa->e))
		goto err;
	if (rsap)
		*rsap = rsa;
	asn1_object_free(object);
	return rsa;

err:
	if (allocated)
		rsa_free(rsa);
	asn1_object_free(object);
	return NULL;
}

struct rsa *a2i_rsa_pubkey(const struct asn1_object *object,
                           struct rsa **rsap)
{
	struct asn1_oid *algorithm;
	struct asn1_object *parameters;
	struct asn1_bit_string *key_data;

	if (!asn1_decode_spki(object, &algorithm, &parameters, &key_data))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(parameters, ASN1_NULL))
		return NULL;
	if (asn1_oid_cmp_uint32(algorithm, oid_rsaEncryption,
	                        sizeof(oid_rsaEncryption) / sizeof(*oid_rsaEncryption)))
		return NULL;
	return a2i_pubkey_key(rsap, key_data->data, key_data->bits / 8);
}

static int i2a_pubkey_key(const struct rsa *rsa, uint8_t **data)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *n = asn1_integer_new();
	struct asn1_integer *e = asn1_integer_new();
	int ret = -1;

	if (!sequence
	 || !n
	 || !e
	 || !asn1_integer_set_bignum(n, rsa->n)
	 || !asn1_integer_set_bignum(e, rsa->e)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)n)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)e))
		goto end;
	ret = i2d_asn1_object((struct asn1_object*)sequence, data);

end:
	asn1_constructed_free(sequence);
	asn1_integer_free(n);
	asn1_integer_free(e);
	return ret;
}

struct asn1_object *i2a_rsa_pubkey(const struct rsa *rsa)
{
	uint8_t *pubkey_data = NULL;
	struct asn1_oid *algorithm = asn1_oid_new();
	struct asn1_null *parameters = asn1_null_new();
	struct asn1_bit_string *pubkey = asn1_bit_string_new();
	struct asn1_object *ret = NULL;
	int key_len = i2a_pubkey_key(rsa, &pubkey_data);

	if (key_len == -1
	 || !algorithm
	 || !parameters
	 || !pubkey
	 || !asn1_bit_string_set1(pubkey, pubkey_data, key_len * 8)
	 || !asn1_oid_set1(algorithm, oid_rsaEncryption,
	                   sizeof(oid_rsaEncryption) / sizeof(*oid_rsaEncryption)))
		goto end;
	ret = asn1_encode_spki(algorithm, (struct asn1_object*)parameters,
	                       pubkey);

end:
	asn1_bit_string_free(pubkey);
	asn1_oid_free(algorithm);
	asn1_null_free(parameters);
	free(pubkey_data);
	return ret;
}

DER_DEF(rsa_pubkey, rsa);
PEM_DEF_RPWC(rsa_pubkey, rsa, "PUBLIC KEY");
