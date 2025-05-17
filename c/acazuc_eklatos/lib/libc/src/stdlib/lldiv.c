#include <stdlib.h>

lldiv_t
lldiv(long long num, long long dem)
{
	lldiv_t ret;

	ret.quot = num / dem;
	ret.rem = num % dem;
	return ret;
}
