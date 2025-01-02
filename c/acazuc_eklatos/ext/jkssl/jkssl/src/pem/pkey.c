#include "asn1/asn1.h"
#include "evp/pkey.h"
#include "pem/pem.h"

#include <jkssl/evp.h>
#include <jkssl/der.h>
#include <jkssl/bio.h>
#include <jkssl/rsa.h>
#include <jkssl/dsa.h>
#include <jkssl/ec.h>

#include <stdlib.h>
#include <string.h>

ASN1_OBJ_ALLOC_DEF(evp_pkey, struct evp_pkey, evp_pkey_new);

static struct evp_pkey *handle_dsa(struct evp_pkey **pkeyp,
                                   uint8_t *data, size_t len)
{
	struct evp_pkey *pkey = NULL;
	struct dsa *dsa = NULL;
	int allocated = 0;

	if (!get_evp_pkey(&pkey, pkeyp, &allocated))
		return NULL;
	dsa = d2i_dsa_private_key(NULL, &data, len);
	if (!dsa)
		goto err;
	if (!evp_pkey_assign_dsa(pkey, dsa))
		goto err;
	return pkey;

err:
	if (allocated)
		evp_pkey_free(pkey);
	dsa_free(dsa);
	return NULL;
}

static struct evp_pkey *handle_rsa(struct evp_pkey **pkeyp,
                                   uint8_t *data, size_t len)
{
	struct evp_pkey *pkey = NULL;
	struct rsa *rsa = NULL;
	int allocated = 0;

	if (!get_evp_pkey(&pkey, pkeyp, &allocated))
		return NULL;
	rsa = d2i_rsa_private_key(NULL, &data, len);
	if (!rsa)
		goto err;
	if (!evp_pkey_assign_rsa(pkey, rsa))
		goto err;
	return pkey;

err:
	if (allocated)
		evp_pkey_free(pkey);
	rsa_free(rsa);
	return NULL;
}

static struct evp_pkey *handle_ec_key(struct evp_pkey **pkeyp,
                                      uint8_t *data, size_t len)
{
	struct evp_pkey *pkey = NULL;
	struct ec_key *ec_key = NULL;
	int allocated = 0;

	if (!get_evp_pkey(&pkey, pkeyp, &allocated))
		return NULL;
	ec_key = d2i_ec_private_key(NULL, &data, len);
	if (!ec_key)
		goto err;
	if (!evp_pkey_assign_ec_key(pkey, ec_key))
		goto err;
	return pkey;

err:
	if (allocated)
		evp_pkey_free(pkey);
	ec_key_free(ec_key);
	return NULL;
}

struct evp_pkey *pem_read_bio_private_key(struct bio *bio,
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
		else if (!strcmp(pem_ctx.matched_text, "DSA PRIVATE KEY"))
			ret = handle_dsa(pkey, data, pem_ctx.len);
		else if (!strcmp(pem_ctx.matched_text, "RSA PRIVATE KEY"))
			ret = handle_rsa(pkey, data, pem_ctx.len);
		else if (!strcmp(pem_ctx.matched_text, "EC PRIVATE KEY"))
			ret = handle_ec_key(pkey, data, pem_ctx.len);
		free(pem_ctx.data);
		if (ret)
			return ret;
	}
	return NULL;
}

struct evp_pkey *pem_read_private_key(FILE *fp,
                                      struct evp_pkey **pkey,
                                      pem_password_cb *cb,
                                      void *userdata)
{
	PEM_READ_FP(private_key, struct evp_pkey, pkey, cb, userdata);
}

int pem_write_bio_private_key(struct bio *bio,
                              const struct evp_pkey *pkey,
                              const struct evp_cipher *cipher,
                              pem_password_cb *cb,
                              void *userdata)
{
	return pem_write_bio_pkcs8_private_key(bio, pkey, cipher, cb, userdata);
}

int pem_write_private_key(FILE *fp,
                          const struct evp_pkey *pkey,
                          const struct evp_cipher *cipher,
                          pem_password_cb *cb,
                          void *userdata)
{
	return pem_write_pkcs8_private_key(fp, pkey, cipher, cb, userdata);
}

struct evp_pkey *pem_read_bio_private_key_traditional(struct bio *bio,
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
		if (!strcmp(pem_ctx.matched_text, "DSA PRIVATE KEY"))
			ret = handle_dsa(pkey, data, pem_ctx.len);
		else if (!strcmp(pem_ctx.matched_text, "RSA PRIVATE KEY"))
			ret = handle_rsa(pkey, data, pem_ctx.len);
		else if (!strcmp(pem_ctx.matched_text, "EC PRIVATE KEY"))
			ret = handle_ec_key(pkey, data, pem_ctx.len);
		free(pem_ctx.data);
		if (ret)
			return ret;
	}
	return NULL;
}

struct evp_pkey *pem_read_private_key_traditional(FILE *fp,
                                                  struct evp_pkey **pkey,
                                                  pem_password_cb *cb,
                                                  void *userdata)
{
	PEM_READ_FP(private_key_traditional, struct evp_pkey, pkey, cb, userdata);
}

int pem_write_bio_private_key_traditional(struct bio *bio,
                                          const struct evp_pkey *pkey,
                                          const struct evp_cipher *cipher,
                                          pem_password_cb *cb,
                                          void *userdata)
{
	struct pem_write_ctx pem_ctx;
	int ret;
	pem_ctx.bio = bio;
	pem_ctx.evp_cipher = cipher;
	pem_ctx.password_cb = cb;
	pem_ctx.password_userdata = userdata;
	pem_ctx.len = -1;
	switch (pkey->type)
	{
		case EVP_PKEY_NONE:
		case EVP_PKEY_DH:
			return 0;
		case EVP_PKEY_RSA:
			pem_ctx.text = "RSA PRIVATE KEY";
			pem_ctx.len = i2d_rsa_private_key(pkey->rsa,
			                                  &pem_ctx.data);
			break;
		case EVP_PKEY_DSA:
			pem_ctx.text = "DSA PRIVATE KEY";
			pem_ctx.len = i2d_dsa_private_key(pkey->dsa,
			                                  &pem_ctx.data);
			break;
		case EVP_PKEY_EC:
			pem_ctx.text = "EC PRIVATE KEY";
			pem_ctx.len = i2d_ec_private_key(pkey->ec,
			                                 &pem_ctx.data);
			break;
	}
	if (pem_ctx.len == (size_t)-1)
		return 0;
	ret = pem_write(&pem_ctx);
	free(pem_ctx.data);
	return ret;
}

int pem_write_private_key_traditional(FILE *fp,
                                      const struct evp_pkey *pkey,
                                      const struct evp_cipher *cipher,
                                      pem_password_cb *cb,
                                      void *userdata)
{
	PEM_WRITE_FP(private_key_traditional, pkey, cipher, cb, userdata);
}
