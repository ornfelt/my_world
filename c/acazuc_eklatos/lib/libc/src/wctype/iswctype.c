#include "_wctype.h"

#include <wctype.h>

static int (*types_fn[])(wint_t) =
{
	[WCTYPE_ALNUM] = iswalnum,
	[WCTYPE_ALPHA] = iswalpha,
	[WCTYPE_BLANK] = iswblank,
	[WCTYPE_CNTRL] = iswcntrl,
	[WCTYPE_DIGIT] = iswdigit,
	[WCTYPE_GRAPH] = iswgraph,
	[WCTYPE_LOWER] = iswlower,
	[WCTYPE_PRINT] = iswprint,
	[WCTYPE_PUNCT] = iswpunct,
	[WCTYPE_SPACE] = iswspace,
	[WCTYPE_UPPER] = iswupper,
	[WCTYPE_XDIGIT] = iswxdigit,
};

int
iswctype(wint_t wc, wctype_t type)
{
	if (type >= sizeof(types_fn) / sizeof(*types_fn))
		return 0;
	if (!types_fn[type])
		return 0;
	return types_fn[type](wc);
}
