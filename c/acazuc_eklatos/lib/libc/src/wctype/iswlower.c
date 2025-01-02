#include <wctype.h>
#include <ctype.h>

int iswlower(wint_t wc)
{
	/* XXX */
	return islower(wc);
}
