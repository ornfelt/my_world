#include <zlib.h>

uint32_t
adler32(uint32_t adler, const uint8_t *buf, unsigned len)
{
	/* XXX can certainly be faster... */
	if (!buf)
		return 1;
	uint32_t s1 = (adler >> 0) & 0xFFFF;
	uint32_t s2 = (adler >> 16) & 0xFFFF;
	for (unsigned i = 0; i < len; ++i)
	{
		s1 = (s1 + buf[i]) % 65521;
		s2 = (s2 + s1) % 65521;
	}
	return (s2 << 16) | s1;
}

uint32_t
adler32_z(uint32_t adler, const uint8_t *buf, size_t len)
{
	/* XXX can certainly be faster... */
	if (!buf)
		return 1 << 16;
	uint32_t s1 = (adler >> 0) & 0xFFFF;
	uint32_t s2 = (adler >> 16) & 0xFFFF;
	for (size_t i = 0; i < len; ++i)
	{
		s1 = (s1 + buf[i]) % 65521;
		s2 = (s2 + s1) % 65521;
	}
	return (s2 << 16) | s1;
}
