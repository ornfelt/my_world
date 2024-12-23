#include "pkcs1.5/pkcs1.5.h"

#include <string.h>

static int unpad_1(uint8_t *out, size_t *outlen, const uint8_t *in, size_t inlen)
{
	size_t i = 1;

	while (i < inlen && in[i] == 0xff)
		++i;
	if (i == inlen || in[i])
		return 0;
	++i;
	memcpy(out, in + i, inlen - i);
	*outlen = inlen - i;
	return 1;
}

static int unpad_2(uint8_t *out, size_t *outlen, const uint8_t *in, size_t inlen)
{
	size_t i = 1;

	while (i < inlen && in[i])
		++i;
	if (i == inlen)
		return 0;
	++i;
	memcpy(out, in + i, inlen - i);
	*outlen = inlen - i;
	return 1;
}

int pkcs1_5_unpad(uint8_t *out, size_t *outlen, const uint8_t *in, size_t inlen)
{
	if (!inlen)
		return 0;
	if (!in[0])
	{
		in++;
		inlen--;
	}
	if (!inlen)
		return 0;
	if (in[0] == 1)
		return unpad_1(out, outlen, in, inlen);
	else if (in[0] == 2)
		return unpad_2(out, outlen, in, inlen);
	return 0;
}
