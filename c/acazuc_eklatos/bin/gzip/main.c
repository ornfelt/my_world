#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <zlib.h>

static int deflate_file(const char *progname, const char *file)
{
	gzFile gzfile;
	uint8_t buf[4096];
	FILE *fp = NULL;
	int ret = 1;

	gzfile = gzdopen(1, "wb");
	if (!gzfile)
	{
		fprintf(stderr, "%s: gzdopen: %s\n", progname, strerror(errno));
		return 1;
	}
	fp = fopen(file, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: open(%s): %s\n", progname, file,
		        strerror(errno));
		goto end;
	}
	while (!feof(fp))
	{
		size_t rd = fread(buf, 1, sizeof(buf), fp);
		if (ferror(fp))
			goto end;
		int wr = gzwrite(gzfile, buf, rd);
		if (wr < 0)
			goto end;
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
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
		if (deflate_file(argv[0], argv[i]))
			ret = EXIT_FAILURE;
	}
	return ret;
}
