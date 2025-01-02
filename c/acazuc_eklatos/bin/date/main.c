#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char **argv)
{
	const char *fmt = "%a. %d %B %Y %T %Z";
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
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char str[4096];
	strftime(str, sizeof(str), fmt, tm);
	puts(str);
	return EXIT_SUCCESS;
}
