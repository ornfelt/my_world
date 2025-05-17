#include "_rand.h"

#include <stdlib.h>

int
rand(void)
{
	rand_val = (rand_val * 1103515245 + 12345) & 0x7FFFFFF;
	return rand_val;
}
