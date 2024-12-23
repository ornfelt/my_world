#include "pkcs1.5/pkcs1.5.h"
#include "asn1/asn1.h"
#include "oid/oid.h"

#include <jkssl/bignum.h>
#include <jkssl/rsa.h>
#include <jkssl/der.h>
#include <jkssl/evp.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int rsa_enc_verify(struct rsa_ctx *ctx, uint8_t *dst, const uint8_t *src,
                          size_t len)
{
	struct bignum_ctx *bn_ctx;
	struct bignum m;
	struct bignum c;
	int ret = -1;

	bignum_init(&c);
	bignum_init(&m);
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx
	 || !bignum_bin2bignum(src, len, &m)
	 || bignum_cmp(&m, ctx->n) >= 0
	 || !bignum_mod_exp(&c, &m, ctx->e, ctx->n, bn_ctx)
	 || !bignum_bignum2bin(&c, dst))
		goto end;
	ret = bignum_num_bytes(&c);

end:
	bignum_ctx_free(bn_ctx);
	bignum_clear(&m);
	bignum_clear(&c);
	return ret;
}

static int rsa_dec_sign(struct rsa_ctx *ctx, uint8_t *dst, const uint8_t *src,
                        size_t len)
{
	struct bignum_ctx *bn_ctx;
	struct bignum tmp;
	struct bignum m;
	struct bignum c;
	int ret = -1;

	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
		return -1;
	bignum_init(&tmp);
	bignum_init(&m);
	bignum_init(&c);
	if (!bignum_bin2bignum(src, len, &c)
	 || bignum_cmp(&c, ctx->n) >= 0
	 || !bignum_mod_exp(&m, &c, ctx->dmp, ctx->p, bn_ctx)
	 || !bignum_mod_exp(&tmp, &c, ctx->dmq, ctx->q, bn_ctx)
	 || !bignum_sub(&m, &m, &tmp, bn_ctx)
	 || !bignum_mul(&m, &m, ctx->coef, bn_ctx)
	 || !bignum_mod(&m, &m, ctx->p, bn_ctx)
	 || !bignum_mul(&m, &m, ctx->q, bn_ctx)
	 || !bignum_add(&m, &m, &tmp, bn_ctx)
	 || !bignum_bignum2bin(&m, dst))
		goto end;
	ret = bignum_num_bytes(&m);

end:
	bignum_clear(&tmp);
	bignum_clear(&m);
	bignum_clear(&c);
	bignum_ctx_free(bn_ctx);
	return ret;
}

