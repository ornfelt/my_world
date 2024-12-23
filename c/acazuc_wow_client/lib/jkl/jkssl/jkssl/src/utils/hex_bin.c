#include "utils.h"

char bin2char(uint8_t val)
{
	if (val >= 10)
		return 'a' + val - 10;
	return '0' + val;
}

void bin2hex(char *dst, const uint8_t *src, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		dst[i * 2 + 0] = bin2char(src[i] >> 4);
		dst[i * 2 + 1] = bin2char(src[i] & 0xf);
	}
}

int ishex(char c)
{
	if (c >= 'a' && c <= 'f')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

uint8_t char2bin(char c)
{
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return c - '0';
}

int hex2bin(uint8_t *dst, const char *src, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		if (!ishex(src[i]))
			return 0;
		if (i & 1)
			dst[i / 2] = (dst[i / 2] << 4) | char2bin(src[i]);
		else
			dst[i / 2] = char2bin(src[i]);
	}
	return 1;
}
