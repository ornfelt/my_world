#ifndef JKSSL_EVP_MD_H
#define JKSSL_EVP_MD_H

#include <stdint.h>
#include <stddef.h>

#define EVP_MAX_MD_SIZE 64

struct evp_md_ctx;
struct evp_md;

struct evp_md_ctx *evp_md_ctx_new(void);
void evp_md_ctx_free(struct evp_md_ctx *ctx);
int evp_md_ctx_reset(struct evp_md_ctx *ctx);
int evp_digest(const void *data, size_t count, uint8_t *md, unsigned *size,
               const struct evp_md *evp_md);
int evp_digest_init(struct evp_md_ctx *ctx, const struct evp_md *evp_md);
int evp_digest_update(struct evp_md_ctx *ctx, const uint8_t *data, size_t size);
int evp_digest_final(struct evp_md_ctx *ctx, uint8_t *md);
const struct evp_md *evp_get_digestbyname(const char *name);
void evp_foreach_digest(int (*cb)(const struct evp_md *md, void *data),
                       void *data);

size_t evp_md_get_size(const struct evp_md *evp_md);
size_t evp_md_get_block_size(const struct evp_md *evp_md);
const char *evp_md_get0_name(const struct evp_md *evp_md);

const struct evp_md *evp_adler32(void);

const struct evp_md *evp_crc32(void);

const struct evp_md *evp_md2(void);
const struct evp_md *evp_md4(void);
const struct evp_md *evp_md5(void);

const struct evp_md *evp_ripemd128(void);
const struct evp_md *evp_ripemd160(void);
const struct evp_md *evp_ripemd256(void);
const struct evp_md *evp_ripemd320(void);

const struct evp_md *evp_sha0(void);
const struct evp_md *evp_sha1(void);
const struct evp_md *evp_sha224(void);
const struct evp_md *evp_sha256(void);
const struct evp_md *evp_sha384(void);
const struct evp_md *evp_sha512(void);

const struct evp_md *evp_sm3(void);

#endif
