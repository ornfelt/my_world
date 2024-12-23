#include "pem/pem.h"

#include <jkssl/evp.h>
#include <jkssl/der.h>
#include <jkssl/bio.h>
#include <jkssl/rsa.h>
#include <jkssl/dsa.h>

#include <stdlib.h>
#include <string.h>

struct evp_pkey *pem_read_bio_private_key(struct bio *bio,
                                          struct evp_pkey **pkey,
                                          const char *pass)
{
	while (1)
	{
		struct pem_read_ctx pem_ctx;
		struct evp_pkey *ret = NULL;
		uint8_t *data;
		pem_ctx.bio = bio;
		pem_ctx.password = pass;
		pem_ctx.text = NULL;
		if (!pem_read(&pem_ctx))
			return NULL;
		data = pem_ctx.data;
		if (!strcmp(pem_ctx.matched_text, "PRIVATE KEY"))
		{
			ret = d2i_pkcs8_private_key(pkey, &data, pem_ctx.len);
		}
		else if (!strcmp(pem_ctx.matched_text, "DSA PRIVATE KEY"))
		{
			struct dsa_ctx *dsa = dsa_new();
			if (dsa)
			{
				if (d2i_dsa_private_key(&dsa, &data, pem_ctx.len))
				{
					ret = evp_pkey_new();
					if (ret)
					{
						if (!evp_pkey_set1_dsa(ret, dsa))
						{
							evp_pkey_free(ret);
							ret = NULL;
						}
					}
				}
				dsa_free(dsa);
			}
		}
		else if (!strcmp(pem_ctx.matched_text, "RSA PRIVATE KEY"))
		{
			struct rsa_ctx *rsa = rsa_new();
			if (rsa)
			{
				if (d2i_rsa_private_key(&rsa, &data, pem_ctx.len))
				{
					ret = evp_pkey_new();
					if (ret)
					{
						if (!evp_pkey_set1_rsa(ret, rsa))
						{
							evp_pkey_free(ret);
							ret = NULL;
						}
					}
				}
				rsa_free(rsa);
			}
		}
		free(pem_ctx.data);
		if (ret)
			return ret;
	}
	return NULL;
}

struct evp_pkey *pem_read_private_key(FILE *fp, struct evp_pkey **pkey,
                                      const char *pass)
{
	PEM_READ_FP(private_key, struct evp_pkey, pkey, pass);
}

PEM_DEF_WRITE_PASS(private_key, evp_pkey, "PRIVATE KEY");
PEM_DEF_RPWP(pkcs8_private_key, evp_pkey, "PRIVATE KEY");
PEM_DEF_RPWC(pubkey, evp_pkey, "PUBLIC KEY");
