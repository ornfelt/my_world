#include "asn1/asn1.h"
#include "evp/pkey.h"
#include "der/der.h"
#include "oid/oid.h"

#include <jkssl/pkcs8.h>
#include <jkssl/bio.h>
#include <jkssl/dsa.h>
#include <jkssl/rsa.h>
#include <jkssl/ec.h>

ASN1_OBJ_ALLOC_DEF(evp_pkey, struct evp_pkey, evp_pkey_new);

static struct evp_pkey *a2i_private_key(const struct asn1_object *object,
                                        struct evp_pkey **pkey)
{
	return a2i_pkcs8_private_key_info(object, pkey);
}

static struct asn1_object *i2a_private_key(const struct evp_pkey *pkey)
{
	switch (pkey->type)
	{
		case EVP_PKEY_RSA:
			return i2a_rsa_private_key(pkey->rsa);
		case EVP_PKEY_DSA:
			return i2a_dsa_private_key(pkey->dsa);
		case EVP_PKEY_EC:
			return i2a_ec_private_key(pkey->ec);
		default:
			return i2a_pkcs8_private_key_info(pkey);
	}
}

DER_DEF(private_key, evp_pkey);

static struct evp_pkey *a2i_private_key_traditional(const struct asn1_object *object,
                                                    struct evp_pkey **pkeyp)
{
	struct evp_pkey *pkey = NULL;
	int allocated = 0;
	struct ec_key *ec_key;
	struct rsa *rsa;
	struct dsa *dsa;

	rsa = a2i_rsa_private_key(object, NULL);
	if (rsa)
	{
		if (!get_evp_pkey(&pkey, pkeyp, &allocated)
		 || !evp_pkey_assign_rsa(pkey, rsa))
			goto err;
	}
	else
	{
		dsa = a2i_dsa_private_key(object, NULL);
		if (dsa)
		{
			if (!get_evp_pkey(&pkey, pkeyp, &allocated)
			 || !evp_pkey_assign_dsa(pkey, dsa))
				goto err;
		}
		else
		{
			ec_key = a2i_ec_private_key(object, NULL);
			if (ec_key)
			{
				if (!get_evp_pkey(&pkey, pkeyp, &allocated)
				 || !evp_pkey_assign_ec_key(pkey, ec_key))
					goto err;
			}
			else
			{
				goto err;
			}
		}
	}
	if (pkeyp)
		*pkeyp = pkey;
	return pkey;

err:
	if (allocated)
		evp_pkey_free(pkey);
	return NULL;
}

static struct asn1_object *i2a_private_key_traditional(const struct evp_pkey *pkey)
{
	switch (pkey->type)
	{
		case EVP_PKEY_NONE:
		case EVP_PKEY_DH:
			return NULL;
		case EVP_PKEY_RSA:
			return i2a_rsa_private_key(pkey->rsa);
		case EVP_PKEY_DSA:
			return i2a_dsa_private_key(pkey->dsa);
		case EVP_PKEY_EC:
			return i2a_ec_private_key(pkey->ec);
	}
	return NULL;
}

DER_DEF(private_key_traditional, evp_pkey);

int asn1_decode_spki(const struct asn1_object *object,
                     struct asn1_oid **algorithm,
                     struct asn1_object **parameters,
                     struct asn1_bit_string **pubkey)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return 0;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_BIT_STRING))
		return 0;
	struct asn1_constructed *alg_seq = (struct asn1_constructed*)sequence->objects[0];
	struct asn1_bit_string *key_data = (struct asn1_bit_string*)sequence->objects[1];
	if (alg_seq->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(alg_seq->objects[0], ASN1_OBJECT_IDENTIFIER)
	 || key_data->bits % 8)
		return 0;
	*algorithm = (struct asn1_oid*)alg_seq->objects[0];
	*parameters = alg_seq->objects[1];
	*pubkey = key_data;
	return 1;
}

struct asn1_object *asn1_encode_spki(struct asn1_oid *algorithm,
                                     struct asn1_object *parameters,
                                     struct asn1_bit_string *pubkey)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_constructed *alg_seq = asn1_sequence_new();
	struct asn1_object *ret = NULL;

	if (!sequence
	 || !alg_seq
	 || !asn1_constructed_add1(alg_seq, (struct asn1_object*)algorithm)
	 || !asn1_constructed_add1(alg_seq, parameters)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)alg_seq)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)pubkey))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_constructed_free(alg_seq);
	return ret;
}
