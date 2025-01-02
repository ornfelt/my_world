#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/evp.h>
#include <jkssl/dsa.h>
#include <jkssl/pem.h>

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct cmd_gendsa_data
{
	struct dsa *dsa;
	const struct evp_cipher *cipher;
	char *passout;
	FILE *fpout;
	FILE *fpin;
};

static void usage(void)
{
	printf("gendsa [options] [paramfile]\n");
	printf("-help:         display this help\n");
	printf("-out file:     output file\n");
	printf("-passout pass: output file protection password\n");
	printf("-*:            algorithm to protect private key\n");
}

static int parse_args(struct cmd_gendsa_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",    no_argument,       NULL, 'h'},
		{"out",     required_argument, NULL, 'o'},
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
				if (!cmd_handle_out("gendsa", optarg,
				                    &data->fpout))
					return 0;
				break;
			case 'p':
				if (!cmd_handle_pass("gendsa", optarg,
				                     &data->passout))
					return 0;
				break;
			default:
				data->cipher = evp_get_cipherbyname(argv[optind - 1] + 1);
				if (!data->cipher)
				{
					fprintf(stderr, "gendsa: unrecognized option: %s\n",
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
		fprintf(stderr, "gendsa: extra operand\n");
		return 0;
	}
	if (!cmd_handle_in("gendsa", argv[optind], &data->fpin))
		return 0;
	return 1;
}

int cmd_gendsa(int argc, char **argv)
{
	struct cmd_gendsa_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.fpout = stdout;
	data.fpin = stdin;
	data.dsa = dsa_new();
	if (!data.dsa)
	{
		fprintf(stderr, "gendsa: malloc: %s\n", strerror(errno));
		goto end;
	}
	if (!parse_args(&data, argc, argv))
		goto end;
	if (!pem_read_dsa_params(data.fpin, &data.dsa, NULL, 0))
	{
		fprintf(stderr, "gendsa: failed to read parameters\n");
		goto end;
	}
	if (!dsa_generate_key(data.dsa))
	{
		fprintf(stderr, "gendsa: failed to generate private key\n");
		goto end;
	}
	if (!pem_write_dsa_private_key(data.fpout, data.dsa, data.cipher,
	                               data.passout ? NULL : cmd_ask_pass,
	                               data.passout))
	{
		fprintf(stderr, "gendsa: failed to write PEM key\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	free(data.passout);
	dsa_free(data.dsa);
	return ret;
}
