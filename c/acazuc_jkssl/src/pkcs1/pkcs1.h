#ifndef JKSSL_PKCS1_H
#define JKSSL_PKCS1_H

#include <stdint.h>
#include <stddef.h>

int pkcs1_pad_1(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);
int pkcs1_pad_2(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen);
int pkcs1_unpad(uint8_t *out, size_t *outlen, const uint8_t *in, size_t inlen);

#endif
