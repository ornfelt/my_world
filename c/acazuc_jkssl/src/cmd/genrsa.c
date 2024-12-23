#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/bignum.h>
#include <jkssl/evp.h>
#include <jkssl/rsa.h>
#include <jkssl/pem.h>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

struct cmd_genrsa_data
{
	struct rsa *rsa;
	struct bignum_gencb *gencb;
	uint64_t key_len;
	FILE *fpout;
	const struct evp_cipher *cipher;
	char *passout;
	uint32_t exp;
};

static void usage(void)
{
	printf("genrsa [options] size\n");
	printf("-help:         display this help\n");
	printf("-out file:     output file\n");
	printf("-f4, -F4:      use 65537 as exponent\n");
	printf("-3:            use 3 as exponent\n");
	printf("-passout pass: pass to protect private key with\n");
	printf("-*:            algorithm to protect private key\n");
}

static int parse_args(struct cmd_genrsa_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",    no_argument,       NULL, 'h'},
		{"out",     required_argument, NULL, 'o'},
		{"f4",      no_argument,       NULL, 'E'},
		{"F4",      no_argument,       NULL, 'E'},
		{"3",       no_argument,       NULL, 'e'},
		{"passout", required_argument, NULL, 'p'},
		{NULL,      0,                 NULL,  0 },
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
			case 'o':
				if (!cmd_handle_out("genrsa", optarg,
				                    &data->fpout))
					return 0;
				break;
			case 'E':
				data->exp = 0x10001;
				break;
			case 'e':
				data->exp = 3;
				break;
			case 'p':
				if (!cmd_handle_pass("genrsa", optarg,
				                     &data->passout))
					return 0;
				break;
			default:
				data->cipher = evp_get_cipherbyname(argv[optind - 1] + 1);
				if (!data->cipher)
				{
					fprintf(stderr, "genrsa: unrecognized option: %s\n",
					        argv[optind - 1]);
					usage();
					return 0;
				}
				break;
		}
	}
	if (optind >= argc)
		return 1;
	if (argc - optind > 1)
	{
		fprintf(stderr, "genrsa: extra operand\n");
		return 0;
	}
	for (size_t j = 0; j < strlen(argv[optind]); ++j)
	{
		if (!isdigit(argv[optind][j]))
		{
			fprintf(stderr, "genrsa: key size isn't numeric\n");
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

int cmd_genrsa(int argc, char **argv)
{
	struct cmd_genrsa_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.exp = 0x10001;
	data.fpout = stdout;
	data.key_len = 512;
	data.rsa = rsa_new();
	if (!data.rsa)
	{
		fprintf(stderr, "genrsa: malloc: %s\n", strerror(errno));
		goto end;
	}
	data.gencb = bignum_gencb_new();
	if (!data.gencb)
	{
		fprintf(stderr, "genrsa: malloc: %s\n", strerror(errno));
		goto end;
	}
	bignum_gencb_set(data.gencb, gencb_cb, NULL);
	if (!parse_args(&data, argc, argv))
		goto end;
	if (data.key_len < 16)
	{
		fprintf(stderr, "genrsa: invalid key length, minimum is 16\n");
		goto end;
	}
	if (!rsa_generate_key(data.rsa, data.key_len, data.exp, data.gencb))
	{
		fprintf(stderr, "genrsa: failed to generate key\n");
		goto end;
	}
	fprintf(stderr, "e is ");
	bignum_print(rsa_get0_e(data.rsa), stderr);
	fprintf(stderr, " (0x");
	bignum_printhex(rsa_get0_e(data.rsa), stderr);
	fprintf(stderr, ")\n");
	if (!pem_write_rsa_private_key(data.fpout, data.rsa, data.cipher,
	                               data.passout ? NULL : cmd_ask_pass,
	                               data.passout))
	{
		fprintf(stderr, "genrsa: failed to write PEM key\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	free(data.passout);
	rsa_free(data.rsa);
	bignum_gencb_free(data.gencb);
	return ret;
}
