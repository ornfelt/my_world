#include <ctype.h>

int
iscntrl(int c)
{
	return c < ' ' || c > '~';
}
