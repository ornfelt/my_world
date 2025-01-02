#include <stdlib.h>

ldiv_t ldiv(long num, long dem)
{
	ldiv_t ret;
	ret.quot = num / dem;
	ret.rem = num % dem;
	return ret;
}
