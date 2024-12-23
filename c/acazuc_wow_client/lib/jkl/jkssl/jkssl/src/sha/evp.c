#include "sha/sha.h"
#include "evp/md.h"

const struct evp_md *evp_sha0(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA0",
		.init = (evp_md_init_t)sha0_init,
		.update = (evp_md_update_t)sha0_update,
		.final = (evp_md_final_t)sha0_final,
		.digest_size = SHA_DIGEST_LENGTH,
		.block_size = 64,
		.ctx_size = sizeof(struct sha0_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha1(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA1",
		.init = (evp_md_init_t)sha1_init,
		.update = (evp_md_update_t)sha1_update,
		.final = (evp_md_final_t)sha1_final,
		.digest_size = SHA_DIGEST_LENGTH,
		.block_size = 64,
		.ctx_size = sizeof(struct sha1_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha224(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA224",
		.init = (evp_md_init_t)sha224_init,
		.update = (evp_md_update_t)sha224_update,
		.final = (evp_md_final_t)sha224_final,
		.digest_size = SHA224_DIGEST_LENGTH,
		.block_size = 64,
		.ctx_size = sizeof(struct sha224_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha256(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA256",
		.init = (evp_md_init_t)sha256_init,
		.update = (evp_md_update_t)sha256_update,
		.final = (evp_md_final_t)sha256_final,
		.digest_size = SHA256_DIGEST_LENGTH,
		.block_size = 64,
		.ctx_size = sizeof(struct sha256_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha384(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA384",
		.init = (evp_md_init_t)sha384_init,
		.update = (evp_md_update_t)sha384_update,
		.final = (evp_md_final_t)sha384_final,
		.digest_size = SHA384_DIGEST_LENGTH,
		.block_size = 128,
		.ctx_size = sizeof(struct sha384_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha512(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA512",
		.init = (evp_md_init_t)sha512_init,
		.update = (evp_md_update_t)sha512_update,
		.final = (evp_md_final_t)sha512_final,
		.digest_size = SHA512_DIGEST_LENGTH,
		.block_size = 128,
		.ctx_size = sizeof(struct sha512_ctx),
	};
	return &evp;
}
