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
		.digest_size = 20,
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
		.digest_size = 20,
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
		.digest_size = 28,
		.block_size = 64,
		.ctx_size = sizeof(struct sha256_ctx),
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
		.digest_size = 32,
		.block_size = 64,
		.ctx_size = sizeof(struct sha256_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha256_192(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA256-192",
		.init = (evp_md_init_t)sha256_192_init,
		.update = (evp_md_update_t)sha256_192_update,
		.final = (evp_md_final_t)sha256_192_final,
		.digest_size = 24,
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
		.digest_size = 48,
		.block_size = 128,
		.ctx_size = sizeof(struct sha512_ctx),
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
		.digest_size = 64,
		.block_size = 128,
		.ctx_size = sizeof(struct sha512_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha512_224(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA512-224",
		.init = (evp_md_init_t)sha512_224_init,
		.update = (evp_md_update_t)sha512_224_update,
		.final = (evp_md_final_t)sha512_224_final,
		.digest_size = 28,
		.block_size = 128,
		.ctx_size = sizeof(struct sha512_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha512_256(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA512-256",
		.init = (evp_md_init_t)sha512_256_init,
		.update = (evp_md_update_t)sha512_256_update,
		.final = (evp_md_final_t)sha512_256_final,
		.digest_size = 32,
		.block_size = 128,
		.ctx_size = sizeof(struct sha512_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha3_224(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA3-224",
		.init = (evp_md_init_t)sha3_224_init,
		.update = (evp_md_update_t)sha3_224_update,
		.final = (evp_md_final_t)sha3_224_final,
		.digest_size = 28,
		.block_size = 144,
		.ctx_size = sizeof(struct sha3_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha3_256(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA3-256",
		.init = (evp_md_init_t)sha3_256_init,
		.update = (evp_md_update_t)sha3_256_update,
		.final = (evp_md_final_t)sha3_256_final,
		.digest_size = 32,
		.block_size = 136,
		.ctx_size = sizeof(struct sha3_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha3_384(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA3-384",
		.init = (evp_md_init_t)sha3_384_init,
		.update = (evp_md_update_t)sha3_384_update,
		.final = (evp_md_final_t)sha3_384_final,
		.digest_size = 48,
		.block_size = 104,
		.ctx_size = sizeof(struct sha3_ctx),
	};
	return &evp;
}

const struct evp_md *evp_sha3_512(void)
{
	static const struct evp_md evp =
	{
		.name = "SHA3-512",
		.init = (evp_md_init_t)sha3_512_init,
		.update = (evp_md_update_t)sha3_512_update,
		.final = (evp_md_final_t)sha3_512_final,
		.digest_size = 64,
		.block_size = 72,
		.ctx_size = sizeof(struct sha3_ctx),
	};
	return &evp;
}

const struct evp_md *evp_shake128(void)
{
	static const struct evp_md evp =
	{
		.name = "SHAKE128",
		.init = (evp_md_init_t)shake128_init,
		.update = (evp_md_update_t)shake128_update,
		.final = (evp_md_final_t)shake128_final,
		.digest_size = 16,
		.block_size = 168,
		.ctx_size = sizeof(struct sha3_ctx),
	};
	return &evp;
}

const struct evp_md *evp_shake256(void)
{
	static const struct evp_md evp =
	{
		.name = "SHAKE256",
		.init = (evp_md_init_t)shake256_init,
		.update = (evp_md_update_t)shake256_update,
		.final = (evp_md_final_t)shake256_final,
		.digest_size = 32,
		.block_size = 136,
		.ctx_size = sizeof(struct sha3_ctx),
	};
	return &evp;
}
