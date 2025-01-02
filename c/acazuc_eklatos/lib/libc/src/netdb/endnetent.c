#include "_netent.h"

#include <netdb.h>

void endnetent(void)
{
	if (!netent_fp)
		return;
	fclose(netent_fp);
	netent_fp = NULL;
}
