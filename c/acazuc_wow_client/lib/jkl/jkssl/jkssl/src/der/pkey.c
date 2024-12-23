#include "asn1/asn1.h"
#include "der/der.h"
#include "oid/oid.h"

#include <jkssl/bio.h>
#include <jkssl/dsa.h>
#include <jkssl/evp.h>
#include <jkssl/rsa.h>

static struct evp_pkey *a2i_private_key(const struct asn1_object *object,
                                        struct evp_pkey **pkey)
{
	return NULL;
}

static struct asn1_object *i2a_private_key(struct evp_pkey *pkey)
{
	return NULL;
}

DER_DEF(private_key, evp_pkey);

static struct evp_pkey *a2i_pkcs8_private_key(const struct asn1_object *object,
                                              struct evp_pkey **pkey)
{
	return NULL;
}

static struct asn1_object *i2a_pkcs8_private_key(struct evp_pkey *pkey)
{
	return NULL;
}

DER_DEF(pkcs8_private_key, evp_pkey);

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

static struct evp_pkey *a2i_pubkey(const struct asn1_object *object,
                                   struct evp_pkey **pkeyp)
{
	struct asn1_oid *algorithm;
	struct asn1_object *parameters;
	struct asn1_bit_string *key_data;
	if (!asn1_decode_spki(object, &algorithm, &parameters, &key_data))
		return NULL;
	struct evp_pkey *pkey;
	if (pkeyp && *pkeyp)
	{
		pkey = *pkeyp;
		evp_pkey_up_ref(pkey);
	}
	else
	{
		pkey = evp_pkey_new();
		if (!pkey)
			return NULL;
	}
	if (!asn1_oid_cmp_uint32(algorithm, oid_dsa,
	                         sizeof(oid_dsa) / sizeof(*oid_dsa)))
	{
		struct dsa_ctx *dsa = dsa_new();
		if (!dsa)
			goto err;
		if (!a2i_dsa_pubkey(object, &dsa)
		 || !evp_pkey_assign_dsa(pkey, dsa))
		{
			dsa_free(dsa);
			goto err;
		}
		return pkey;
	}
	if (!asn1_oid_cmp_uint32(algorithm, oid_rsaEncryption,
	                         sizeof(oid_rsaEncryption) / sizeof(*oid_rsaEncryption)))
	{
		struct rsa_ctx *rsa = rsa_new();
		if (!rsa)
			goto err;
		if (!a2i_rsa_pubkey(object, &rsa)
		 || !evp_pkey_assign_rsa(pkey, rsa))
		{
			rsa_free(rsa);
			goto err;
		}
		return pkey;
	}
err:
	evp_pkey_free(pkey);
	return NULL;
}

static struct asn1_object *i2a_pubkey(struct evp_pkey *pkey)
{
	return NULL;
}

DER_DEF(pubkey, evp_pkey);
