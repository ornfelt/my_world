#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int
main(int argc, char **argv)
{
	const char *fmt = "%a. %d %B %Y %T %Z";
	struct tm tm;
	char str[4096];
	time_t t;

	if (argc > 2)
	{
		fprintf(stderr, "%s: extra operand '%s'\n", argv[0], argv[2]);
		return EXIT_FAILURE;
	}
	if (argc == 2)
	{
		if (argv[1][0] != '%')
		{
			fprintf(stderr, "%s: set not supported\n", argv[0]);
			return EXIT_FAILURE;
		}
		fmt = &argv[1][1];
	}
	t = time(NULL);
	if (!localtime_r(&t, &tm))
	{
		fprintf(stderr, "%s: localtime failed\n", argv[0]);
		return EXIT_FAILURE;
	}
	strftime(str, sizeof(str), fmt, &tm);
	puts(str);
	return EXIT_SUCCESS;
}
