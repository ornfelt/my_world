#include <eklat/reboot.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv)
{
	(void)argc;
	int ret = reboot(REBOOT_SUSPEND);
	if (ret)
	{
		fprintf(stderr, "%s: reboot: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
