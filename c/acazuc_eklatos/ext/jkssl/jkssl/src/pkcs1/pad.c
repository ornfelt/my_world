#include "pkcs1/pkcs1.h"

#include <jkssl/rand.h>

#include <string.h>

int pkcs1_pad_1(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
	if (outlen < inlen + 11)
		return 0;
	out[0] = 0;
	out[1] = 1;
	memset(out + 2, 0xFF, outlen - inlen - 3);
	size_t i = 2 + outlen - inlen - 3;
	out[i] = 0;
	memcpy(out + i + 1, in, inlen);
	return 1;
}

static int do_fill(uint8_t *out, size_t len)
{
	uint8_t random[64];
	size_t i;

	if (rand_bytes(random, sizeof(random)) != 1)
		return -1;
	for (i = 0; i < len && i < sizeof(random); ++i)
	{
		if (!random[i])
			return i;
		out[i] = random[i];
	}
	return i;
}

int pkcs1_pad_2(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
	if (outlen < inlen + 11)
		return 0;
	int tmp;
	out[0] = 0;
	out[1] = 2;
	size_t i = 2;
	while (i < outlen - inlen - 1)
	{
		if ((tmp = do_fill(out + i, outlen - inlen - 1 - i)) == -1)
			return 0;
		i += tmp;
	}
	out[i] = 0;
	memcpy(out + i + 1, in, inlen);
	return 1;
}
