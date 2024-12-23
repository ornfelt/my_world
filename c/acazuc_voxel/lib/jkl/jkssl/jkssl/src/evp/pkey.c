#include "evp/pkey.h"

#include <jkssl/rsa.h>
#include <jkssl/dsa.h>
#include <jkssl/dh.h>

#include <stdlib.h>
#include <string.h>

struct evp_pkey *evp_pkey_new(void)
{
	struct evp_pkey *pkey = calloc(sizeof(*pkey), 1);
	if (!pkey)
		return NULL;
	refcount_init(&pkey->refcount, 1);
	return pkey;
}

static void release_pkey_key(struct evp_pkey *pkey)
{
	switch (pkey->type)
	{
		default:
			break;
		case EVP_PKEY_RSA:
			rsa_free(pkey->rsa);
			break;
		case EVP_PKEY_DSA:
			dsa_free(pkey->dsa);
			break;
		case EVP_PKEY_DH:
			dh_free(pkey->dh);
			break;
	}
}

void evp_pkey_free(struct evp_pkey *pkey)
{
	if (!pkey)
		return;
	if (refcount_dec(&pkey->refcount))
		return;
	release_pkey_key(pkey);
	free(pkey);
}

int evp_pkey_up_ref(struct evp_pkey *pkey)
{
	return refcount_inc(&pkey->refcount);
}

int evp_pkey_assign_rsa(struct evp_pkey *pkey, struct rsa_ctx *rsa)
{
	release_pkey_key(pkey);
	pkey->type = EVP_PKEY_RSA;
	pkey->rsa = rsa;
	return 1;
}

int evp_pkey_set1_rsa(struct evp_pkey *pkey, struct rsa_ctx *rsa)
{
	if (!evp_pkey_assign_rsa(pkey, rsa))
		return 0;
	rsa_up_ref(rsa);
	return 1;
}

int evp_pkey_assign_dsa(struct evp_pkey *pkey, struct dsa_ctx *dsa)
{
	release_pkey_key(pkey);
	pkey->type = EVP_PKEY_DSA;
	pkey->dsa = dsa;
	return 1;
}

int evp_pkey_set1_dsa(struct evp_pkey *pkey, struct dsa_ctx *dsa)
{
	if (!evp_pkey_assign_dsa(pkey, dsa))
		return 0;
	dsa_up_ref(dsa);
	return 1;
}

int evp_pkey_assign_dh(struct evp_pkey *pkey, struct dh_ctx *dh)
{
	release_pkey_key(pkey);
	pkey->type = EVP_PKEY_DH;
	pkey->dh = dh;
	return 1;
}

int evp_pkey_set1_dh(struct evp_pkey *pkey, struct dh_ctx *dh)
{
	if (!evp_pkey_assign_dh(pkey, dh))
		return 0;
	dh_up_ref(dh);
	return 1;
}

struct rsa_ctx *evp_pkey_get0_rsa(struct evp_pkey *pkey)
{
	if (pkey->type != EVP_PKEY_RSA)
		return NULL;
	return pkey->rsa;
}

struct rsa_ctx *evp_pkey_get1_rsa(struct evp_pkey *pkey)
{
	struct rsa_ctx *rsa = evp_pkey_get0_rsa(pkey);
	if (!rsa)
		return NULL;
	rsa_up_ref(rsa);
	return rsa;
}

struct dsa_ctx *evp_pkey_get0_dsa(struct evp_pkey *pkey)
{
	if (pkey->type != EVP_PKEY_DSA)
		return NULL;
	return pkey->dsa;
}

struct dsa_ctx *evp_pkey_get1_dsa(struct evp_pkey *pkey)
{
	struct dsa_ctx *dsa = evp_pkey_get0_dsa(pkey);
	if (!dsa)
		return NULL;
	dsa_up_ref(dsa);
	return dsa;
}

struct dh_ctx *evp_pkey_get0_dh(struct evp_pkey *pkey)
{
	if (pkey->type != EVP_PKEY_DH)
		return NULL;
	return pkey->dh;
}

struct dh_ctx *evp_pkey_get1_dh(struct evp_pkey *pkey)
{
	struct dh_ctx *dh = evp_pkey_get0_dh(pkey);
	if (!dh)
		return NULL;
	dh_up_ref(dh);
	return dh;
}

struct evp_pkey_ctx *evp_pkey_ctx_new(struct evp_pkey *pkey)
{
	struct evp_pkey_ctx *ctx = calloc(sizeof(*ctx), 1);
	if (!ctx)
		return NULL;
	ctx->pkey = pkey;
	evp_pkey_up_ref(pkey);
	ctx->rsa_padding = RSA_PKCS1_PADDING;
	return ctx;
}

void evp_pkey_ctx_free(struct evp_pkey_ctx *ctx)
{
	if (!ctx)
		return;
	evp_pkey_free(ctx->pkey);
	free(ctx);
}

int evp_pkey_ctx_set_rsa_padding(struct evp_pkey_ctx *ctx, int padding)
{
	ctx->rsa_padding = padding;
	return 1;
}

