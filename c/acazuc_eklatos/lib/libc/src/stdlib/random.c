#include "_rand.h"

#include <stdlib.h>

long
random(void)
{
	*random_val = (*random_val * 1103515245 + 12345) & 0x7FFFFFF;
	return *random_val;
}
