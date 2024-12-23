#ifndef JKSSL_EVP_CIPHER_H
#define JKSSL_EVP_CIPHER_H

#include <stdint.h>
#include <stddef.h>

#define EVP_MAX_KEY_LENGTH 32
#define EVP_MAX_IV_LENGTH  16

struct evp_cipher_ctx;
struct evp_cipher;

struct evp_cipher_ctx *evp_cipher_ctx_new(void);
void evp_cipher_ctx_free(struct evp_cipher_ctx *ctx);
int evp_cipher_init(struct evp_cipher_ctx *ctx,
                    const struct evp_cipher *evp_cipher, const uint8_t *key,
                    const uint8_t *iv, int enc);
int evp_cipher_update(struct evp_cipher_ctx *ctx, uint8_t *out, size_t *outl,
                      const uint8_t *in, size_t inlen);
int evp_cipher_final(struct evp_cipher_ctx *ctx, uint8_t *out, size_t *outl);
const struct evp_cipher *evp_get_cipherbyname(const char *name);
void evp_foreach_cipher(int (*cb)(const struct evp_cipher *cipher, void *data),
                        void *data);

size_t evp_cipher_get_block_size(const struct evp_cipher *evp_cipher);
size_t evp_cipher_get_key_length(const struct evp_cipher *evp_cipher);
size_t evp_cipher_get_iv_length(const struct evp_cipher *evp_cipher);
const char *evp_cipher_get0_name(const struct evp_cipher *evp_cipher);

const struct evp_cipher *evp_aes_128_ecb(void);
const struct evp_cipher *evp_aes_128_cbc(void);
const struct evp_cipher *evp_aes_128_pcbc(void);
const struct evp_cipher *evp_aes_128_cfb(void);
const struct evp_cipher *evp_aes_128_ofb(void);
const struct evp_cipher *evp_aes_128_ctr(void);
const struct evp_cipher *evp_aes_128_gcm(void);
const struct evp_cipher *evp_aes_192_ecb(void);
const struct evp_cipher *evp_aes_192_cbc(void);
const struct evp_cipher *evp_aes_192_pcbc(void);
const struct evp_cipher *evp_aes_192_cfb(void);
const struct evp_cipher *evp_aes_192_ofb(void);
const struct evp_cipher *evp_aes_192_ctr(void);
const struct evp_cipher *evp_aes_192_gcm(void);
const struct evp_cipher *evp_aes_256_ecb(void);
const struct evp_cipher *evp_aes_256_cbc(void);
const struct evp_cipher *evp_aes_256_pcbc(void);
const struct evp_cipher *evp_aes_256_cfb(void);
const struct evp_cipher *evp_aes_256_ofb(void);
const struct evp_cipher *evp_aes_256_ctr(void);
const struct evp_cipher *evp_aes_256_gcm(void);

const struct evp_cipher *evp_aria_128_ecb(void);
const struct evp_cipher *evp_aria_128_cbc(void);
const struct evp_cipher *evp_aria_128_pcbc(void);
const struct evp_cipher *evp_aria_128_cfb(void);
const struct evp_cipher *evp_aria_128_ofb(void);
const struct evp_cipher *evp_aria_128_ctr(void);
const struct evp_cipher *evp_aria_128_gcm(void);
const struct evp_cipher *evp_aria_192_ecb(void);
const struct evp_cipher *evp_aria_192_cbc(void);
const struct evp_cipher *evp_aria_192_pcbc(void);
const struct evp_cipher *evp_aria_192_cfb(void);
const struct evp_cipher *evp_aria_192_ofb(void);
const struct evp_cipher *evp_aria_192_ctr(void);
const struct evp_cipher *evp_aria_192_gcm(void);
const struct evp_cipher *evp_aria_256_ecb(void);
const struct evp_cipher *evp_aria_256_cbc(void);
const struct evp_cipher *evp_aria_256_pcbc(void);
const struct evp_cipher *evp_aria_256_cfb(void);
const struct evp_cipher *evp_aria_256_ofb(void);
const struct evp_cipher *evp_aria_256_ctr(void);
const struct evp_cipher *evp_aria_256_gcm(void);

const struct evp_cipher *evp_bf_ecb(void);
const struct evp_cipher *evp_bf_cbc(void);
const struct evp_cipher *evp_bf_pcbc(void);
const struct evp_cipher *evp_bf_cfb(void);
const struct evp_cipher *evp_bf_ofb(void);
const struct evp_cipher *evp_bf_ctr(void);

