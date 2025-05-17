#include "_rand.h"

#include <stdlib.h>

char *
initstate(unsigned seed, char *state, size_t n)
{
	char *ret;

	if (n < 4)
		return NULL;
	ret = (char*)random_val;
	random_val = (uint32_t*)state;
	*random_val = seed;
	return ret;
}
