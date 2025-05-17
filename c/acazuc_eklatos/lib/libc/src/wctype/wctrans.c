#include "_wctype.h"

#include <wctype.h>
#include <string.h>

static const char *trans_str[] =
{
	[WCTRANS_TOLOWER] = "tolower",
	[WCTRANS_TOUPPER] = "toupper",
};

wctrans_t
wctrans(const char *property)
{
	if (!property)
		return WCTRANS_NONE;
	for (size_t i = 0; i < sizeof(trans_str) / sizeof(*trans_str); ++i)
	{
		if (!trans_str[i])
			continue;
		if (!strcmp(trans_str[i], property))
			return i;
	}
	return WCTRANS_NONE;
}
