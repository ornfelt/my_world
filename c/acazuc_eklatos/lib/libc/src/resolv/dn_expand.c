#include <resolv.h>
#include <string.h>

int
dn_expand(const uint8_t *msg,
          const uint8_t *eom,
          const uint8_t *dn,
          char *str,
          int str_len)
{
	size_t n = 0;
	int ref = 0;
	int first = 1;

	if (dn >= eom)
		return -1;
	if (!str_len)
		return -1;
	while (1)
	{
		if (dn >= eom)
			return -1;
		uint8_t len = *(dn++);
		if (!len)
		{
			*str = 0;
			if (!ref)
				n++;
			return n;
		}
		if (len >= 0xC0)
		{
			if (ref) /* no recursion */
				return -1;
			if (dn + 1 > eom)
				return -1;
			uint16_t off = (len & ~0xC0) << 8;
			off |= *(dn++);
			dn = &msg[off];
			ref = 1;
			n += 2;
			continue;
		}
		if (len > 63)
			return -1;
		if (first)
		{
			first = 0;
		}
		else
		{
			if (str_len > 1)
			{
				*(str++) = '.';
				str_len--;
			}
		}
		int cpy_len = len;
		if (cpy_len >= str_len)
			cpy_len = str_len - 1;
		memcpy(str, dn, cpy_len);
		dn += len;
		str += cpy_len;
		if (!ref)
			n += len + 1;
	}
}
