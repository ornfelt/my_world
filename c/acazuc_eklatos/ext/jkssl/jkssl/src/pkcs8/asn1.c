#include "evp/cipher.h"
#include "asn1/asn1.h"
#include "evp/pkey.h"
#include "der/der.h"
#include "oid/oid.h"
#include "pem/pem.h"

#include <jkssl/pkcs8.h>
#include <jkssl/rand.h>
#include <jkssl/bio.h>
#include <jkssl/rsa.h>
#include <jkssl/dsa.h>
#include <jkssl/ec.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>

ASN1_OBJ_ALLOC_DEF(evp_pkey, struct evp_pkey, evp_pkey_new);

struct evp_pkey *a2i_pkcs8_private_key_info(const struct asn1_object *object,
                                            struct evp_pkey **pkeyp)
{
	struct evp_pkey *pkey = NULL;
	int allocated = 0;

	if (!get_evp_pkey(&pkey, pkeyp, &allocated))
		return NULL;
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto err;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 3
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[2], ASN1_OCTET_STRING))
		goto err;
	struct asn1_integer *version = (struct asn1_integer*)sequence->objects[0];
	struct asn1_constructed *algorithm = (struct asn1_constructed*)sequence->objects[1];
	struct asn1_octet_string *private_key = (struct asn1_octet_string*)sequence->objects[2];
	uint64_t v;
	if (!asn1_integer_get_uint64(version, &v)
	 || v
	 || algorithm->objects_count < 1
	 || !ASN1_IS_UNIVERSAL_TYPE(algorithm->objects[0], ASN1_OBJECT_IDENTIFIER))
		goto err;
	struct asn1_oid *oid = (struct asn1_oid*)algorithm->objects[0];
	if (!asn1_oid_cmp_uint32(oid, oid_dsa,
	                         sizeof(oid_dsa) / sizeof(*oid_dsa)))
	{
		if (algorithm->objects_count != 2)
			goto err;
		struct asn1_object *params = algorithm->objects[1];
		struct dsa *dsa = dsa_new();
		if (!dsa)
			goto err;
		uint8_t *pkey_data = private_key->data;
		if (!a2i_dsa_params(params, &dsa)
		 || !d2i_dsa_private_key(&dsa, &pkey_data, private_key->size)
		 || !evp_pkey_assign_dsa(pkey, dsa))
		{
			dsa_free(dsa);
			goto err;
		}
	}
	else if (!asn1_oid_cmp_uint32(oid, oid_rsaEncryption,
	                              sizeof(oid_rsaEncryption) / sizeof(*oid_rsaEncryption)))
	{
		if (algorithm->objects_count > 2
		 || (algorithm->objects_count == 2 && !ASN1_IS_UNIVERSAL_TYPE(algorithm->objects[1], ASN1_NULL)))
			goto err;
		struct rsa *rsa = rsa_new();
		if (!rsa)
			goto err;
		uint8_t *pkey_data = private_key->data;
		if (!d2i_rsa_private_key(&rsa, &pkey_data, private_key->size)
		 || !evp_pkey_assign_rsa(pkey, rsa))
		{
			rsa_free(rsa);
			goto err;
		}
	}
	else if (!asn1_oid_cmp_uint32(oid, oid_ecPublicKey,
	                              sizeof(oid_ecPublicKey) / sizeof(*oid_ecPublicKey)))
	{
		if (algorithm->objects_count != 2)
			goto err;
		struct asn1_object *params = algorithm->objects[1];
		struct ec_group *group = ec_group_new();
		if (!group)
			goto err;
		if (!a2i_ecpk_parameters(params, &group))
		{
			ec_group_free(group);
			goto err;
		}
		struct ec_key *ec = ec_key_new();
		if (!ec)
		{
			ec_group_free(group);
			goto err;
		}
		uint8_t *pkey_data = private_key->data;
		if (!d2i_ec_private_key(&ec, &pkey_data, private_key->size))
		{
			ec_group_free(group);
			ec_key_free(ec);
			goto err;
		}
		ec_key_set_group(ec, group);
		ec_group_free(group);
		evp_pkey_assign_ec_key(pkey, ec);
	}
	else
	{
		goto err;
	}
	if (pkeyp)
		*pkeyp = pkey;
	return pkey;

err:
	if (allocated)
		evp_pkey_free(pkey);
	return NULL;
}

