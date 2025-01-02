#include "pkcs1/pkcs1.h"
#include "asn1/asn1.h"
#include "oid/oid.h"
#include "rsa/rsa.h"

#include <jkssl/bignum.h>
#include <jkssl/rsa.h>
#include <jkssl/der.h>
#include <jkssl/evp.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int rsa_enc_verify(struct rsa *rsa, uint8_t *dst, const uint8_t *src,
                          size_t len)
{
	struct bignum_ctx *bn_ctx = NULL;
	struct bignum *m = NULL;
	struct bignum *c = NULL;
	int ret = -1;

	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		goto end;
	m = bignum_ctx_get(bn_ctx);
	c = bignum_ctx_get(bn_ctx);
	if (!bn_ctx
	 || !bignum_bin2bignum(src, len, m)
	 || bignum_cmp(m, rsa->n) >= 0
	 || !bignum_mod_exp(c, m, rsa->e, rsa->n, bn_ctx)
	 || !bignum_bignum2bin(c, dst))
		goto end;
	ret = bignum_num_bytes(c);

end:
	bignum_ctx_release(bn_ctx, m);
	bignum_ctx_release(bn_ctx, c);
	bignum_ctx_free(bn_ctx);
	return ret;
}

static int rsa_dec_sign(struct rsa *rsa, uint8_t *dst, const uint8_t *src,
                        size_t len)
{
	struct bignum_ctx *bn_ctx = NULL;
	struct bignum *tmp = NULL;
	struct bignum *m = NULL;
	struct bignum *c = NULL;
	int ret = -1;

	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		goto end;
	tmp = bignum_ctx_get(bn_ctx);
	m = bignum_ctx_get(bn_ctx);
	c = bignum_ctx_get(bn_ctx);
	if (!tmp
	 || !m
	 || !c
	 || !bignum_bin2bignum(src, len, c)
	 || bignum_cmp(c, rsa->n) >= 0
	 || !bignum_mod_exp(m, c, rsa->dmp, rsa->p, bn_ctx)
	 || !bignum_mod_exp(tmp, c, rsa->dmq, rsa->q, bn_ctx)
	 || !bignum_sub(m, m, tmp, bn_ctx)
	 || !bignum_mul(m, m, rsa->coef, bn_ctx)
	 || !bignum_mod(m, m, rsa->p, bn_ctx)
	 || !bignum_mul(m, m, rsa->q, bn_ctx)
	 || !bignum_add(m, m, tmp, bn_ctx)
	 || !bignum_bignum2bin(m, dst))
		goto end;
	ret = bignum_num_bytes(m);

end:
	bignum_ctx_release(bn_ctx, tmp);
	bignum_ctx_release(bn_ctx, m);
	bignum_ctx_release(bn_ctx, c);
	bignum_ctx_free(bn_ctx);
	return ret;
}

static int do_pad(struct rsa *rsa, const uint8_t *src, size_t len,
                  int padding, int enc_sign, uint8_t **padded,
                  size_t *padded_len)
{
	switch (padding)
	{
		case RSA_PKCS1_PADDING:
		{
			ssize_t tmp = rsa_size(rsa);
			if (tmp == -1)
				return 0;
			*padded_len = tmp;
			*padded = malloc(*padded_len);
			if (!*padded)
				return 0;
			if ((enc_sign && !pkcs1_pad_1(*padded, *padded_len, src, len))
			 || (!enc_sign && !pkcs1_pad_2(*padded, *padded_len, src, len)))
			{
				free(*padded);
				return 0;
			}
			return 1;
		}
		case RSA_PKCS1_OAEP_PADDING:
			/* XXX */
			return 0;
		default:
			return 0;
	}
}

static int do_unpad(struct rsa *rsa, uint8_t *dst, const uint8_t *src,
                    size_t len, int padding)
{
	switch (padding)
	{
		case RSA_PKCS1_PADDING:
		{
			ssize_t tmp = rsa_size(rsa);
			if (tmp == -1)
				return -1;
			size_t outlen = tmp;
			if (!pkcs1_unpad(dst, &outlen, src, len))
				return -1;
			return outlen;
		}
		case RSA_PKCS1_OAEP_PADDING:
			/* XXX */
			return -1;
		default:
			return -1;
	}
}

int rsa_enc(struct rsa *rsa, uint8_t *dst, const uint8_t *src,
            size_t len, int padding)
{
	return rsa_public_encrypt(len, src, dst, rsa, padding);
}

int rsa_dec(struct rsa *rsa, uint8_t *dst, const uint8_t *src,
            size_t len, int padding)
{
	return rsa_private_decrypt(len, src, dst, rsa, padding);
}

