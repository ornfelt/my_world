#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/bignum.h>
#include <jkssl/dsa.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

struct cmd_dsaparam_data
{
	struct dsa *dsa;
	struct bignum_gencb *gencb;
	enum cmd_format inform;
	enum cmd_format outform;
	size_t key_len;
	FILE *fpout;
	FILE *fpin;
};

static int do_read(struct cmd_dsaparam_data *data)
{
	switch (data->inform)
	{
		case CMD_FORMAT_PEM:
			return pem_read_dsa_params(data->fpin,
			                           &data->dsa,
			                           NULL,
			                           0) != NULL;
		case CMD_FORMAT_DER:
			return d2i_dsa_params_fp(data->fpin,
			                         &data->dsa) != NULL;
		default:
			return 0;
	}
}

static int do_write(struct cmd_dsaparam_data *data)
{
	switch (data->outform)
	{
		case CMD_FORMAT_PEM:
			return pem_write_dsa_params(data->fpout, data->dsa);
		case CMD_FORMAT_DER:
			return i2d_dsa_params_fp(data->fpout, data->dsa);
		default:
			return 0;
	}
}

static void usage(void)
{
	printf("dsaparam [options] [size]\n");
	printf("if size is set, new parameters will be generated (input will be ignored)\n");
	printf("-help:           display this help\n");
	printf("-inform format:  input format (PEM or DER)\n");
	printf("-outform format: output format (PEM or DER)\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
}

static int parse_args(struct cmd_dsaparam_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",    no_argument,       NULL, 'h'},
		{"inform",  required_argument, NULL, 'I'},
		{"outform", required_argument, NULL, 'O'},
		{"in",      required_argument, NULL, 'i'},
		{"out",     required_argument, NULL, 'o'},
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
				if (!cmd_handle_format("dsaparam", optarg,
				                       &data->inform))
					return 0;
				break;
			case 'O':
				if (!cmd_handle_format("dsaparam", optarg,
				                       &data->outform))
					return 0;
				break;
			case 'i':
				if (!cmd_handle_in("dsaparam", optarg,
				                   &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("dsaparam", optarg,
				                    &data->fpout))
					return 0;
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
		fprintf(stderr, "dsaparam: extra operand\n");
		return 0;
	}
	for (size_t j = 0; j < strlen(argv[optind]); ++j)
	{
		if (!isdigit(argv[optind][j]))
		{
			fprintf(stderr, "dsaparam: key size isn't numeric\n");
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

int cmd_dsaparam(int argc, char **argv)
{
	struct cmd_dsaparam_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.inform = CMD_FORMAT_PEM;
	data.outform = CMD_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	data.dsa = dsa_new();
	if (!data.dsa)
	{
		fprintf(stderr, "dsaparam: malloc: %s\n", strerror(errno));
		goto end;
	}
	data.gencb = bignum_gencb_new();
	if (!data.gencb)
	{
		fprintf(stderr, "dsaparam: malloc: %s\n", strerror(errno));
		goto end;
	}
	bignum_gencb_set(data.gencb, gencb_cb, NULL);
	if (!parse_args(&data, argc, argv))
		goto end;
	if (data.key_len)
	{
		if (!dsa_generate_parameters(data.dsa, data.key_len, data.gencb))
		{
			fprintf(stderr, "dsaparam: dsa param creation failed\n");
			goto end;
		}
	}
	else
	{
		if (!do_read(&data))
		{
			fprintf(stderr, "dsaparam: failed to read parameters\n");
			goto end;
		}
	}
	if (!do_write(&data))
	{
		fprintf(stderr, "dsaparam: failed to write PEM parameters\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	dsa_free(data.dsa);
	bignum_gencb_free(data.gencb);
	return ret;
}
