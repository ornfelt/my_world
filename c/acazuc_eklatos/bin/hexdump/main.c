#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define OPT_b (1 << 0)
#define OPT_c (1 << 1)
#define OPT_C (1 << 2)
#define OPT_d (1 << 3)
#define OPT_o (1 << 4)
#define OPT_x (1 << 5)

struct env
{
	const char *progname;
	int opt;
};

static char hexchar(uint8_t v)
{
	if (v < 10)
		return '0' + v;
	return 'a' + v - 10;
}

static char octchar(uint8_t v)
{
	return '0' + v;
}

static void print_octal(uint32_t sum, const uint8_t *buf, size_t rd)
{
	printf("%07" PRIx32, sum);
	for (size_t i = 0; i < rd; ++i)
	{
		putchar(' ');
		putchar(octchar((buf[i] >> 6) & 0x7));
		putchar(octchar((buf[i] >> 3) & 0x7));
		putchar(octchar((buf[i] >> 0) & 0x7));
	}
	putchar('\n');
}

static void print_ascii(uint32_t sum, const uint8_t *buf, size_t rd)
{
	printf("%07" PRIx32, sum);
	for (size_t i = 0; i < rd; ++i)
	{
		fputs("   ", stdout);
		if (buf[i] < 0x20 || buf[i] >= 0x7F)
			putchar('.');
		else
			putchar(buf[i]);
	}
	putchar('\n');
}

static void print_decimal(uint32_t sum, const uint8_t *buf, size_t rd)
{
	printf("%07" PRIx32, sum);
	for (size_t i = 0; i < rd; i += 2)
	{
		fputs("   ", stdout);
		uint16_t v = buf[i] | ((uint16_t)buf[i + 1] << 8);
		printf("%05" PRIu16, v);
	}
	putchar('\n');
}

static void print_double_octal(uint32_t sum, const uint8_t *buf, size_t rd)
{
	printf("%07" PRIx32, sum);
	for (size_t i = 0; i < rd; i += 2)
	{
		putchar(' ');
		putchar(' ');
		putchar(octchar((buf[i + 1] >> 7) & 0x1));
		putchar(octchar((buf[i + 1] >> 4) & 0x7));
		putchar(octchar((buf[i + 1] >> 1) & 0x7));
		putchar(octchar(((buf[i + 0] >> 6) & 0x3) | ((buf[i + 1] << 2) & 0x4)));
		putchar(octchar((buf[i + 0] >> 3) & 0x7));
		putchar(octchar((buf[i + 0] >> 0) & 0x7));
	}
	putchar('\n');
}

static void print_double_hex(uint32_t sum, const uint8_t *buf, size_t rd)
{
	printf("%07" PRIx32, sum);
	for (size_t i = 0; i < rd; i += 2)
	{
		fputs("    ", stdout);
		putchar(hexchar((buf[i + 1] >> 4) & 0xF));
		putchar(hexchar((buf[i + 1] >> 0) & 0xF));
		putchar(hexchar((buf[i + 0] >> 4) & 0xF));
		putchar(hexchar((buf[i + 0] >> 0) & 0xF));
	}
	putchar('\n');
}

static void print_classic(uint32_t sum, const uint8_t *buf, size_t rd)
{
	printf("%07" PRIx32, sum);
	for (size_t i = 0; i < rd; i += 2)
	{
		putchar(' ');
		putchar(hexchar((buf[i + 1] >> 4) & 0xF));
		putchar(hexchar((buf[i + 1] >> 0) & 0xF));
		putchar(hexchar((buf[i + 0] >> 4) & 0xF));
		putchar(hexchar((buf[i + 0] >> 0) & 0xF));
	}
	putchar('\n');
}

static void print_canonical(uint32_t sum, const uint8_t *buf, size_t rd)
{
	printf("%08" PRIx32, sum);
	for (size_t i = 0; i < rd; ++i)
	{
		if (!(i & 7))
			putchar(' ');
		putchar(' ');
		putchar(hexchar((buf[i] >> 4) & 0xF));
		putchar(hexchar((buf[i] >> 0) & 0xF));
	}
	for (size_t i = rd; i < 16; ++i)
	{
		if (!(i & 7))
			putchar(' ');
		fputs("   ", stdout);
	}
	fputs("  |", stdout);
	for (size_t i = 0; i < rd; ++i)
	{
		if (buf[i] < 0x20 || buf[i] >= 0x7F)
			putchar('.');
		else
			putchar(buf[i]);
	}
	fputs("|\n", stdout);
}

static int hexdump_fp(struct env *env, FILE *fp)
{
	uint32_t sum = 0;
	uint8_t buf[16];
	while (1)
	{
		size_t rd = fread(buf, 1, 16, fp);
		if (ferror(fp))
		{
			fprintf(stderr, "%s: read: %s\n", env->progname,
			        strerror(errno));
			return 1;
		}
		for (size_t i = rd; i < 16; ++i)
			buf[i] = 0;
		if (!rd && feof(fp))
		{
			if (env->opt & OPT_C)
				printf("%08" PRIx32, sum);
			else
				printf("%07" PRIx32, sum);
			putchar('\n');
			break;
		}
		if (env->opt & OPT_b)
			print_octal(sum, buf, rd);
		if (env->opt & OPT_c)
			print_ascii(sum, buf, rd);
		if (env->opt & OPT_d)
			print_decimal(sum, buf, rd);
		if (env->opt & OPT_C)
			print_canonical(sum, buf, rd);
		if (env->opt & OPT_o)
			print_double_octal(sum, buf, rd);
		if (env->opt & OPT_x)
			print_double_hex(sum, buf, rd);
		if (!(env->opt & (OPT_b | OPT_c | OPT_C | OPT_d | OPT_o | OPT_x)))
			print_classic(sum, buf, rd);
		sum += rd;
	}
	return 0;
}

static int hexdump_file(struct env *env, const char *file)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		fprintf(stderr, "%s: open: %s\n", env->progname,
		        strerror(errno));
		return 1;
	}
	int ret = hexdump_fp(env, fp);
	fclose(fp);
	return ret;
}

static void usage(const char *progname)
{
	printf("%s [-b] [-c] [-C] [-d] [-o] [-x] [-h] [FILES]\n", progname);
	printf("-b: octal display\n");
	printf("-c: character display\n");
	printf("-C: canonical display\n");
	printf("-d: decimal display\n");
	printf("-o: double-octal display\n");
	printf("-x: double-hexadecimal display\n");
	printf("-h: show this help\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hbcCdox")) != -1)
	{
		switch (c)
		{
			case 'b':
				env.opt |= OPT_b;
				break;
			case 'c':
				env.opt |= OPT_c;
				break;
			case 'C':
				env.opt |= OPT_C;
				break;
			case 'd':
				env.opt |= OPT_d;
				break;
			case 'o':
				env.opt |= OPT_o;
				break;
			case 'x':
				env.opt |= OPT_x;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind == argc)
	{
		if (hexdump_fp(&env, stdin))
			return EXIT_FAILURE;
	}
	else
	{
		for (int i = optind; i < argc; ++i)
		{
			if (hexdump_file(&env, argv[i]))
				return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
