#include <ctype.h>

int isspace(int c)
{
	return c == ' '  || c == '\t'
	    || c == '\n' || c == '\f'
	    || c == '\r' || c == '\v';
}