struct asn1_object *i2a_pkcs8_private_key_info(const struct evp_pkey *pkey)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_integer *version = asn1_integer_new();
	struct asn1_constructed *algorithm = asn1_sequence_new();
	struct asn1_oid *oid = asn1_oid_new();
	struct asn1_octet_string *private_key = asn1_octet_string_new();
	struct asn1_object *params = NULL;
	struct asn1_object *ret = NULL;
	uint8_t *pkey_data = NULL;
	int pkey_size = -1;

	if (!oid)
		goto end;
	switch (pkey->type)
	{
		case EVP_PKEY_NONE:
		case EVP_PKEY_DH:
			break;
		case EVP_PKEY_DSA:
			if (!asn1_oid_set1(oid, oid_dsa, sizeof(oid_dsa) / sizeof(*oid_dsa)))
				goto end;
			params = i2a_dsa_params(pkey->dsa);
			pkey_size = i2d_dsa_private_key(pkey->dsa, &pkey_data);
			break;
		case EVP_PKEY_RSA:
			if (!asn1_oid_set1(oid, oid_rsaEncryption, sizeof(oid_rsaEncryption) / sizeof(*oid_rsaEncryption)))
				goto end;
			params = (struct asn1_object*)asn1_null_new();
			pkey_size = i2d_rsa_private_key(pkey->rsa, &pkey_data);
			break;
		case EVP_PKEY_EC:
			if (!asn1_oid_set1(oid, oid_ecPublicKey, sizeof(oid_ecPublicKey) / sizeof(*oid_ecPublicKey)))
				goto end;
			params = i2a_ecpk_parameters((struct ec_group*)ec_key_get0_group(pkey->ec));
			pkey_size = i2d_ec_private_key(pkey->ec, &pkey_data);
			break;
	}
	if (!sequence
	 || !version
	 || !algorithm
	 || !params
	 || !pkey_data
	 || pkey_size < 0
	 || !private_key
	 || !asn1_integer_set_uint64(version, 0)
	 || !asn1_constructed_add1(algorithm, (struct asn1_object*)oid)
	 || !asn1_constructed_add1(algorithm, (struct asn1_object*)params)
	 || !asn1_octet_string_set1(private_key, pkey_data, pkey_size)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)version)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)algorithm)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)private_key))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_integer_free(version);
	asn1_constructed_free(algorithm);
	asn1_oid_free(oid);
	asn1_octet_string_free(private_key);
	asn1_object_free(params);
	free(pkey_data);
	return ret;
}

DER_DEF(pkcs8_private_key_info, evp_pkey);
PEM_DEF_RCWC(pkcs8_private_key_info, evp_pkey, "PRIVATE KEY");

static int get_enc_values(struct asn1_constructed *enc,
                          const struct evp_cipher **cipher,
                          uint8_t **iv)
{
	if (enc->objects_count < 1
	 || !ASN1_IS_UNIVERSAL_TYPE(enc->objects[0], ASN1_OBJECT_IDENTIFIER))
		return 0;
	struct asn1_oid *enc_oid = (struct asn1_oid*)enc->objects[0];
	*cipher = evp_get_cipherbyoid(enc_oid->values, enc_oid->values_count);
	if (!*cipher)
		return 0;
	if (enc->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(enc->objects[1], ASN1_OCTET_STRING))
		return 0;
	struct asn1_octet_string *asn1_iv = (struct asn1_octet_string*)enc->objects[1];
	if (asn1_iv->size != (*cipher)->block_size)
		return 0;
	*iv = malloc((*cipher)->block_size);
	if (!*iv)
		return 0;
	memcpy(*iv, asn1_iv->data, asn1_iv->size);
	return 1;
}

static const struct evp_md *get_hash_evp(struct asn1_oid *oid)
{
	if (!asn1_oid_cmp_uint32(oid, oid_hmacWithMD5,
	                         sizeof(oid_hmacWithMD5) / sizeof(*oid_hmacWithMD5)))
		return evp_md5();
	if (!asn1_oid_cmp_uint32(oid, oid_hmacWithSHA1,
	                         sizeof(oid_hmacWithSHA1) / sizeof(*oid_hmacWithSHA1)))
		return evp_sha1();
	if (!asn1_oid_cmp_uint32(oid, oid_hmacWithSHA224,
	                         sizeof(oid_hmacWithSHA224) / sizeof(*oid_hmacWithSHA224)))
		return evp_sha224();
	if (!asn1_oid_cmp_uint32(oid, oid_hmacWithSHA256,
	                         sizeof(oid_hmacWithSHA256) / sizeof(*oid_hmacWithSHA256)))
		return evp_sha256();
	if (!asn1_oid_cmp_uint32(oid, oid_hmacWithSHA384,
	                         sizeof(oid_hmacWithSHA384) / sizeof(*oid_hmacWithSHA384)))
		return evp_sha384();
	if (!asn1_oid_cmp_uint32(oid, oid_hmacWithSHA512,
	                         sizeof(oid_hmacWithSHA512) / sizeof(*oid_hmacWithSHA512)))
		return evp_sha512();
	if (!asn1_oid_cmp_uint32(oid, oid_hmacWithSHA512_224,
	                         sizeof(oid_hmacWithSHA512_224) / sizeof(*oid_hmacWithSHA512_224)))
		return evp_sha512_224();
	if (!asn1_oid_cmp_uint32(oid, oid_hmacWithSHA512_256,
	                         sizeof(oid_hmacWithSHA512_256) / sizeof(*oid_hmacWithSHA512_256)))
		return evp_sha512_256();
	return NULL;
}