static int do_pad(struct rsa_ctx *ctx, const uint8_t *src, size_t len,
                  int padding, int enc_sign, uint8_t **padded,
                  size_t *padded_len)
{
	switch (padding)
	{
		case RSA_PKCS1_PADDING:
		{
			ssize_t tmp = rsa_size(ctx);
			if (tmp == -1)
				return 0;
			*padded_len = tmp;
			*padded = malloc(*padded_len);
			if (!*padded)
				return 0;
			if ((enc_sign && !pkcs1_5_pad_1(*padded, *padded_len, src, len))
			 || (!enc_sign && !pkcs1_5_pad_2(*padded, *padded_len, src, len)))
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

static int do_unpad(struct rsa_ctx *ctx, uint8_t *dst, const uint8_t *src,
                    size_t len, int padding)
{
	switch (padding)
	{
		case RSA_PKCS1_PADDING:
		{
			ssize_t tmp = rsa_size(ctx);
			if (tmp == -1)
				return -1;
			size_t outlen = tmp;
			if (!pkcs1_5_unpad(dst, &outlen, src, len))
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

int rsa_enc(struct rsa_ctx *ctx, uint8_t *dst, const uint8_t *src,
            size_t len, int padding)
{
	return rsa_public_encrypt(len, src, dst, ctx, padding);
}

int rsa_dec(struct rsa_ctx *ctx, uint8_t *dst, const uint8_t *src,
            size_t len, int padding)
{
	return rsa_private_decrypt(len, src, dst, ctx, padding);
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

int rsa_sign(const struct evp_md *md, struct rsa_ctx *ctx,
             const uint8_t *tbs, size_t tbslen,
             uint8_t *sig, size_t *siglen)
{
	ssize_t size = rsa_size(ctx);
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
	int ret = rsa_private_encrypt(tmplen, tmp, sig, ctx, RSA_PKCS1_PADDING);
	free(tmp);
	if (ret == -1)
		return 0;
	*siglen = ret;
	return 1;
}

int rsa_verify(const struct evp_md *md, struct rsa_ctx *ctx,
               const uint8_t *tbs, size_t tbslen,
               const uint8_t *sig, size_t siglen)
{
	ssize_t size = rsa_size(ctx);
	if (size == -1)
		return 0;
	uint8_t *tmp = malloc(size);
	if (!tmp)
		return 0;
	int ret = rsa_public_decrypt(siglen, sig, tmp, ctx, RSA_PKCS1_PADDING);
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
                       struct rsa_ctx *ctx, int padding)
{
	if (padding == RSA_NO_PADDING)
		return rsa_enc_verify(ctx, out, in, inlen);
	uint8_t *padded;
	size_t padded_len;
	if (!do_pad(ctx, in, inlen, padding, 0, &padded, &padded_len))
		return -1;
	int ret = rsa_enc_verify(ctx, out, padded, padded_len);
	free(padded);
	return ret;
}

int rsa_private_decrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                        struct rsa_ctx *ctx, int padding)
{
	if (padding == RSA_NO_PADDING)
		return rsa_dec_sign(ctx, out, in, inlen);
	ssize_t tmp = rsa_size(ctx);
	if (tmp == -1)
		return -1;
	uint8_t *padded = malloc(tmp);
	if (!padded)
		return -1;
	int ret = rsa_dec_sign(ctx, padded, in, inlen);
	if (ret != -1)
		ret = do_unpad(ctx, out, padded, ret, padding);
	free(padded);
	return ret;
}

int rsa_private_encrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                        struct rsa_ctx *ctx, int padding)
{
	if (padding == RSA_NO_PADDING)
		return rsa_dec_sign(ctx, out, in, inlen);
	uint8_t *padded;
	size_t padded_len;
	if (!do_pad(ctx, in, inlen, padding, 1, &padded, &padded_len))
		return -1;
	int ret = rsa_dec_sign(ctx, out, padded, padded_len);
	free(padded);
	return ret;
}

int rsa_public_decrypt(size_t inlen, const uint8_t *in, uint8_t *out,
                       struct rsa_ctx *ctx, int padding)
{
	ssize_t size = rsa_size(ctx);
	if (size == -1)
		return -1;
	uint8_t *tmp = malloc(size);
	if (!tmp)
		return -1;
	int ret = rsa_enc_verify(ctx, tmp, in, inlen);
	if (ret == -1)
		goto end;
	if (padding == RSA_NO_PADDING)
	{
		memcpy(out, tmp, ret);
		goto end;
	}
	ret = do_unpad(ctx, out, tmp, ret, padding);

end:
	free(tmp);
	return ret;
}

ssize_t rsa_size(struct rsa_ctx *ctx)
{
	ssize_t bits = rsa_bits(ctx);
	if (bits == -1)
		return bits;
	return bits / 8;
}

ssize_t rsa_bits(struct rsa_ctx *ctx)
{
	if (!ctx || !ctx->n)
		return -1;
	return bignum_num_bits(ctx->n);
}

struct rsa_ctx *rsa_new(void)
{
	struct rsa_ctx *ctx = calloc(sizeof(*ctx), 1);
	if (!ctx)
		return NULL;
	refcount_init(&ctx->refcount, 1);
	return ctx;
}

void rsa_free(struct rsa_ctx *ctx)
{
	if (!ctx)
		return;
	if (refcount_dec(&ctx->refcount))
		return;
	bignum_free(ctx->n);
	bignum_free(ctx->e);
	bignum_free(ctx->q);
	bignum_free(ctx->p);
	bignum_free(ctx->d);
	bignum_free(ctx->phi);
	bignum_free(ctx->dmp);
	bignum_free(ctx->dmq);
	bignum_free(ctx->coef);
	free(ctx);
}

int rsa_up_ref(struct rsa_ctx *ctx)
{
	return refcount_inc(&ctx->refcount);
}