const struct evp_cipher *evp_camellia_128_ecb(void);
const struct evp_cipher *evp_camellia_128_cbc(void);
const struct evp_cipher *evp_camellia_128_pcbc(void);
const struct evp_cipher *evp_camellia_128_cfb(void);
const struct evp_cipher *evp_camellia_128_ofb(void);
const struct evp_cipher *evp_camellia_128_ctr(void);
const struct evp_cipher *evp_camellia_128_gcm(void);
const struct evp_cipher *evp_camellia_192_ecb(void);
const struct evp_cipher *evp_camellia_192_cbc(void);
const struct evp_cipher *evp_camellia_192_pcbc(void);
const struct evp_cipher *evp_camellia_192_cfb(void);
const struct evp_cipher *evp_camellia_192_ofb(void);
const struct evp_cipher *evp_camellia_192_ctr(void);
const struct evp_cipher *evp_camellia_192_gcm(void);
const struct evp_cipher *evp_camellia_256_ecb(void);
const struct evp_cipher *evp_camellia_256_cbc(void);
const struct evp_cipher *evp_camellia_256_pcbc(void);
const struct evp_cipher *evp_camellia_256_cfb(void);
const struct evp_cipher *evp_camellia_256_ofb(void);
const struct evp_cipher *evp_camellia_256_ctr(void);
const struct evp_cipher *evp_camellia_256_gcm(void);

const struct evp_cipher *evp_cast5_ecb(void);
const struct evp_cipher *evp_cast5_cbc(void);
const struct evp_cipher *evp_cast5_pcbc(void);
const struct evp_cipher *evp_cast5_cfb(void);
const struct evp_cipher *evp_cast5_ofb(void);
const struct evp_cipher *evp_cast5_ctr(void);

const struct evp_cipher *evp_chacha20(void);

const struct evp_cipher *evp_des_ecb(void);
const struct evp_cipher *evp_des_cbc(void);
const struct evp_cipher *evp_des_pcbc(void);
const struct evp_cipher *evp_des_cfb(void);
const struct evp_cipher *evp_des_ofb(void);
const struct evp_cipher *evp_des_ctr(void);
const struct evp_cipher *evp_des_ede_ecb(void);
const struct evp_cipher *evp_des_ede_cbc(void);
const struct evp_cipher *evp_des_ede_pcbc(void);
const struct evp_cipher *evp_des_ede_cfb(void);
const struct evp_cipher *evp_des_ede_ofb(void);
const struct evp_cipher *evp_des_ede_ctr(void);
const struct evp_cipher *evp_des_ede3_ecb(void);
const struct evp_cipher *evp_des_ede3_cbc(void);
const struct evp_cipher *evp_des_ede3_pcbc(void);
const struct evp_cipher *evp_des_ede3_cfb(void);
const struct evp_cipher *evp_des_ede3_ofb(void);
const struct evp_cipher *evp_des_ede3_ctr(void);

const struct evp_cipher *evp_rc2_ecb(void);
const struct evp_cipher *evp_rc2_cbc(void);
const struct evp_cipher *evp_rc2_pcbc(void);
const struct evp_cipher *evp_rc2_cfb(void);
const struct evp_cipher *evp_rc2_ofb(void);
const struct evp_cipher *evp_rc2_ctr(void);

const struct evp_cipher *evp_rc4(void);

const struct evp_cipher *evp_seed_ecb(void);
const struct evp_cipher *evp_seed_cbc(void);
const struct evp_cipher *evp_seed_pcbc(void);
const struct evp_cipher *evp_seed_cfb(void);
const struct evp_cipher *evp_seed_ofb(void);
const struct evp_cipher *evp_seed_ctr(void);
const struct evp_cipher *evp_seed_gcm(void);

const struct evp_cipher *evp_serpent_128_ecb(void);
const struct evp_cipher *evp_serpent_128_cbc(void);
const struct evp_cipher *evp_serpent_128_pcbc(void);
const struct evp_cipher *evp_serpent_128_cfb(void);
const struct evp_cipher *evp_serpent_128_ofb(void);
const struct evp_cipher *evp_serpent_128_ctr(void);
const struct evp_cipher *evp_serpent_128_gcm(void);
const struct evp_cipher *evp_serpent_192_ecb(void);
const struct evp_cipher *evp_serpent_192_cbc(void);
const struct evp_cipher *evp_serpent_192_pcbc(void);
const struct evp_cipher *evp_serpent_192_cfb(void);
const struct evp_cipher *evp_serpent_192_ofb(void);
const struct evp_cipher *evp_serpent_192_ctr(void);
const struct evp_cipher *evp_serpent_192_gcm(void);
const struct evp_cipher *evp_serpent_256_ecb(void);
const struct evp_cipher *evp_serpent_256_cbc(void);
const struct evp_cipher *evp_serpent_256_pcbc(void);
const struct evp_cipher *evp_serpent_256_cfb(void);
const struct evp_cipher *evp_serpent_256_ofb(void);
const struct evp_cipher *evp_serpent_256_ctr(void);
const struct evp_cipher *evp_serpent_256_gcm(void);

const struct evp_cipher *evp_sm4_ecb(void);
const struct evp_cipher *evp_sm4_cbc(void);
const struct evp_cipher *evp_sm4_pcbc(void);
const struct evp_cipher *evp_sm4_cfb(void);
const struct evp_cipher *evp_sm4_ofb(void);
const struct evp_cipher *evp_sm4_ctr(void);
const struct evp_cipher *evp_sm4_gcm(void);

#endif
