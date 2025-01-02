#include <jkssl/evp.h>

#include <string.h>

int evp_bytestokey(const struct evp_cipher *type,
                   const struct evp_md *md,
                   const uint8_t *salt, const uint8_t *data,
                   size_t datal, size_t count, uint8_t *key, uint8_t *iv)
{
	struct evp_md_ctx *evp_md_ctx;
	size_t ivlen;
	size_t keylen = evp_cipher_get_key_length(type);
	uint8_t digest[EVP_MAX_MD_SIZE];
	int ret = 0;
	int first = 1;

	evp_md_ctx = evp_md_ctx_new();
	if (!evp_md_ctx)
		return 0;
	if (!evp_digest_init(evp_md_ctx, md))
		goto end;
	if (iv)
		ivlen = evp_cipher_get_block_size(type);
	else
		ivlen = 0;
	while (ivlen > 0 || keylen > 0)
	{
		if (!evp_md_ctx_reset(evp_md_ctx))
			goto end;
		if (!first)
		{
			if (!evp_digest_update(evp_md_ctx, digest, evp_md_get_size(md)))
				goto end;
		}
		else
		{
			first = 0;
		}
		if (!evp_digest_update(evp_md_ctx, data, datal)
		 || (salt && !evp_digest_update(evp_md_ctx, salt, 8))
		 || !evp_digest_final(evp_md_ctx, digest))
			goto end;
		for (size_t i = 1; i < count; ++i)
		{
			if (!evp_digest(digest, evp_md_get_size(md), digest, NULL, md))
				goto end;
		}
		size_t avail = evp_md_get_size(md);
		uint8_t *it = digest;
		if (keylen > 0)
		{
			size_t n = avail;
			if (n > keylen)
				n = keylen;
			memcpy(key, it, n);
			it += n;
			avail -= n;
			key += n;
			keylen -= n;
		}
		if (ivlen > 0)
		{
			size_t n = avail;
			if (n > ivlen)
				n = ivlen;
			memcpy(iv, it, n);
			iv += n;
			ivlen -= n;
		}
	}
	ret = 1;

end:
	evp_md_ctx_free(evp_md_ctx);
	return ret;
}
