#include <libintl.h>

char *
dcngettext(const char *domain,
           const char *msgid,
           const char *msgid_plural,
           unsigned long n,
           int category)
{
	(void)domain;
	(void)category;
	/* XXX */
	if (n > 1)
		return (char*)msgid_plural;
	return (char*)msgid;
}
