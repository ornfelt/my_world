#ifndef JKSSL_EVP_H
#define JKSSL_EVP_H

#include "evp_cipher.h"
#include "evp_encode.h"
#include "evp_pkey.h"
#include "evp_md.h"

int evp_bytestokey(const struct evp_cipher *type, const struct evp_md *md,
                   const uint8_t *salt, const uint8_t *data, size_t datal,
                   size_t count, uint8_t *key, uint8_t *iv);

int pkcs5_pbkdf2_hmac(const char *pass, size_t passlen, const uint8_t *salt,
                      size_t saltlen, size_t iter, const struct evp_md *digest,
                      size_t keylen, uint8_t *out);

int pkcs5_pbkdf2_hmac_sha1(const char *pass, size_t passlen, const uint8_t *salt,
                           size_t saltlen, size_t iter, size_t keylen,
                           uint8_t *out);

#endif
