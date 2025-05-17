#include <eklat/reboot.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int
main(int argc, char **argv)
{
	(void)argc;
	if (reboot(REBOOT_SHUTDOWN))
	{
		fprintf(stderr, "%s: reboot: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
