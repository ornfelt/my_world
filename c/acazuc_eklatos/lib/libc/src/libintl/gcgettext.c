#include <libintl.h>

char *dcgettext(const char *domain, const char *msgid, int category)
{
	(void)domain;
	(void)category;
	/* XXX */
	return (char*)msgid;
}