static int get_key(struct asn1_constructed *kdf,
                   const struct evp_cipher *cipher,
                   pem_password_cb *cb,
                   void *userdata,
                   uint8_t **key)
{
	if (kdf->objects_count < 1
	 || !ASN1_IS_UNIVERSAL_TYPE(kdf->objects[0], ASN1_OBJECT_IDENTIFIER))
		return 0;
	struct asn1_oid *kdf_oid = (struct asn1_oid*)kdf->objects[0];
	if (asn1_oid_cmp_uint32(kdf_oid, oid_pkcs5_pbkdf2,
	                        sizeof(oid_pkcs5_pbkdf2) / sizeof(*oid_pkcs5_pbkdf2)))
		return 0;
	if (kdf->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(kdf->objects[1], ASN1_SEQUENCE))
		return 0;
	struct asn1_constructed *params = (struct asn1_constructed*)kdf->objects[1];
	if (params->objects_count != 3
	 || !ASN1_IS_UNIVERSAL_TYPE(params->objects[0], ASN1_OCTET_STRING)
	 || !ASN1_IS_UNIVERSAL_TYPE(params->objects[1], ASN1_INTEGER)
	 || !ASN1_IS_UNIVERSAL_TYPE(params->objects[2], ASN1_SEQUENCE))
		return 0;
	struct asn1_octet_string *salt = (struct asn1_octet_string*)params->objects[0];
	struct asn1_integer *iter = (struct asn1_integer*)params->objects[1];
	struct asn1_constructed *hash = (struct asn1_constructed*)params->objects[2];
	uint64_t iterations;
	if (!asn1_integer_get_uint64(iter, &iterations)
	 || iterations < 1
	 || iterations > INT_MAX
	 || hash->objects_count < 1
	 || !ASN1_IS_UNIVERSAL_TYPE(hash->objects[0], ASN1_OBJECT_IDENTIFIER))
		return 0;
	char *password;
	char buf[512];
	if (cb)
	{
		if (cb(buf, sizeof(buf), 0, userdata) < 0)
			return 0;
		password = buf;
	}
	else if (userdata)
	{
		password = userdata;
	}
	else
	{
		return 0;
	}
	struct asn1_oid *hash_oid = (struct asn1_oid*)hash->objects[0];
	const struct evp_md *evp_md = get_hash_evp(hash_oid);
	if (!evp_md)
		return 0;
	size_t key_size = cipher->key_size;
	*key = malloc(key_size);
	if (!*key)
		return 0;
	if (!pkcs5_pbkdf2_hmac(password, strlen(password), salt->data, salt->size,
	                       iterations, evp_md, key_size, *key))
	{
		free(*key);
		*key = NULL;
		return 0;
	}
	return 1;
}

static uint8_t *decipher_pkey(const struct evp_cipher *cipher,
                              uint8_t *key,
                              uint8_t *iv,
                              struct asn1_octet_string *pkey_data,
                              size_t *pki_size)
{
	struct evp_cipher_ctx *cipher_ctx = NULL;
	uint8_t *ret = NULL;
	uint8_t *ptr;
	size_t outl;

	*pki_size = 0;
	ret = malloc(pkey_data->size);
	if (!ret)
		goto err;
	cipher_ctx = evp_cipher_ctx_new();
	if (!cipher_ctx)
		goto err;
	if (!evp_cipher_init(cipher_ctx, cipher, key, iv, 0))
		goto err;
	ptr = ret;
	outl = pkey_data->size;
	if (!evp_cipher_update(cipher_ctx, ptr, &outl, pkey_data->data,
	                       pkey_data->size))
		goto err;
	*pki_size += outl;
	ptr += outl;
	outl = pkey_data->size - outl;
	if (!evp_cipher_final(cipher_ctx, ptr, &outl))
		goto err;
	*pki_size += outl;
	evp_cipher_ctx_free(cipher_ctx);
	return ret;

err:
	evp_cipher_ctx_free(cipher_ctx);
	free(ret);
	return NULL;
}

