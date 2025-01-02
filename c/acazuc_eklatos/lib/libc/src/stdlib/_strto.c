#include "_strto.h"

#include <ctype.h>

const char *strto_init(const char *nptr, int *base, int *is_neg)
{
	const char *it = nptr;
	while (isspace(*it))
		it++;
	if (*it == '+')
	{
		it++;
	}
	else if (*it == '-')
	{
		*is_neg = 1;
		it++;
	}
	if (*base != 0)
		return it;
	if (*it == '0')
	{
		if (it[1] == 'x' || it[1] == 'X')
		{
			*base = 16;
			return it + 2;
		}
		else
		{
			*base = 8;
			return it + 1;
		}
	}
	else
	{
		*base = 10;
	}
	return it;
}

int strto_getc(const char *it, int base, char *c)
{
	*c = tolower(*it);
	if (*c < '0')
		return 1;
	if (*c > '9')
	{
		if (*c < 'a')
			return 1;
		if (*c > 'z')
			return 1;
		*c = *c - 'a' + 10;
	}
	else
	{
		*c = *c - '0';
	}
	return *c >= base;
}
