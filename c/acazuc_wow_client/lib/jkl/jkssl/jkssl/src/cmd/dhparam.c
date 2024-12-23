#include "cmd/cmd.h"

#include <jkssl/bignum.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>
#include <jkssl/dh.h>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

enum dhparam_format
{
	DHPARAM_FORMAT_PEM,
	DHPARAM_FORMAT_DER,
};

struct cmd_dhparam_data
{
	struct dh_ctx *dh_ctx;
	struct bignum_gencb *gencb;
	enum dhparam_format inform;
	enum dhparam_format outform;
	size_t key_len;
	FILE *fpout;
	FILE *fpin;
	size_t generator;
	int text;
	int noout;
};

static int do_read(struct cmd_dhparam_data *data)
{
	switch (data->inform)
	{
		case DHPARAM_FORMAT_PEM:
			return pem_read_dh_params(data->fpin,
			                          &data->dh_ctx,
			                          NULL) != NULL;
		case DHPARAM_FORMAT_DER:
			return d2i_dh_params_fp(data->fpin,
			                        &data->dh_ctx) != NULL;
		default:
			return 0;
	}
}

static int do_write(struct cmd_dhparam_data *data)
{
	switch (data->inform)
	{
		case DHPARAM_FORMAT_PEM:
			return pem_write_dh_params(data->fpout, data->dh_ctx);
		case DHPARAM_FORMAT_DER:
			return i2d_dh_params_fp(data->fpout, data->dh_ctx);
		default:
			return 0;
	}
}

static void print_hex(uint8_t val)
{
	if (val >= 10)
		printf("%c", 'a' + val - 10);
	else
		printf("%c", '0' + val);
}

static void do_print_bignum(struct bignum *bignum)
{
	int n;
	int i;

	n = 0;
	i = bignum_num_bytes(bignum);
	while (i)
	{
		if (!(n % 16))
			printf("            ");
		--i;
		print_hex(((uint8_t*)bignum->data)[i] >> 4);
		print_hex(((uint8_t*)bignum->data)[i] & 0xF);
		if (n % 16 == 15)
			printf("\n");
		else
			printf(":");
		++n;
	}
	if (n % 16)
		printf("\n");
}

static void print_text(struct cmd_dhparam_data *data)
{
	printf("    DH Parameters: (%d bit)\n",
	       bignum_num_bits(data->dh_ctx->p));
	printf("        prime:\n");
	do_print_bignum(data->dh_ctx->p);
	printf("        generator: ");
	bignum_print(data->dh_ctx->g, stdout);
	printf(" (0x");
	bignum_printhex(data->dh_ctx->g, stdout);
	printf(")\n");
}

static int handle_inform_outform(const char *arg, enum dhparam_format *format)
{
	if (!strcmp(arg, "PEM"))
	{
		*format = DHPARAM_FORMAT_PEM;
		return 1;
	}
	if (!strcmp(arg, "DER"))
	{
		*format = DHPARAM_FORMAT_DER;
		return 1;
	}
	fprintf(stderr, "dhparam: unsupported format: %s\n", arg);
	return 0;
}

static void usage(void)
{
	printf("dhparam [options] [size]\n");
	printf("if size is set, new parameters will be generated (input will be ignored)\n");
	printf("-help:           display this help\n");
	printf("-inform format:  input format (PEM or DER)\n");
	printf("-outform format: output format (PEM or DER)\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
	printf("-2:              use 2 as generator\n");
	printf("-5:              use 5 as generator\n");
}

static int parse_args(struct cmd_dhparam_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",    no_argument,       NULL, 'h'},
		{"inform",  required_argument, NULL, 'I'},
		{"outform", required_argument, NULL, 'O'},
		{"in",      required_argument, NULL, 'i'},
		{"out",     required_argument, NULL, 'o'},
		{"text",    no_argument,       NULL, 't'},
		{"noout",   no_argument,       NULL, 'n'},
		{"2",       no_argument,       NULL, '2'},
		{"5",       no_argument,       NULL, '5'},
		{NULL,      0,                 NULL,  0 },
	};
	int c;
	opterr = 1;
	while ((c = getopt_long_only(argc, argv, "", opts, NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				usage();
				return 0;
			case 'I':
				if (!handle_inform_outform(optarg, &data->inform))
					return 0;
				break;
			case 'O':
				if (!handle_inform_outform(optarg, &data->outform))
					return 0;
				break;
			case 'i':
				if (data->fpin != stdin)
					fclose(data->fpin);
				data->fpin = fopen(optarg, "r");
				if (!data->fpin)
				{
					fprintf(stderr, "dhparam: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			case 'o':
				if (data->fpout != stdout)
					fclose(data->fpout);
				data->fpout = fopen(optarg, "w");
				if (!data->fpout)
				{
					fprintf(stderr, "dhparam: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			case 't':
				data->text = 1;
				break;
			case 'n':
				data->noout = 1;
				break;
			case '2':
				data->generator = 2;
				break;
			case '5':
				data->generator = 5;
				break;
			default:
				usage();
				return 0;
		}
	}
	if (optind >= argc)
		return 1;
	if (argc - optind > 1)
	{
		fprintf(stderr, "dhparam: extra operand\n");
		return 0;
	}
	for (size_t j = 0; j < strlen(argv[optind]); ++j)
	{
		if (!isdigit(argv[optind][j]))
		{
			fprintf(stderr, "dhparam: key size isn't numeric\n");
			return 0;
		}
	}
	data->key_len = atol(argv[optind]);
	return 1;
}

static int gencb_cb(int a, int b, struct bignum_gencb *gencb)
{
	(void)gencb;
	if (a == 2)
	{
		fputc('\n', stderr);
		return 1;
	}
	if (a != 1)
		return 1;
	if (b == -1)
	{
		fputc('.', stderr);
		return 1;
	}
	fputc('+', stderr);
	return 1;
}

int cmd_dhparam(int argc, char **argv)
{
	struct cmd_dhparam_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.fpin = stdin;
	data.fpout = stdout;
	data.generator = 2;
	data.dh_ctx = dh_new();
	if (!data.dh_ctx)
	{
		fprintf(stderr, "dhparam: malloc: %s\n", strerror(errno));
		goto end;
	}
	data.gencb = bignum_gencb_new();
	if (!data.gencb)
	{
		fprintf(stderr, "dhparam: malloc: %s\n", strerror(errno));
		goto end;
	}
	bignum_gencb_set(data.gencb, gencb_cb, NULL);
	if (!parse_args(&data, argc, argv))
		goto end;
	if (data.key_len)
	{
		if (!dh_generate_parameters(data.dh_ctx, data.key_len, data.generator, data.gencb))
		{
			fprintf(stderr, "dhparam: dh param creation failed\n");
			goto end;
		}
	}
	else
	{
		if (!do_read(&data))
		{
			fprintf(stderr, "dhparam: failed to read parameters\n");
			goto end;
		}
	}
	if (data.text)
		print_text(&data);
	if (!data.noout && !do_write(&data))
	{
		fprintf(stderr, "dhparam: failed to write PEM parameters\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	dh_free(data.dh_ctx);
	bignum_gencb_free(data.gencb);
	return ret;
}
