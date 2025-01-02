#include <inttypes.h>

imaxdiv_t imaxdiv(intmax_t num, intmax_t dem)
{
	imaxdiv_t ret;
	ret.quot = num / dem;
	ret.rem = num % dem;
	return ret;
}
