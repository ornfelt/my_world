#include "_rand.h"

#include <stdlib.h>

long
lrand48(void)
{
	return nrand48(rand48_x);
}
