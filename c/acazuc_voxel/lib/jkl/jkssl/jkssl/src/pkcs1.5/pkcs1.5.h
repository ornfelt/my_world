#ifndef JKSSL_PKCS1_5_H
#define JKSSL_PKCS1_5_H

#include <stdint.h>
#include <stddef.h>

int pkcs1_5_pad_1(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);
int pkcs1_5_pad_2(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);
int pkcs1_5_unpad(uint8_t *out, size_t *outlen, const uint8_t *in, size_t inlen);

#endif
