#include "utils/utils.h"

#include <jkssl/hmac.h>
#include <jkssl/evp.h>

#include <string.h>
#include <stdlib.h>

static int loop(const char *pass, size_t passlen, size_t saltlen, size_t iter,
                const struct evp_md *evp_md, uint32_t i, uint8_t *tmp,
                uint8_t *sum)
{
	uint8_t hash_tmp[EVP_MAX_MD_SIZE];

	be32enc(&tmp[saltlen], i);
	if (!hmac(evp_md, pass, passlen, tmp, saltlen + 4, hash_tmp, NULL))
		return 0;
	memcpy(sum, hash_tmp, evp_md_get_size(evp_md));
	for (uint32_t n = 1; n < iter; ++n)
	{
		if (!hmac(evp_md, pass, passlen, hash_tmp, evp_md_get_size(evp_md),
		          hash_tmp, NULL))
			return 0;
		for (size_t j = 0; j < evp_md_get_size(evp_md); ++j)
			sum[j] ^= hash_tmp[j];
	}
	return 1;
}

int pkcs5_pbkdf2_hmac(const char *pass, size_t passlen,
                      const uint8_t *salt, size_t saltlen, size_t iter,
                      const struct evp_md *evp_md,
                      size_t keylen, uint8_t *out)
{
	uint8_t *tmp = NULL;
	uint8_t sum[EVP_MAX_MD_SIZE];
	int ret = 0;

	tmp = malloc(saltlen + 4);
	if (!tmp)
		goto end;
	memcpy(tmp, salt, saltlen);
	for (uint32_t i = 1; keylen; ++i)
	{
		if (!loop(pass, passlen, saltlen, iter, evp_md, i, tmp, sum))
			goto end;
		size_t n = evp_md_get_size(evp_md);
		if (keylen < n)
			n = keylen;
		memcpy(out, sum, n);
		keylen -= n;
		out += n;
	}
	ret = 1;

end:

	free(tmp);
	return ret;
}

int pkcs5_pbkdf2_hmac_sha1(const char *pass, size_t passlen,
                           const uint8_t *salt, size_t saltlen,
                           size_t iter, size_t keylen, uint8_t *out)
{
	return pkcs5_pbkdf2_hmac(pass, passlen, salt, saltlen, iter, evp_sha1(),
	                         keylen, out);
}
