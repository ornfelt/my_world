#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/bignum.h>
#include <jkssl/evp.h>
#include <jkssl/rsa.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

enum rsa_type
{
	RSA_TYPE_PRIVATE_KEY,
	RSA_TYPE_PUBLIC_KEY,
	RSA_TYPE_PUBKEY,
};

struct cmd_rsa_data
{
	struct rsa *rsa;
	enum cmd_format inform;
	enum cmd_format outform;
	char *passin;
	char *passout;
	FILE *fpout;
	FILE *fpin;
	enum rsa_type intype;
	enum rsa_type outtype;
	int text;
	int modulus;
	int noout;
	int check;
	const struct evp_cipher *cipher;
};

static int do_read(struct cmd_rsa_data *data)
{
	switch (data->inform)
	{
		case CMD_FORMAT_PEM:
			switch (data->intype)
			{
				case RSA_TYPE_PRIVATE_KEY:
					return pem_read_rsa_private_key(data->fpin,
					                                &data->rsa,
					                                data->passin ? NULL : cmd_ask_pass,
					                                data->passin) != NULL;
				case RSA_TYPE_PUBLIC_KEY:
					return pem_read_rsa_public_key(data->fpin,
					                               &data->rsa,
					                               data->passin ? NULL : cmd_ask_pass,
					                               data->passin) != NULL;
				case RSA_TYPE_PUBKEY:
					return pem_read_rsa_pubkey(data->fpin,
					                           &data->rsa,
					                           data->passin ? NULL : cmd_ask_pass,
					                           data->passin) != NULL;
				default:
					return 0;
			}
		case CMD_FORMAT_DER:
			switch (data->intype)
			{
				case RSA_TYPE_PRIVATE_KEY:
					return d2i_rsa_private_key_fp(data->fpin,
					                              &data->rsa) != NULL;
				case RSA_TYPE_PUBLIC_KEY:
					return d2i_rsa_public_key_fp(data->fpin,
					                             &data->rsa) != NULL;
				case RSA_TYPE_PUBKEY:
					return d2i_rsa_pubkey_fp(data->fpin,
					                         &data->rsa) != NULL;
				default:
					return 0;
			}
		default:
			return 0;
	}
}

