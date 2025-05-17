#include "_hostent.h"

#include <netdb.h>

void
endhostent(void)
{
	if (!hostent_fp)
		return;
	fclose(hostent_fp);
	hostent_fp = NULL;
}
