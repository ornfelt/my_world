#include "_rand.h"

#include <stdlib.h>

char *initstate(unsigned seed, char *state, size_t n)
{
	if (n < 4)
		return NULL;
	char *ret = (char*)random_val;
	random_val = (uint32_t*)state;
	*random_val = seed;
	return ret;
}
