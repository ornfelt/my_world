#include "_wcsto.h"

#include <limits.h>
#include <wchar.h>
#include <errno.h>

long
wcstol(const wchar_t *nptr, wchar_t **endptr, int base)
{
	if (base && (base < 2 || base > 36))
	{
		errno = EINVAL;
		return 0;
	}
	int is_neg = 0;
	const wchar_t *it = wcsto_init(nptr, &base, &is_neg);
	unsigned long ret = 0;
	int err = 0;
	for (; *it; it++)
	{
		wchar_t c;
		if (wcsto_getc(it, base, &c))
			break;
		if (err)
			continue;
		unsigned long tmp = ret * base + c;
		if (tmp < ret)
		{
			errno = ERANGE;
			ret = is_neg ? LONG_MIN : LONG_MAX;
			break;
		}
		if (is_neg)
		{
			if (tmp > (unsigned long)LONG_MAX + 1)
			{
				errno = ERANGE;
				ret = LONG_MIN;
				err = 1;
				continue;
			}
		}
		else
		{
			if (tmp > LONG_MAX)
			{
				errno = ERANGE;
				ret = LONG_MAX;
				err = 1;
				continue;
			}
		}
		ret = tmp;
	}
	if (endptr)
		*endptr = (wchar_t*)it;
	if (err)
		return ret;
	return is_neg ? -ret : ret;
}
