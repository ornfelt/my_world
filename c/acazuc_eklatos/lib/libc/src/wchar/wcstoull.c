#include "_wcsto.h"

#include <limits.h>
#include <wchar.h>
#include <errno.h>

unsigned long long
wcstoull(const wchar_t *nptr, wchar_t **endptr, int base)
{
	if (base && (base < 2 || base > 36))
	{
		errno = EINVAL;
		return 0;
	}
	int is_neg = 0;
	const wchar_t *it = wcsto_init(nptr, &base, &is_neg);
	unsigned long long ret = 0;
	int err = 0;
	for (; *it; it++)
	{
		wchar_t c;
		if (wcsto_getc(it, base, &c))
			break;
		if (err)
			continue;
		unsigned long long tmp = ret * base + c;
		if (tmp < ret)
		{
			errno = ERANGE;
			ret = ULLONG_MAX;
			err = 1;
			continue;
		}
		ret = tmp;
	}
	if (endptr)
		*endptr = (wchar_t*)it;
	return is_neg ? ~(ret - 1) : ret;
}