static int do_write(struct cmd_rsa_data *data)
{
	switch (data->outform)
	{
		case CMD_FORMAT_PEM:
			switch (data->outtype)
			{
				case RSA_TYPE_PRIVATE_KEY:
					return pem_write_rsa_private_key(data->fpout,
					                                 data->rsa,
					                                 data->cipher,
					                                 data->passout ? NULL : cmd_ask_pass,
					                                 data->passout);
				case RSA_TYPE_PUBLIC_KEY:
					return pem_write_rsa_public_key(data->fpout,
					                                data->rsa);
				case RSA_TYPE_PUBKEY:
					return pem_write_rsa_pubkey(data->fpout,
					                            data->rsa);
				default:
					return 0;
			}
		case CMD_FORMAT_DER:
			switch (data->outtype)
			{
				case RSA_TYPE_PRIVATE_KEY:
					return i2d_rsa_private_key_fp(data->fpout,
					                              data->rsa);
				case RSA_TYPE_PUBLIC_KEY:
					return i2d_rsa_public_key_fp(data->fpout,
					                             data->rsa);
				case RSA_TYPE_PUBKEY:
					return i2d_rsa_pubkey_fp(data->fpout,
					                         data->rsa);
				default:
					return 0;
			}
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

	printf("%s (%d bits):\n", name, bignum_num_bits(bignum));
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
		fprintf(stderr, "rsa: allocation failed\n");
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

static int print_text(struct cmd_rsa_data *data)
{
	if (data->intype != RSA_TYPE_PRIVATE_KEY)
	{
		if (!do_print_bignum(rsa_get0_n(data->rsa), "Modulus"))
			return 0;
		printf("Exponent: ");
		bignum_print(rsa_get0_e(data->rsa), stdout);
		printf(" (0x");
		bignum_printhex(rsa_get0_e(data->rsa), stdout);
		printf(")\n");
	}
	else
	{
		if (!do_print_bignum(rsa_get0_n(data->rsa), "Modulus"))
			return 0;
		printf("Public exponent: ");
		bignum_print(rsa_get0_e(data->rsa), stdout);
		printf(" (0x");
		bignum_printhex(rsa_get0_e(data->rsa), stdout);
		printf(")\n");
		if (!do_print_bignum(rsa_get0_d(data->rsa), "Private exponent")
		 || !do_print_bignum(rsa_get0_p(data->rsa), "Prime 1")
		 || !do_print_bignum(rsa_get0_q(data->rsa), "Prime 2")
		 || !do_print_bignum(rsa_get0_dmp1(data->rsa), "Exponent 1")
		 || !do_print_bignum(rsa_get0_dmq1(data->rsa), "Exponent 2")
		 || !do_print_bignum(rsa_get0_iqmp(data->rsa), "Coefficient"))
			return 0;
	}
	return 1;
}

static void usage(void)
{
	printf("rsa [options]\n");
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
	printf("-check:          check the validity of given key\n");
	printf("-*:              algorithm to protect private key\n");
}

static int parse_args(struct cmd_rsa_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",             no_argument,       NULL, 'h'},
		{"inform",           required_argument, NULL, 'I'},
		{"outform",          required_argument, NULL, 'O'},
		{"passin",           required_argument, NULL, 'p'},
		{"passout",          required_argument, NULL, 'P'},
		{"in",               required_argument, NULL, 'i'},
		{"out",              required_argument, NULL, 'o'},
		{"pubin",            no_argument,       NULL, 'x'},
		{"pubout",           no_argument,       NULL, 'X'},
		{"text",             no_argument,       NULL, 't'},
		{"modulus",          no_argument,       NULL, 'm'},
		{"noout",            no_argument,       NULL, 'n'},
		{"check",            no_argument,       NULL, 'c'},
		{"RSAPublicKey_in",  no_argument,       NULL, 'r'},
		{"RSAPublicKey_out", no_argument,       NULL, 'R'},
		{NULL,               0,                 NULL,  0 },
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
				if (!cmd_handle_format("rsa", optarg,
				                       &data->inform))
					return 0;
				break;
			case 'O':
				if (!cmd_handle_format("rsa", optarg,
				                       &data->outform))
					return 0;
				break;
			case 'p':
				if (!cmd_handle_pass("rsa", optarg,
				                     &data->passin))
					return 0;
				break;
			case 'P':
				if (!cmd_handle_pass("rsa", optarg,
				                     &data->passout))
					return 0;
				break;
			case 'i':
				if (!cmd_handle_in("rsa", optarg, &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("rsa", optarg, &data->fpout))
					return 0;
				break;
			case 'x':
				data->intype = RSA_TYPE_PUBKEY;
				if (data->outtype == RSA_TYPE_PRIVATE_KEY)
					data->outtype = RSA_TYPE_PUBKEY;
				break;
			case 'X':
				data->outtype = RSA_TYPE_PUBKEY;
				break;
			case 'r':
				data->intype = RSA_TYPE_PUBLIC_KEY;
				if (data->outtype == RSA_TYPE_PRIVATE_KEY)
					data->outtype = RSA_TYPE_PUBKEY;
				break;
			case 'R':
				data->outtype = RSA_TYPE_PUBLIC_KEY;
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
			case 'c':
				data->check = 1;
				break;
			default:
				data->cipher = evp_get_cipherbyname(argv[optind - 1] + 1);
				if (!data->cipher)
				{
					fprintf(stderr, "rsa: unrecognized option: %s\n",
					        argv[optind - 1]);
					usage();
					return 0;
				}
				break;
		}
	}
	return 1;
}

int cmd_rsa(int argc, char **argv)
{
	struct cmd_rsa_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.inform = CMD_FORMAT_PEM;
	data.outform = CMD_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	data.intype = RSA_TYPE_PRIVATE_KEY;
	data.outtype = RSA_TYPE_PRIVATE_KEY;
	data.rsa = rsa_new();
	if (!data.rsa)
	{
		fprintf(stderr, "rsa: malloc: %s\n", strerror(errno));
		goto end;
	}
	if (!parse_args(&data, argc, argv))
		goto end;
	if (!do_read(&data))
	{
		fprintf(stderr, "rsa: invalid key\n");
		goto end;
	}
	if (data.check)
	{
		switch (rsa_check_key(data.rsa))
		{
			case -1:
				fprintf(stderr, "rsa: failed to check key\n");
				goto end;
			case 0:
				printf("invalid key\n");
				break;
			case 1:
				printf("valid key\n");
				break;
		}
	}
	if (data.text && !print_text(&data))
		goto end;
	if (data.modulus)
	{
		printf("Modulus=");
		bignum_printhex(rsa_get0_n(data.rsa), stdout);
		printf("\n");
	}
	if (!data.noout && !do_write(&data))
	{
		fprintf(stderr, "rsa: failed to write key\n");
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
	rsa_free(data.rsa);
	return ret;
}
