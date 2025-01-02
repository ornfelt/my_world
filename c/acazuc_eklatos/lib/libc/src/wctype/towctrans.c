#include "_wctype.h"

#include <wctype.h>

static wint_t (*trans_fn[])(wint_t) =
{
	[WCTRANS_TOLOWER] = towlower,
	[WCTRANS_TOUPPER] = towupper,
};

wint_t towctrans(wint_t wc, wctrans_t trans)
{
	if (trans >= sizeof(trans_fn) / sizeof(*trans_fn))
		return 0;
	if (!trans_fn[trans])
		return 0;
	return trans_fn[trans](wc);
}
