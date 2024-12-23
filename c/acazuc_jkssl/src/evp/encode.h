#ifndef JKSSL_EVP_ENCODE_INT_H
#define JKSSL_EVP_ENCODE_INT_H

#include <jkssl/evp.h>

#include <stdint.h>
#include <stddef.h>

struct evp_encode_ctx
{
	uint8_t tmp[4];
	uint8_t tmp_len;
	size_t count;
	int ended;
};

#endif
