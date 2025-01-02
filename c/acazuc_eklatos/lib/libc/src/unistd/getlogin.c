#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char *getlogin(void)
{
	static char buf[1024];
	char *name = getenv("USER"); /* XXX use /etc/passwd & uid from controlling tty */
	if (!name)
		return NULL;
	strlcpy(buf, name, sizeof(buf));
	return buf;
}