static const uint32_t *get_oid(const struct evp_md *md, size_t *len)
{
#define TEST_OID(evp_md, oid) \
	if (md == evp_md()) \
	{ \
		*len = sizeof(oid) / sizeof(*oid); \
		return oid; \
	}

	TEST_OID(evp_sha1, oid_sha1);
	TEST_OID(evp_sha256, oid_sha256);
	TEST_OID(evp_sha384, oid_sha384);
	TEST_OID(evp_sha512, oid_sha512);
	TEST_OID(evp_sha512_224, oid_sha512_224);
	TEST_OID(evp_sha512_256, oid_sha512_256);
	TEST_OID(evp_md2, oid_md2);
	TEST_OID(evp_md4, oid_md4);
	TEST_OID(evp_md5, oid_md5);
	TEST_OID(evp_ripemd160, oid_ripemd160);
	TEST_OID(evp_ripemd128, oid_ripemd128);
	TEST_OID(evp_ripemd256, oid_ripemd256);
	return NULL;

#undef TEST_OID
}

static int pkcs1_pad(uint8_t *out, size_t *outlen, const uint8_t *in,
                     size_t inlen, const struct evp_md *md)
{
	struct asn1_constructed *sequence = asn1_sequence_new();
	struct asn1_constructed *parameters = asn1_sequence_new();
	struct asn1_oid *oid = asn1_oid_new();
	struct asn1_null *null = asn1_null_new();
	struct asn1_octet_string *str = asn1_octet_string_new();
	size_t oid_len = 0;
	const uint32_t *oid_data = get_oid(md, &oid_len);
	uint8_t *der = NULL;
	int ret = 0;
	if (!sequence
	 || !parameters
	 || !oid
	 || !null
	 || !str
	 || !oid_data
	 || !asn1_oid_set1(oid, oid_data, oid_len)
	 || !asn1_octet_string_set1(str, in, inlen)
	 || !asn1_constructed_add1(parameters, (struct asn1_object*)oid)
	 || !asn1_constructed_add1(parameters, (struct asn1_object*)null)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)parameters)
	 || !asn1_constructed_add1(sequence, (struct asn1_object*)str))
		goto end;
	ret = i2d_asn1_object((struct asn1_object*)sequence, &der);
	if (ret <= 0)
	{
		ret = 0;
		goto end;
	}
	if ((size_t)ret > *outlen)
	{
		ret = 0;
		goto end;
	}
	memcpy(out, der, ret);
	*outlen = ret;
	ret = 1;

end:
	asn1_octet_string_free(str);
	asn1_null_free(null);
	asn1_oid_free(oid);
	asn1_constructed_free(sequence);
	asn1_constructed_free(parameters);
	free(der);
	return ret;
}

static int pkcs1_verify(const uint8_t *tbs, size_t tbslen,
                        const uint8_t *sig, size_t siglen,
                        const struct evp_md *md)
{
	struct asn1_object *object = NULL;
	int ret = 0;

	if (!d2i_asn1_object(&object, (uint8_t**)&sig, siglen)
	 || !ASN1_IS_UNIVERSAL_TYPE(object, ASN1_SEQUENCE))
		goto end;
	struct asn1_constructed *sequence = (struct asn1_constructed*)object;
	if (sequence->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[0], ASN1_SEQUENCE)
	 || !ASN1_IS_UNIVERSAL_TYPE(sequence->objects[1], ASN1_OCTET_STRING))
		goto end;
	struct asn1_constructed *parameters = (struct asn1_constructed*)sequence->objects[0];
	struct asn1_octet_string *str = (struct asn1_octet_string*)sequence->objects[1];
	if (parameters->objects_count != 2
	 || !ASN1_IS_UNIVERSAL_TYPE(parameters->objects[0], ASN1_OBJECT_IDENTIFIER)
	 || !ASN1_IS_UNIVERSAL_TYPE(parameters->objects[1], ASN1_NULL))
		goto end;
	struct asn1_oid *oid = (struct asn1_oid*)parameters->objects[0];
	size_t oid_len = 0;
	const uint32_t *oid_data = get_oid(md, &oid_len);
	if (!oid_data
	 || asn1_oid_cmp_uint32(oid, oid_data, oid_len))
		goto end;
	if (str->size != tbslen
	 || memcmp(tbs, str->data, tbslen))
		goto end;
	ret = 1;

end:
	asn1_object_free(object);
	return ret;
}

