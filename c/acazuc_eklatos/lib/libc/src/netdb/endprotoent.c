#include "_protoent.h"

#include <netdb.h>

void
endprotoent(void)
{
	if (!protoent_fp)
		return;
	fclose(protoent_fp);
	protoent_fp = NULL;
}