int evp_pkey_ctx_set_signature_md(struct evp_pkey_ctx *ctx,
                                  const struct evp_md *md)
{
	ctx->md = md;
	return 1;
}

int evp_pkey_sign_init(struct evp_pkey_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int evp_pkey_sign(struct evp_pkey_ctx *ctx, uint8_t *sig, size_t *siglen,
                  const uint8_t *tbs, size_t tbslen)
{
	if (!ctx->pkey)
		return -1;
	switch (ctx->pkey->type)
	{
		case EVP_PKEY_NONE:
			return -1;
		case EVP_PKEY_RSA:
			if (!sig)
			{
				*siglen = rsa_size(ctx->pkey->rsa);
				return 1;
			}
			switch (ctx->rsa_padding)
			{
				case RSA_NO_PADDING:
				{
					int ret = rsa_private_encrypt(tbslen, tbs, sig,
					                              ctx->pkey->rsa,
					                              RSA_NO_PADDING);
					if (ret == -1)
						return -1;
					*siglen = ret;
					return 1;
				}
				case RSA_PKCS1_PADDING:
					return rsa_sign(ctx->md, ctx->pkey->rsa, tbs, tbslen,
					                sig, siglen);
				default:
					return -1;
			}
		case EVP_PKEY_DSA:
			if (!sig)
			{
				*siglen = dsa_size(ctx->pkey->dsa);
				return 1;
			}
			return dsa_sign(ctx->pkey->dsa, tbs, tbslen, sig, siglen);
		case EVP_PKEY_DH:
			return -2;
	}
	return -1;
}

int evp_pkey_verify_init(struct evp_pkey_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int evp_pkey_verify(struct evp_pkey_ctx *ctx, const uint8_t *sig,
                    size_t siglen, const uint8_t *tbs, size_t tbslen)
{
	if (!ctx->pkey)
		return -1;
	switch (ctx->pkey->type)
	{
		case EVP_PKEY_NONE:
			return -1;
		case EVP_PKEY_RSA:
			switch (ctx->rsa_padding)
			{
				case RSA_NO_PADDING:
				{
					uint8_t *tmp = malloc(rsa_size(ctx->pkey->rsa));
					if (!tmp)
						return -1;
					int ret = rsa_public_decrypt(tbslen, tbs, tmp,
					                             ctx->pkey->rsa,
					                             RSA_NO_PADDING);
					if (ret == -1)
					{
						free(tmp);
						return -1;
					}
					if ((size_t)ret != tbslen)
					{
						free(tmp);
						return 0;
					}
					ret = !memcmp(tmp, sig, tbslen);
					free(tmp);
					return ret;
				}
				case RSA_PKCS1_PADDING:
					return rsa_verify(ctx->md, ctx->pkey->rsa, tbs, tbslen,
					                  sig, siglen);
				default:
					return -1;
			}
		case EVP_PKEY_DSA:
			return dsa_verify(ctx->pkey->dsa, tbs, tbslen, sig, siglen);
		case EVP_PKEY_DH:
			return -2;
	}
	return -1;
}

int evp_pkey_encrypt_init(struct evp_pkey_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int evp_pkey_encrypt(struct evp_pkey_ctx *ctx, uint8_t *out,
                     size_t *outlen, const uint8_t *in, size_t inlen)
{
	if (!ctx->pkey)
		return -1;
	switch (ctx->pkey->type)
	{
		case EVP_PKEY_NONE:
			return -1;
		case EVP_PKEY_RSA:
		{
			int ret = rsa_enc(ctx->pkey->rsa, out, in, inlen,
			                   RSA_PKCS1_PADDING);
			if (ret == -1)
				return -1;
			*outlen = ret;
			return 0;
		}
		case EVP_PKEY_DSA:
			return -2;
		case EVP_PKEY_DH:
			return -2;
	}
	return -1;
}

int evp_pkey_decrypt_init(struct evp_pkey_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int evp_pkey_decrypt(struct evp_pkey_ctx *ctx, uint8_t *out,
                     size_t *outlen, const uint8_t *in, size_t inlen)
{
	if (!ctx->pkey)
		return -1;
	switch (ctx->pkey->type)
	{
		case EVP_PKEY_NONE:
			return -1;
		case EVP_PKEY_RSA:
		{
			int ret = rsa_dec(ctx->pkey->rsa, out, in, inlen,
			                  RSA_PKCS1_PADDING);
			if (ret == -1)
				return -1;
			*outlen = ret;
			return 0;
		}
		case EVP_PKEY_DSA:
			return -2;
		case EVP_PKEY_DH:
			return -2;
	}
	return 0;
}

int evp_pkey_derive_init(struct evp_pkey_ctx *ctx)
{
	(void)ctx;
	return 1;
}

int evp_pkey_derive(struct evp_pkey_ctx *ctx, uint8_t *key, size_t *keylen)
{
	if (!ctx->pkey)
		return -1;
	switch (ctx->pkey->type)
	{
		case EVP_PKEY_NONE:
			return -1;
		case EVP_PKEY_RSA:
			return -2;
		case EVP_PKEY_DSA:
			return -2;
		case EVP_PKEY_DH:
			return 0; /* XXX */
	}
	return 0;
}
