#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/evp.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>

struct cmd_pkcs8_data
{
	struct evp_pkey *pkey;
	enum cmd_format inform;
	enum cmd_format outform;
	char *passin;
	char *passout;
	FILE *fpin;
	FILE *fpout;
	int topk8;
	int traditional;
	int iterations;
	int nocrypt;
	const struct evp_cipher *cipher;
};

static int do_read(struct cmd_pkcs8_data *data)
{
	switch (data->inform)
	{
		case CMD_FORMAT_PEM:
			if (data->topk8)
				return pem_read_private_key(data->fpin,
				                            &data->pkey,
				                            data->passin ? NULL : cmd_ask_pass,
				                            data->passin) != NULL;
			if (data->nocrypt)
				return pem_read_pkcs8_private_key_info(data->fpin,
				                                       &data->pkey) != NULL;
			return pem_read_pkcs8_private_key(data->fpin,
			                                  &data->pkey,
			                                  data->passin ? NULL : cmd_ask_pass,
			                                  data->passin) != NULL;
		case CMD_FORMAT_DER:
			if (data->topk8)
				return d2i_private_key_fp(data->fpin,
				                          &data->pkey) != NULL;
			if (data->nocrypt)
				return d2i_pkcs8_private_key_info_fp(data->fpin,
				                                     &data->pkey) != NULL;
			return d2i_pkcs8_private_key_fp(data->fpin,
			                                &data->pkey,
			                                data->passin ? NULL : cmd_ask_pass,
			                                data->passin) != NULL;
		default:
			return 0;
	}
}

static int do_write(struct cmd_pkcs8_data *data)
{
	switch (data->outform)
	{
		case CMD_FORMAT_PEM:
			if (data->traditional)
				return pem_write_private_key_traditional(data->fpout,
				                                         data->pkey,
				                                         NULL,
				                                         NULL,
				                                         NULL);
			if (data->nocrypt)
				return pem_write_pkcs8_private_key_info(data->fpout,
				                                        data->pkey);
			return pem_write_pkcs8_private_key(data->fpout,
			                                   data->pkey,
			                                   data->cipher,
			                                   data->passout ? NULL : cmd_ask_pass,
			                                   data->passout);
		case CMD_FORMAT_DER:
			if (data->traditional)
				return i2d_private_key_traditional_fp(data->fpout,
				                                      data->pkey);
			if (data->nocrypt)
				return i2d_pkcs8_private_key_info_fp(data->fpout,
				                                     data->pkey);
			return i2d_pkcs8_private_key_fp(data->fpout,
			                                data->pkey,
			                                data->cipher,
			                                data->passout ? NULL : cmd_ask_pass,
			                                data->passout);
		default:
			return 0;
	}
}

static void usage(void)
{
	printf("pkcs8 [options]\n");
	printf("-help:           displat this help\n");
	printf("-inform format:  input format (PEM or DER)\n");
	printf("-outform format: output format (PEM or DER)\n");
	printf("-passin pass:    input password\n");
	printf("-passout pass:   output password\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
	printf("-topk8:          read non-pkcs8 private key as input\n");
	printf("-traditional:    write non-pkcs8 private key to output\n");
	printf("-v1 val:         use PKCS#5 V1.5 cipher\n");
	printf("-v2 val:         use PKCS#5 v2.0 cipher\n");
	printf("-iter iter:      use the given number of iterations for key derivation\n");
	printf("-nocrypt:        read or write unencrypted key\n");
}

static int parse_args(struct cmd_pkcs8_data *data, int argc, char **argv)
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
		{"topk8",       no_argument,       NULL, 't'},
		{"traditional", no_argument,       NULL, 'f'},
		{"v1",          required_argument, NULL, '1'},
		{"v2",          required_argument, NULL, '2'},
		{"iter",        required_argument, NULL, 'n'},
		{"nocrypt",     no_argument,       NULL, 'c'},
	};
	int c;
	while ((c = getopt_long_only(argc, argv, "", opts, NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				usage();
				return 0;
			case 'I':
				if (!cmd_handle_format("pkcs8", optarg,
				                       &data->inform))
					return 0;
				break;
			case 'O':
				if (!cmd_handle_format("pkcs8", optarg,
				                       &data->outform))
					return 0;
				break;
			case 'p':
				if (!cmd_handle_pass("pkcs8", optarg,
				                     &data->passin))
					return 0;
				break;
			case 'P':
				if (!cmd_handle_pass("pkcs8", optarg,
				                     &data->passout))
					return 0;
				break;
			case 'i':
				if (!cmd_handle_in("pkcs8", optarg,
				                   &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("pkcs8", optarg,
				                    &data->fpout))
					return 0;
				break;
			case 't':
				data->topk8 = 1;
				break;
			case 'f':
				data->traditional = 1;
				break;
			case '1':
				break;
			case '2':
				break;
			case 'c':
				data->nocrypt = 1;
				break;
			case 'n':
			{
				errno = 0;
				long iterations = strtol(optarg, NULL, 10);
				if (errno || iterations < 0 || iterations >= INT_MAX)
				{
					fprintf(stderr, "pkcs8: invalid iterations count: %s\n",
					        optarg);
					usage();
					return 0;
				}
				data->iterations = iterations;
				break;
			}
			default:
				usage();
				return 0;
		}
	}
	return 1;
}

int cmd_pkcs8(int argc, char **argv)
{
	struct cmd_pkcs8_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.inform = CMD_FORMAT_PEM;
	data.outform = CMD_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	data.iterations = 2048;
	data.cipher = evp_aes_256_cbc();
	data.pkey = evp_pkey_new();
	if (!data.pkey)
	{
		fprintf(stderr, "pkcs8: pkey allocation failed\n");
		goto end;
	}
	if (!parse_args(&data, argc, argv))
		goto end;
	if (!do_read(&data))
	{
		fprintf(stderr, "pkcs8: failed to read input pkcs8\n");
		goto end;
	}
	if (!do_write(&data))
	{
		fprintf(stderr, "pkcs8: failed to write output pkcs8\n");
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
	evp_pkey_free(data.pkey);
	return ret;
}