struct evp_pkey *a2i_pkcs8_private_key(const struct asn1_object *object,
                                       struct evp_pkey **pkeyp,
                                       pem_password_cb *cb,
                                       void *userdata)
{
	if (!ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		return NULL;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_OCTET_STRING))
		return NULL;
	struct asn1_constructed *encryption_algorithm = (struct asn1_constructed*)sequence->objects[0];
	struct asn1_octet_string *pkey_data = (struct asn1_octet_string*)sequence->objects[1];
	if (encryption_algorithm->objects_count < 1
	 || !ASN1_IS_UNIVERSAL_TYPE(encryption_algorithm->objects[0], ASN1_OBJECT_IDENTIFIER))
		return NULL;
	struct asn1_oid *algorithm = (struct asn1_oid*)encryption_algorithm->objects[0];
	if (!asn1_oid_cmp_uint32(algorithm, oid_pkcs5_pbes2,
	                         sizeof(oid_pkcs5_pbes2) / sizeof(*oid_pkcs5_pbes2)))
	{
		if (encryption_algorithm->objects_count != 2
		 || !ASN1_IS_UNIVERSAL_TYPE(encryption_algorithm->objects[1], ASN1_SEQUENCE))
			return NULL;
		struct asn1_constructed *params = (struct asn1_constructed*)encryption_algorithm->objects[1];
		if (params->objects_count != 2
		 || !ASN1_IS_UNIVERSAL_TYPE(params->objects[0], ASN1_SEQUENCE)
		 || !ASN1_IS_UNIVERSAL_TYPE(params->objects[1], ASN1_SEQUENCE))
			return NULL;
		struct asn1_constructed *kdf = (struct asn1_constructed*)params->objects[0];
		struct asn1_constructed *enc = (struct asn1_constructed*)params->objects[1];
		const struct evp_cipher *cipher;
		uint8_t *iv = NULL;
		uint8_t *key = NULL;
		if (!get_enc_values(enc, &cipher, &iv)
		 || !get_key(kdf, cipher, cb, userdata, &key))
		{
			free(key);
			free(iv);
			return NULL;
		}
		size_t pki_size;
		uint8_t *pki_data = decipher_pkey(cipher, key, iv, pkey_data,
		                                  &pki_size);
		free(iv);
		free(key);
		if (!pki_data)
			return NULL;
		uint8_t *tmp = pki_data;
		struct evp_pkey *ret = d2i_pkcs8_private_key_info(pkeyp, &tmp, pki_size);
		free(pki_data);
		return ret;
	}
	return NULL;
}

static int cipher_key(const struct evp_pkey *pkey,
                      const struct evp_cipher *cipher,
                      const uint8_t *salt,
                      size_t saltlen,
                      const uint8_t *iv,
                      pem_password_cb *cb,
                      void *userdata,
                      int iterations,
                      const struct evp_md *evp_md,
                      uint8_t **datap,
                      size_t *sizep)
{
	struct evp_cipher_ctx *cipher_ctx = NULL;
	uint8_t key[EVP_MAX_KEY_LENGTH];
	size_t key_size = cipher->key_size;
	char *password;
	uint8_t *ciphered_data = NULL;
	uint8_t *data = NULL;
	uint8_t *ptr;
	size_t outl;
	int ciphered_size;
	int data_size;
	int ret = 0;
	char buf[512];

	if (cb)
	{
		if (cb(buf, sizeof(buf), 1, userdata) < 0)
			return 0;
		password = buf;
	}
	else if (userdata)
	{
		password = userdata;
	}
	else
	{
		goto end;
	}
	if (!pkcs5_pbkdf2_hmac(password, strlen(password), salt, saltlen,
	                       iterations, evp_md, key_size, key))
		return 0;
	data_size = i2d_pkcs8_private_key_info(pkey, &data);
	if (!data_size)
		goto end;
	ciphered_size = data_size + cipher->block_size;
	ciphered_data = malloc(ciphered_size);
	if (!ciphered_data)
		goto end;
	cipher_ctx = evp_cipher_ctx_new();
	if (!cipher_ctx)
		goto end;
	if (!evp_cipher_init(cipher_ctx, cipher, key, iv, 1))
		goto end;
	ptr = ciphered_data;
	outl = ciphered_size;
	if (!evp_cipher_update(cipher_ctx, ptr, &outl, data, data_size))
		goto end;
	*sizep = outl;
	ptr += outl;
	outl = ciphered_size - outl;
	if (!evp_cipher_final(cipher_ctx, ptr, &outl))
		goto end;
	*sizep += outl;
	*datap = ciphered_data;
	ret = 1;

end:
	if (!ret)
		free(ciphered_data);
	evp_cipher_ctx_free(cipher_ctx);
	free(data);
	return ret;
}

