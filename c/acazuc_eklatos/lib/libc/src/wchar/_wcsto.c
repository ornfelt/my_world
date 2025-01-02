#include "_wcsto.h"

#include <wctype.h>

const wchar_t *wcsto_init(const wchar_t *nptr, int *base, int *is_neg)
{
	const wchar_t *it = nptr;
	while (iswspace(*it))
		it++;
	if (*it == L'+')
	{
		it++;
	}
	else if (*it == L'-')
	{
		*is_neg = 1;
		it++;
	}
	if (*base != 0)
		return it;
	if (*it == L'0')
	{
		if (it[1] == L'x' || it[1] == L'X')
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

int wcsto_getc(const wchar_t *it, int base, wchar_t *c)
{
	*c = towlower(*it);
	if (*c < L'0')
		return 1;
	if (*c > L'9')
	{
		if (*c < L'a')
			return 1;
		if (*c > L'z')
			return 1;
		*c = *c - L'a' + 10;
	}
	else
	{
		*c = *c - L'0';
	}
	return *c >= base;
}
