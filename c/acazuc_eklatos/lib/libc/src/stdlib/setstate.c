#include "_rand.h"

#include <stdlib.h>

char *
setstate(char *state)
{
	char *ret;

	ret = (char*)random_val;
	random_val = (uint32_t*)state;
	return ret;
}