struct asn1_object *i2a_pkcs8_private_key(const struct evp_pkey *pkey,
                                          const struct evp_cipher *cipher,
                                          pem_password_cb *cb,
                                          void *userdata)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_constructed *algorithm = asn1_sequence_new();
	struct asn1_oid *algorithm_oid = asn1_oid_new();
	struct asn1_constructed *algorithm_params = asn1_sequence_new();
	struct asn1_constructed *kdf = asn1_sequence_new();
	struct asn1_oid *kdf_oid = asn1_oid_new();
	struct asn1_constructed *kdf_params = asn1_sequence_new();
	struct asn1_octet_string *kdf_salt = asn1_octet_string_new();
	struct asn1_integer *kdf_iter = asn1_integer_new();
	struct asn1_constructed *hash = asn1_sequence_new();
	struct asn1_oid *hash_oid = asn1_oid_new();
	struct asn1_null *hash_params = asn1_null_new();
	struct asn1_constructed *enc = asn1_sequence_new();
	struct asn1_oid *enc_oid = asn1_oid_new();
	struct asn1_octet_string *enc_params = asn1_octet_string_new();
	struct asn1_octet_string *encrypted_data = asn1_octet_string_new();
	struct asn1_object *ret = NULL;
	uint8_t *pkey_data = NULL;
	size_t pkey_size;
	uint8_t salt[8];
	uint8_t iv[EVP_MAX_IV_LENGTH];

	if (!cipher
	 || !cipher->oid
	 || !sequence
	 || !algorithm
	 || !algorithm_oid
	 || !algorithm_params
	 || !kdf
	 || !kdf_oid
	 || !kdf_params
	 || !kdf_salt
	 || !kdf_iter
	 || !hash
	 || !hash_oid
	 || !hash_params
	 || !enc
	 || !enc_oid
	 || !enc_params
	 || !encrypted_data
	 || !asn1_oid_set1(hash_oid, oid_hmacWithSHA256, sizeof(oid_hmacWithSHA256) / sizeof(*oid_hmacWithSHA256)) /* XXX */
	 || !asn1_constructed_add1(hash, (struct asn1_object*)hash_oid)
	 || !asn1_constructed_add1(hash, (struct asn1_object*)hash_params)
	 || rand_bytes(salt, sizeof(salt)) != 1
	 || !asn1_octet_string_set1(kdf_salt, salt, sizeof(salt))
	 || !asn1_integer_set_uint64(kdf_iter, 2048) /* XXX */
	 || !asn1_constructed_add1(kdf_params, (struct asn1_object*)kdf_salt)
	 || !asn1_constructed_add1(kdf_params, (struct asn1_object*)kdf_iter)
	 || !asn1_constructed_add1(kdf_params, (struct asn1_object*)hash)
	 || !asn1_oid_set1(kdf_oid, oid_pkcs5_pbkdf2, sizeof(oid_pkcs5_pbkdf2) / sizeof(*oid_pkcs5_pbkdf2))
	 || !asn1_constructed_add1(kdf, (struct asn1_object*)kdf_oid)
	 || !asn1_constructed_add1(kdf, (struct asn1_object*)kdf_params)
	 || !asn1_oid_set1(enc_oid, cipher->oid, cipher->oid_size)
	 || rand_bytes(iv, cipher->block_size) != 1
	 || !asn1_octet_string_set1(enc_params, iv, cipher->block_size)
	 || !asn1_constructed_add1(enc, (struct asn1_object*)enc_oid)
	 || !asn1_constructed_add1(enc, (struct asn1_object*)enc_params)
	 || !asn1_constructed_add1(algorithm_params, (struct asn1_object*)kdf)
	 || !asn1_constructed_add1(algorithm_params, (struct asn1_object*)enc)
	 || !asn1_oid_set1(algorithm_oid, oid_pkcs5_pbes2, sizeof(oid_pkcs5_pbes2) / sizeof(*oid_pkcs5_pbes2))
	 || !asn1_constructed_add1(algorithm, (struct asn1_object*)algorithm_oid)
	 || !asn1_constructed_add1(algorithm, (struct asn1_object*)algorithm_params)
	 || !cipher_key(pkey, cipher, salt, sizeof(salt), iv, cb, userdata, 2048, evp_sha256(), &pkey_data, &pkey_size) /* XXX */
	 || !asn1_octet_string_set1(encrypted_data, pkey_data, pkey_size)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)algorithm)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)encrypted_data))
		goto end;
	ret = (struct asn1_object*)sequence;