int rsa_sign(const struct evp_md *md, struct rsa *rsa,
             const uint8_t *tbs, size_t tbslen,
             uint8_t *sig, size_t *siglen)
{
	ssize_t size = rsa_size(rsa);
	if (size == -1)
		return 0;
	size_t tmplen = size;
	uint8_t *tmp = malloc(size);
	if (!tmp)
		return 0;
	if (!pkcs1_pad(tmp, &tmplen, tbs, tbslen, md))
	{
		free(tmp);
		return 0;
	}
	int ret = rsa_private_encrypt(tmplen, tmp, sig, rsa, RSA_PKCS1_PADDING);
	free(tmp);
	if (ret == -1)
		return 0;
	*siglen = ret;
	return 1;
}

int rsa_verify(const struct evp_md *md, struct rsa *rsa,
               const uint8_t *tbs, size_t tbslen,
               const uint8_t *sig, size_t siglen)
{
	ssize_t size = rsa_size(rsa);
	if (size == -1)
		return 0;
	uint8_t *tmp = malloc(size);
	if (!tmp)
		return 0;
	int ret = rsa_public_decrypt(siglen, sig, tmp, rsa, RSA_PKCS1_PADDING);
	if (ret == -1)
	{
		ret = 0;
		goto end;
	}
	ret = pkcs1_verify(tbs, tbslen, tmp, ret, md);

end:
	free(tmp);
	return ret;
}

int rsa_public_encrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                       struct rsa *rsa, int padding)
{
	if (padding == RSA_NO_PADDING)
		return rsa_enc_verify(rsa, out, in, inlen);
	uint8_t *padded;
	size_t padded_len;
	if (!do_pad(rsa, in, inlen, padding, 0, &padded, &padded_len))
		return -1;
	int ret = rsa_enc_verify(rsa, out, padded, padded_len);
	free(padded);
	return ret;
}

int rsa_private_decrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                        struct rsa *rsa, int padding)
{
	if (padding == RSA_NO_PADDING)
		return rsa_dec_sign(rsa, out, in, inlen);
	ssize_t tmp = rsa_size(rsa);
	if (tmp == -1)
		return -1;
	uint8_t *padded = malloc(tmp);
	if (!padded)
		return -1;
	int ret = rsa_dec_sign(rsa, padded, in, inlen);
	if (ret != -1)
		ret = do_unpad(rsa, out, padded, ret, padding);
	free(padded);
	return ret;
}

int rsa_private_encrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                        struct rsa *rsa, int padding)
{
	if (padding == RSA_NO_PADDING)
		return rsa_dec_sign(rsa, out, in, inlen);
	uint8_t *padded;
	size_t padded_len;
	if (!do_pad(rsa, in, inlen, padding, 1, &padded, &padded_len))
		return -1;
	int ret = rsa_dec_sign(rsa, out, padded, padded_len);
	free(padded);
	return ret;
}

int rsa_public_decrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                       struct rsa *rsa, int padding)
{
	ssize_t size = rsa_size(rsa);
	if (size == -1)
		return -1;
	uint8_t *tmp = malloc(size);
	if (!tmp)
		return -1;
	int ret = rsa_enc_verify(rsa, tmp, in, inlen);
	if (ret == -1)
		goto end;
	if (padding == RSA_NO_PADDING)
	{
		memcpy(out, tmp, ret);
		goto end;
	}
	ret = do_unpad(rsa, out, tmp, ret, padding);

end:
	free(tmp);
	return ret;
}

ssize_t rsa_size(struct rsa *rsa)
{
	ssize_t bits = rsa_bits(rsa);
	if (bits == -1)
		return bits;
	return bits / 8;
}

ssize_t rsa_bits(struct rsa *rsa)
{
	if (!rsa || !rsa->n)
		return -1;
	return bignum_num_bits(rsa->n);
}

struct rsa *rsa_new(void)
{
	struct rsa *rsa = calloc(sizeof(*rsa), 1);
	if (!rsa)
		return NULL;
	refcount_init(&rsa->refcount, 1);
	return rsa;
}

void rsa_free(struct rsa *rsa)
{
	if (!rsa)
		return;
	if (refcount_dec(&rsa->refcount))
		return;
	bignum_free(rsa->n);
	bignum_free(rsa->e);
	bignum_free(rsa->q);
	bignum_free(rsa->p);
	bignum_free(rsa->d);
	bignum_free(rsa->phi);
	bignum_free(rsa->dmp);
	bignum_free(rsa->dmq);
	bignum_free(rsa->coef);
	free(rsa);
}

int rsa_up_ref(struct rsa *rsa)
{
	return refcount_inc(&rsa->refcount);
}

void rsa_get0_key(const struct rsa *rsa, const struct bignum **n,
                  const struct bignum **e, const struct bignum **d)
{
	if (n)
		*n = rsa->n;
	if (e)
		*e = rsa->e;
	if (d)
		*d = rsa->d;
}

