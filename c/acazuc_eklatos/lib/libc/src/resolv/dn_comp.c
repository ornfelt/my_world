#include <resolv.h>
#include <string.h>

int
dn_comp(const char *dn,
        uint8_t *dst,
        int len,
        uint8_t **dnptrs,
        uint8_t **lastdnptr)
{
	const uint8_t *org;

	(void)dnptrs; /* XXX use */
	(void)lastdnptr; /* XXX use */
	org = dst;
	while (1)
	{
		char *iter;
		int n;

		iter = strchrnul(dn, '.');
		n = iter - dn;
		if (n == 0 || n > 63)
			return -1;
		if (len < 1 + n)
			return -1;
		*dst = n;
		dst++;
		len--;
		memcpy(dst, dn, n);
		dst += n;
		len -= n;
		if (!*iter)
			break;
		dn = iter + 1;
	}
	if (len < 1)
		return -1;
	*dst = 0;
	return dst - org + 1;
}
