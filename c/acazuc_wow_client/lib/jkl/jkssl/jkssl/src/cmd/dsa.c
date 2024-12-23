#include "cmd/cmd.h"

#include <jkssl/bignum.h>
#include <jkssl/evp.h>
#include <jkssl/dsa.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

enum dsa_format
{
	DSA_FORMAT_PEM,
	DSA_FORMAT_DER,
};

struct cmd_dsa_data
{
	struct dsa_ctx *dsa_ctx;
	enum dsa_format inform;
	enum dsa_format outform;
	const char *passin;
	const char *passout;
	FILE *fpout;
	FILE *fpin;
	int pubin;
	int pubout;
	int text;
	int modulus;
	int noout;
	const struct evp_cipher *cipher;
};

static int do_read(struct cmd_dsa_data *data)
{
	switch (data->inform)
	{
		case DSA_FORMAT_PEM:
			if (data->pubin)
				return pem_read_dsa_pubkey(data->fpin, 
				                           &data->dsa_ctx,
				                           data->passin) != NULL;
			return pem_read_dsa_private_key(data->fpin,
			                                &data->dsa_ctx,
			                                data->passin) != NULL;
		case DSA_FORMAT_DER:
			if (data->pubin)
				return d2i_dsa_pubkey_fp(data->fpin,
				                         &data->dsa_ctx) != NULL;
			return d2i_dsa_private_key_fp(data->fpin,
			                              &data->dsa_ctx) != NULL;
		default:
			return 0;
	}
}

static int do_write(struct cmd_dsa_data *data)
{
	switch (data->outform)
	{
		case DSA_FORMAT_PEM:
			if (data->pubout)
				return pem_write_dsa_pubkey(data->fpout,
				                            data->dsa_ctx);
			return pem_write_dsa_private_key(data->fpout,
			                                 data->dsa_ctx,
			                                 data->cipher,
			                                 data->passout);
		case DSA_FORMAT_DER:
			if (data->pubout)
				return i2d_dsa_pubkey_fp(data->fpout,
				                         data->dsa_ctx);
			return i2d_dsa_private_key_fp(data->fpout,
			                              data->dsa_ctx);
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

static void do_print_bignum(struct bignum *bignum, char *name)
{
	int n;
	int i;

	printf("%s:\n", name);
	n = 0;
	i = bignum_num_bytes(bignum);
	while (i)
	{
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

static void print_text(struct cmd_dsa_data *data)
{
	if (data->pubin)
	{
		do_print_bignum(data->dsa_ctx->y, "pub");
		do_print_bignum(data->dsa_ctx->p, "P");
		do_print_bignum(data->dsa_ctx->q, "Q");
		do_print_bignum(data->dsa_ctx->g, "G");
	}
	else
	{
		do_print_bignum(data->dsa_ctx->x, "priv");
		do_print_bignum(data->dsa_ctx->y, "pub");
		do_print_bignum(data->dsa_ctx->p, "P");
		do_print_bignum(data->dsa_ctx->q, "Q");
		do_print_bignum(data->dsa_ctx->g, "G");
	}
}

static int handle_inform_outform(const char *arg, enum dsa_format *format)
{
	if (!strcmp(arg, "PEM"))
	{
		*format = DSA_FORMAT_PEM;
		return 1;
	}
	if (!strcmp(arg, "DER"))
	{
		*format = DSA_FORMAT_DER;
		return 1;
	}
	fprintf(stderr, "dsa: unsupported format: %s\n", arg);
	return 0;
}

static void usage(void)
{
	printf("dsa [options]\n");
	printf("-help:           display this help\n");
	printf("-inform format:  input format (PEM or DER)\n");
	printf("-outform format: output format (PEM or DER)\n");
	printf("-passin pass:    input password\n");
	printf("-passout pass:   output password\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
	printf("-pubin:          input file in pubkey\n");
	printf("-pubout:         output file as pubkey\n");
	printf("-text:           print text representation of key\n");
	printf("-modulus:        print modulus of key\n");
	printf("-noout:          don't output key\n");
	printf("-*:              algorithm to protect private key\n");
}

static int parse_args(struct cmd_dsa_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",        no_argument,       NULL, 'h'},
		{"inform",      required_argument, NULL, 'I'},
		{"outform",     required_argument, NULL, 'O'},
		{"passin",      required_argument, NULL, 'p'},
		{"passout",     required_argument, NULL, 'P'},
		{"in",          required_argument, NULL, 'i'},
		{"out",         required_argument, NULL, 'o'},
		{"pubin",       no_argument,       NULL, 'x'},
		{"pubout",      no_argument,       NULL, 'X'},
		{"text",        no_argument,       NULL, 't'},
		{"modulus",     no_argument,       NULL, 'm'},
		{"noout",       no_argument,       NULL, 'n'},
		{NULL,          0,                 NULL,  0 },
	};
	int c;
	opterr = 0;
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
			case 'p':
				data->passin = optarg;
				break;
			case 'P':
				data->passout = optarg;
				break;
			case 'i':
				if (data->fpin != stdin)
					fclose(data->fpin);
				data->fpin = fopen(optarg, "r");
				if (!data->fpin)
				{
					fprintf(stderr, "dsa: open(%s): %s\n",
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
					fprintf(stderr, "dsa: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			case 'x':
				data->pubin = 1;
				data->pubout = 1;
				break;
			case 'X':
				data->pubout = 1;
				break;
			case 't':
				data->text = 1;
				break;
			case 'm':
				data->modulus = 1;
				break;
			case 'n':
				data->noout = 1;
				break;
			default:
				data->cipher = evp_get_cipherbyname(argv[optind - 1] + 1);
				if (!data->cipher)
				{
					fprintf(stderr, "dsa: unrecognized option: %s\n",
					        argv[optind - 1]);
					usage();
					return 0;
				}
				break;
		}
	}
	return 1;
}

int cmd_dsa(int argc, char **argv)
{
	struct cmd_dsa_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.fpin = stdin;
	data.fpout = stdout;
	data.dsa_ctx = dsa_new();
	if (!data.dsa_ctx)
	{
		fprintf(stderr, "dsa: malloc: %s\n", strerror(errno));
		goto end;
	}
	if (!parse_args(&data, argc, argv))
		goto end;
	if (!do_read(&data))
	{
		fprintf(stderr, "dsa: invalid key\n");
		goto end;
	}
	if (data.text)
		print_text(&data);
	if (data.modulus)
	{
		printf("Modulus=");
		bignum_printhex(data.dsa_ctx->p, stdout);
		printf("\n");
	}
	if (!data.noout && !do_write(&data))
	{
		fprintf(stderr, "dsa: failed to write key\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	dsa_free(data.dsa_ctx);
	return ret;
}
