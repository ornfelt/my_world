#include "_strto.h"

#include <stdlib.h>
#include <limits.h>
#include <errno.h>

unsigned long strtoul(const char *nptr, char **endptr, int base)
{
	if (base && (base < 2 || base > 36))
	{
		errno = EINVAL;
		return 0;
	}
	int is_neg = 0;
	const char *it = strto_init(nptr, &base, &is_neg);
	unsigned long ret = 0;
	int err = 0;
	for (; *it; it++)
	{
		char c;
		if (strto_getc(it, base, &c))
			break;
		if (err)
			continue;
		unsigned long tmp = ret * base + c;
		if (tmp < ret)
		{
			errno = ERANGE;
			ret = ULONG_MAX;
			err = 1;
			continue;
		}
		ret = tmp;
	}
	if (endptr)
		*endptr = (char*)it;
	return is_neg ? ~(ret - 1) : ret;
}