int rsa_set0_key(struct rsa *rsa, struct bignum *n,
                 struct bignum *e, struct bignum *d)
{
	bignum_free(rsa->n);
	rsa->n = n;
	bignum_free(rsa->e);
	rsa->e = e;
	bignum_free(rsa->d);
	rsa->d = d;
	return 1;
}

void rsa_get0_factors(const struct rsa *rsa, const struct bignum **p,
                      const struct bignum **q)
{
	if (p)
		*p = rsa->p;
	if (q)
		*q = rsa->q;
}

int rsa_set0_factors(struct rsa *rsa, struct bignum *p, struct bignum *q)
{
	bignum_free(rsa->p);
	rsa->p = p;
	bignum_free(rsa->q);
	rsa->q = q;
	return 1;
}

void rsa_get0_crt_params(const struct rsa *rsa, const struct bignum **dmp,
                         const struct bignum **dmq, const struct bignum **coef)
{
	if (dmp)
		*dmp = rsa->dmp;
	if (dmq)
		*dmq = rsa->dmq;
	if (coef)
		*coef = rsa->coef;
}

int rsa_set0_crt_params(struct rsa *rsa, struct bignum *dmp,
                        struct bignum *dmq, struct bignum *coef)
{
	if (!dmp || !dmq || !coef)
		return 0;
	bignum_free(rsa->dmp);
	rsa->dmp = dmp;
	bignum_free(rsa->dmq);
	rsa->dmq = dmq;
	bignum_free(rsa->coef);
	rsa->coef = coef;
	return 1;
}

const struct bignum *rsa_get0_n(const struct rsa *rsa)
{
	return rsa->n;
}

const struct bignum *rsa_get0_e(const struct rsa *rsa)
{
	return rsa->e;
}

const struct bignum *rsa_get0_d(const struct rsa *rsa)
{
	return rsa->d;
}

const struct bignum *rsa_get0_p(const struct rsa *rsa)
{
	return rsa->p;
}

const struct bignum *rsa_get0_q(const struct rsa *rsa)
{
	return rsa->q;
}

const struct bignum *rsa_get0_dmp1(const struct rsa *rsa)
{
	return rsa->dmp;
}

const struct bignum *rsa_get0_dmq1(const struct rsa *rsa)
{
	return rsa->dmq;
}

const struct bignum *rsa_get0_iqmp(const struct rsa *rsa)
{
	return rsa->coef;
}

int rsa_check_key(const struct rsa *rsa)
{
	struct bignum_ctx *bn_ctx = NULL;
	struct bignum *one = NULL;
	struct bignum *tmp = NULL;
	int ret = -1;

	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		goto end;
	one = bignum_ctx_get(bn_ctx);
	tmp = bignum_ctx_get(bn_ctx);
	if (!one
	 || !tmp
	 || !bignum_one(one))
		goto end;
	if (bignum_is_one(rsa->e)
	 || !bignum_is_bit_set(rsa->e, 0))
	{
		ret = 0;
		goto end;
	}
	if (!rsa->p || !rsa->q)
	{
		ret = 1;
		goto end;
	}
	if (rsa->p)
	{
		switch (bignum_check_prime(rsa->p, bn_ctx, NULL))
		{
			case -1:
				goto end;
			case 0:
				ret = 0;
				goto end;
		}
	}
	if (rsa->q)
	{
		switch (bignum_check_prime(rsa->q, bn_ctx, NULL))
		{
			case -1:
				goto end;
			case 0:
				ret = 0;
				goto end;
		}
	}
	if (!bignum_mul(tmp, rsa->p, rsa->q, bn_ctx))
		goto end;
	if (bignum_cmp(tmp, rsa->n))
	{
		ret = 0;
		goto end;
	}
	if (rsa->dmp)
	{
		if (!bignum_sub(tmp, rsa->p, one, bn_ctx)
		 || !bignum_mod(tmp, rsa->d, tmp, bn_ctx))
			goto end;
		if (bignum_cmp(tmp, rsa->dmp))
		{
			ret = 0;
			goto end;
		}
	}
	if (rsa->dmq)
	{
		if (!bignum_sub(tmp, rsa->q, one, bn_ctx)
		 || !bignum_mod(tmp, rsa->d, tmp, bn_ctx))
			goto end;
		if (bignum_cmp(tmp, rsa->dmq))
		{
			ret = 0;
			goto end;
		}
	}
	if (!bignum_mod_inverse(tmp, rsa->q, rsa->p, bn_ctx))
		goto end;
	if (bignum_cmp(tmp, rsa->coef))
	{
		ret = 0;
		goto end;
	}
	ret = 1;

end:
	bignum_ctx_release(bn_ctx, one);
	bignum_ctx_release(bn_ctx, tmp);
	bignum_ctx_free(bn_ctx);
	return ret;
}
