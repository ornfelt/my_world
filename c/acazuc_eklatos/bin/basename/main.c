#include <sys/param.h>

#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i)
	{
		char buf[MAXPATHLEN];

		if (strlcpy(buf, argv[i], sizeof(buf)) >= sizeof(buf))
		{
			fprintf(stderr, "%s: argument too long\n", argv[0]);
			return EXIT_FAILURE;
		}
		puts(basename(buf));
	}
	return EXIT_SUCCESS;
}
