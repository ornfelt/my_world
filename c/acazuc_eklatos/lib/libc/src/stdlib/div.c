#include <stdlib.h>

div_t
div(int num, int dem)
{
	div_t ret;

	ret.quot = num / dem;
	ret.rem = num % dem;
	return ret;
}
