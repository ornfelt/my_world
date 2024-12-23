#ifndef JKSSL_EVP_PKEY_INT_H
#define JKSSL_EVP_PKEY_INT_H

#include "refcount.h"

#include <jkssl/evp.h>

#include <stddef.h>
#include <stdint.h>

enum evp_pkey_type
{
	EVP_PKEY_NONE,
	EVP_PKEY_RSA,
	EVP_PKEY_DSA,
	EVP_PKEY_DH,
};

struct evp_pkey
{
	refcount_t refcount;
	enum evp_pkey_type type;
	union
	{
		void *ptr;
		struct rsa_ctx *rsa;
		struct dsa_ctx *dsa;
		struct dh_ctx *dh;
	};
};

struct evp_pkey_ctx
{
	struct evp_pkey *pkey;
	const struct evp_md *md;
	int rsa_padding;
};

#endif
