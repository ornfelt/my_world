#ifndef JKSSL_EVP_MD_INT_H
#define JKSSL_EVP_MD_INT_H

#include <jkssl/evp.h>

#include <stdint.h>
#include <stddef.h>

typedef int (*evp_md_init_t)(void *ctx);
typedef int (*evp_md_update_t)(void *ctx, const uint8_t *data, size_t size);
typedef int (*evp_md_final_t)(uint8_t *md, void *ctx);

struct evp_md
{
	const char *name;
	evp_md_init_t init;
	evp_md_update_t update;
	evp_md_final_t final;
	uint32_t digest_size;
	uint32_t block_size;
	uint32_t ctx_size;
};

struct evp_md_ctx
{
	const struct evp_md *evp_md;
	void *ctx;
};

#endif
