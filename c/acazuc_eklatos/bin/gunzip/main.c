#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <zlib.h>

static int inflate_file(const char *progname, const char *file)
{
	gzFile gzfile;
	uint8_t buf[4096];
	int ret = 1;

	gzfile = gzopen(file, "rb");
	if (!gzfile)
	{
		fprintf(stderr, "%s: gzopen: %s\n", progname, strerror(errno));
		return 1;
	}
	while (1)
	{
		int rd = gzread(gzfile, buf, sizeof(buf));
		if (rd < 0)
			goto end;
		if (!rd)
			break;
		fwrite(buf, 1, rd, stdout);
	}
	ret = 0;

end:
	gzclose(gzfile);
	return ret;
}

static void usage(const char *progname)
{
	printf("%s FILES\n", progname);
}

int main(int argc, char **argv)
{
	int ret;

	if (argc < 2)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	ret = EXIT_SUCCESS;
	for (int i = 1; i < argc; ++i)
	{
		if (inflate_file(argv[0], argv[i]))
			ret = EXIT_FAILURE;
	}
	return ret;
}
