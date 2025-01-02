#include "_wctype.h"

#include <wctype.h>
#include <string.h>

static const char *types_str[] =
{
	[WCTYPE_ALNUM] = "alnum",
	[WCTYPE_ALPHA] = "alpha",
	[WCTYPE_BLANK] = "blank",
	[WCTYPE_CNTRL] = "cntrl",
	[WCTYPE_DIGIT] = "digit",
	[WCTYPE_GRAPH] = "graph",
	[WCTYPE_LOWER] = "lower",
	[WCTYPE_PRINT] = "print",
	[WCTYPE_PUNCT] = "punct",
	[WCTYPE_SPACE] = "space",
	[WCTYPE_UPPER] = "upper",
	[WCTYPE_XDIGIT] = "xdigit",
};

wctype_t wctype(const char *property)
{
	if (!property)
		return WCTYPE_NONE;
	for (size_t i = 0; i < sizeof(types_str) / sizeof(*types_str); ++i)
	{
		if (!types_str[i])
			continue;
		if (!strcmp(types_str[i], property))
			return i;
	}
	return WCTYPE_NONE;
}
