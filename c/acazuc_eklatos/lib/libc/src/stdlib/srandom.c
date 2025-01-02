#include "_rand.h"

#include <stdlib.h>

void srandom(unsigned seed)
{
	*random_val = seed;
}
