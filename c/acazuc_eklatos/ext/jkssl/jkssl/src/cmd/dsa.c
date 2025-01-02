#include "cmd/common.h"
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

struct cmd_dsa_data
{
	struct dsa *dsa;
	enum cmd_format inform;
	enum cmd_format outform;
	char *passin;
	char *passout;
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
		case CMD_FORMAT_PEM:
			if (data->pubin)
				return pem_read_dsa_pubkey(data->fpin, 
				                           &data->dsa,
				                           data->passin ? NULL : cmd_ask_pass,
				                           data->passin) != NULL;
			return pem_read_dsa_private_key(data->fpin,
			                                &data->dsa,
			                                data->passin ? NULL : cmd_ask_pass,
			                                data->passin) != NULL;
		case CMD_FORMAT_DER:
			if (data->pubin)
				return d2i_dsa_pubkey_fp(data->fpin,
				                         &data->dsa) != NULL;
			return d2i_dsa_private_key_fp(data->fpin,
			                              &data->dsa) != NULL;
		default:
			return 0;
	}
}

static int do_write(struct cmd_dsa_data *data)
{
	switch (data->outform)
	{
		case CMD_FORMAT_PEM:
			if (data->pubout)
				return pem_write_dsa_pubkey(data->fpout,
				                            data->dsa);
			return pem_write_dsa_private_key(data->fpout,
			                                 data->dsa,
			                                 data->cipher,
			                                 data->passout ? NULL : cmd_ask_pass,
			                                 data->passout);
		case CMD_FORMAT_DER:
			if (data->pubout)
				return i2d_dsa_pubkey_fp(data->fpout,
				                         data->dsa);
			return i2d_dsa_private_key_fp(data->fpout,
			                              data->dsa);
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

static int do_print_bignum(const struct bignum *bignum, char *name)
{
	uint8_t *data;
	int n;
	int i;

	printf("%s:\n", name);
	n = 0;
	i = bignum_num_bytes(bignum);
	if (!i)
	{
		printf("00:\n");
		return 1;
	}
	data = malloc(i);
	if (!data)
	{
		fprintf(stderr, "dsa: allocation failed\n");
		return 0;
	}
	bignum_bignum2bin(bignum, data);
	while (i)
	{
		--i;
		print_hex(data[i] >> 4);
		print_hex(data[i] & 0xF);
		if (n % 16 == 15)
			printf("\n");
		else
			printf(":");
		++n;
	}
	if (n % 16)
		printf("\n");
	free(data);
	return 1;
}

static int print_text(struct cmd_dsa_data *data)
{
	const struct bignum *p;
	const struct bignum *q;
	const struct bignum *g;
	const struct bignum *x;
	const struct bignum *y;

	dsa_get0_pqg(data->dsa, &p, &q, &g);
	dsa_get0_key(data->dsa, &y, &x);
	if (data->pubin)
	{
		if (!do_print_bignum(y, "pub")
		 || !do_print_bignum(p, "P")
		 || !do_print_bignum(q, "Q")
		 || !do_print_bignum(g, "G"))
			return 0;
	}
	else
	{
		if (!do_print_bignum(x, "priv")
		 || !do_print_bignum(y, "pub")
		 || !do_print_bignum(p, "P")
		 || !do_print_bignum(q, "Q")
		 || !do_print_bignum(g, "G"))
			return 0;
	}
	return 1;
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
				if (!cmd_handle_format("dsa", optarg,
				                       &data->inform))
					return 0;
				break;
			case 'O':
				if (!cmd_handle_format("dsa", optarg,
				                       &data->outform))
					return 0;
				break;
			case 'p':
				if (!cmd_handle_pass("dsa", optarg,
				                     &data->passin))
					return 0;
				break;
			case 'P':
				if (!cmd_handle_pass("dsa", optarg,
				                     &data->passout))
					return 0;
				break;
			case 'i':
				if (!cmd_handle_in("dsa", optarg,
				                   &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("dsa", optarg,
				                    &data->fpout))
					return 0;
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
	data.inform = CMD_FORMAT_PEM;
	data.outform = CMD_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	data.dsa = dsa_new();
	if (!data.dsa)
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
	if (data.text && !print_text(&data))
		goto end;
	if (data.modulus)
	{
		printf("Modulus=");
		bignum_printhex(dsa_get0_p(data.dsa), stdout);
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
	free(data.passin);
	free(data.passout);
	dsa_free(data.dsa);
	return ret;
}