end:
	if (!ret)
		asn1_constructed_free(sequence);
	asn1_constructed_free(algorithm);
	asn1_oid_free(algorithm_oid);
	asn1_constructed_free(algorithm_params);
	asn1_constructed_free(kdf);
	asn1_oid_free(kdf_oid);
	asn1_constructed_free(kdf_params);
	asn1_octet_string_free(kdf_salt);
	asn1_integer_free(kdf_iter);
	asn1_constructed_free(hash);
	asn1_oid_free(hash_oid);
	asn1_null_free(hash_params);
	asn1_constructed_free(enc);
	asn1_oid_free(enc_oid);
	asn1_octet_string_free(enc_params);
	asn1_octet_string_free(encrypted_data);
	free(pkey_data);
	return ret;
}

DER_DEF_PASS(pkcs8_private_key, evp_pkey);

struct evp_pkey *pem_read_bio_pkcs8_private_key(struct bio *bio,
                                                struct evp_pkey **pkey,
                                                pem_password_cb *cb,
                                                void *userdata)
{
	while (1)
	{
		struct pem_read_ctx pem_ctx;
		struct evp_pkey *ret = NULL;
		uint8_t *data;
		pem_ctx.bio = bio;
		pem_ctx.password_cb = cb;
		pem_ctx.password_userdata = userdata;
		pem_ctx.text = NULL;
		if (!pem_read(&pem_ctx))
			return NULL;
		data = pem_ctx.data;
		if (!strcmp(pem_ctx.matched_text, "PRIVATE KEY"))
			ret = d2i_pkcs8_private_key_info(pkey, &data, pem_ctx.len);
		else if (!strcmp(pem_ctx.matched_text, "ENCRYPTED PRIVATE KEY"))
			ret = d2i_pkcs8_private_key(pkey, &data, pem_ctx.len, cb, userdata);
		free(pem_ctx.data);
		if (ret)
			return ret;
	}
	return NULL;
}

struct evp_pkey *pem_read_pkcs8_private_key(FILE *fp,
                                            struct evp_pkey **pkey,
                                            pem_password_cb *cb,
                                            void *userdata)
{
	PEM_READ_FP(pkcs8_private_key, struct evp_pkey, pkey, cb, userdata);
}

int pem_write_bio_pkcs8_private_key(struct bio *bio,
                                    const struct evp_pkey *pkey,
                                    const struct evp_cipher *cipher,
                                    pem_password_cb *cb,
                                    void *userdata)
{
	struct pem_write_ctx pem_ctx;
	int ret;
	pem_ctx.bio = bio;
	if (cipher)
	{
		pem_ctx.evp_cipher = NULL;
		pem_ctx.password_cb = NULL;
		pem_ctx.password_userdata = NULL;
		pem_ctx.text = "ENCRYPTED PRIVATE KEY";
		pem_ctx.len = i2d_pkcs8_private_key(pkey, &pem_ctx.data,
		                                    cipher, cb, userdata);
	}
	else
	{
		pem_ctx.evp_cipher = cipher;
		pem_ctx.password_cb = cb;
		pem_ctx.password_userdata = userdata;
		pem_ctx.text = "PRIVATE KEY";
		pem_ctx.len = i2d_pkcs8_private_key_info(pkey, &pem_ctx.data);
	}
	if (pem_ctx.len == (size_t)-1)
		return 0;
	ret = pem_write(&pem_ctx);
	free(pem_ctx.data);
	return ret;
}

int pem_write_pkcs8_private_key(FILE *fp,
                                const struct evp_pkey *pkey,
                                const struct evp_cipher *cipher,
                                pem_password_cb *cb,
                                void *userdata)
{
	PEM_WRITE_FP(pkcs8_private_key, pkey, cipher, cb, userdata);
}
