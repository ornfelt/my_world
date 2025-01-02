#include <wctype.h>
#include <ctype.h>

int iswcntrl(wint_t wc)
{
	 /* XXX */
	return iscntrl(wc);
}
