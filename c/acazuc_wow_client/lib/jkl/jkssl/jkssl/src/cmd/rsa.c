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

enum rsa_format
{
	RSA_FORMAT_PEM,
	RSA_FORMAT_DER,
};

enum rsa_type
{
	RSA_TYPE_PRIVATE_KEY,
	RSA_TYPE_PUBLIC_KEY,
	RSA_TYPE_PUBKEY,
};

struct cmd_rsa_data
{
	struct rsa_ctx *rsa_ctx;
	enum rsa_format inform;
	enum rsa_format outform;
	const char *passin;
	const char *passout;
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
		case RSA_FORMAT_PEM:
			switch (data->intype)
			{
				case RSA_TYPE_PRIVATE_KEY:
					return pem_read_rsa_private_key(data->fpin,
					                                &data->rsa_ctx,
					                                data->passin) != NULL;
				case RSA_TYPE_PUBLIC_KEY:
					return pem_read_rsa_public_key(data->fpin,
					                               &data->rsa_ctx,
					                               data->passin) != NULL;
				case RSA_TYPE_PUBKEY:
					return pem_read_rsa_pubkey(data->fpin,
					                           &data->rsa_ctx,
					                           data->passin) != NULL;
				default:
					return 0;
			}
		case RSA_FORMAT_DER:
			switch (data->intype)
			{
				case RSA_TYPE_PRIVATE_KEY:
					return d2i_rsa_private_key_fp(data->fpin,
					                              &data->rsa_ctx) != NULL;
				case RSA_TYPE_PUBLIC_KEY:
					return d2i_rsa_public_key_fp(data->fpin,
					                             &data->rsa_ctx) != NULL;
				case RSA_TYPE_PUBKEY:
					return d2i_rsa_pubkey_fp(data->fpin,
					                         &data->rsa_ctx) != NULL;
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
		case RSA_FORMAT_PEM:
			switch (data->outtype)
			{
				case RSA_TYPE_PRIVATE_KEY:
					return pem_write_rsa_private_key(data->fpout,
					                                 data->rsa_ctx,
					                                 data->cipher,
					                                 data->passout);
				case RSA_TYPE_PUBLIC_KEY:
					return pem_write_rsa_public_key(data->fpout,
					                                data->rsa_ctx);
				case RSA_TYPE_PUBKEY:
					return pem_write_rsa_pubkey(data->fpout,
					                            data->rsa_ctx);
				default:
					return 0;
			}
		case RSA_FORMAT_DER:
			switch (data->outtype)
			{
				case RSA_TYPE_PRIVATE_KEY:
					return i2d_rsa_private_key_fp(data->fpout,
					                              data->rsa_ctx);
				case RSA_TYPE_PUBLIC_KEY:
					return i2d_rsa_public_key_fp(data->fpout,
					                             data->rsa_ctx);
				case RSA_TYPE_PUBKEY:
					return i2d_rsa_pubkey_fp(data->fpout,
					                         data->rsa_ctx);
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

static void do_print_bignum(struct bignum *bignum, char *name)
{
	int n;
	int i;

	printf("%s (%d bits):\n", name, bignum_num_bits(bignum));
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

static void print_text(struct cmd_rsa_data *data)
{
	if (data->intype != RSA_TYPE_PRIVATE_KEY)
	{
		do_print_bignum(data->rsa_ctx->n, "Modulus");
		printf("Exponent: ");
		bignum_print(data->rsa_ctx->e, stdout);
		printf(" (0x");
		bignum_printhex(data->rsa_ctx->e, stdout);
		printf(")\n");
	}
	else
	{
		do_print_bignum(data->rsa_ctx->n, "Modulus");
		printf("Public exponent: ");
		bignum_print(data->rsa_ctx->e, stdout);
		printf(" (0x");
		bignum_printhex(data->rsa_ctx->e, stdout);
		printf(")\n");
		do_print_bignum(data->rsa_ctx->d, "Private exponent");
		do_print_bignum(data->rsa_ctx->p, "Prime 1");
		do_print_bignum(data->rsa_ctx->q, "Prime 2");
		do_print_bignum(data->rsa_ctx->dmp, "Exponent 1");
		do_print_bignum(data->rsa_ctx->dmq, "Exponent 2");
		do_print_bignum(data->rsa_ctx->coef, "Coefficient");
	}
}

static int check_dmp(struct cmd_rsa_data *data, struct bignum_ctx *bn_ctx)
{
	struct bignum one;
	struct bignum tmp;
	int ret = 0;

	bignum_init(&tmp);
	bignum_init(&one);
	if (!bignum_one(&one)
	 || !bignum_sub(&tmp, data->rsa_ctx->p, &one, bn_ctx)
	 || !bignum_mod(&tmp, data->rsa_ctx->d, &tmp, bn_ctx))
	{
		fprintf(stderr, "rsa: bignum internal error\n");
		goto end;
	}
	if (bignum_cmp(&tmp, data->rsa_ctx->dmp))
	{
		fprintf(stderr, "rsa: dmp is not equal to d %% (p - 1)\n");
		goto end;
	}
	ret = 1;
end:
	bignum_clear(&one);
	bignum_clear(&tmp);
	return ret;
}

static int check_dmq(struct cmd_rsa_data *data, struct bignum_ctx *bn_ctx)
{
	struct bignum one;
	struct bignum tmp;
	int ret = 0;

	bignum_init(&tmp);
	bignum_init(&one);
	if (!bignum_one(&one)
	 || !bignum_sub(&tmp, data->rsa_ctx->q, &one, bn_ctx)
	 || !bignum_mod(&tmp, data->rsa_ctx->d, &tmp, bn_ctx))
	{
		fprintf(stderr, "rsa: bignum internal error\n");
		goto end;
	}
	if (bignum_cmp(&tmp, data->rsa_ctx->dmq))
	{
		fprintf(stderr, "rsa: dmq is not equal to d %% (q - 1)\n");
		goto end;
	}
	ret = 1;
end:
	bignum_clear(&one);
	bignum_clear(&tmp);
	return ret;
}

static int do_check(struct cmd_rsa_data *data)
{
	struct bignum_ctx *bn_ctx;
	struct bignum tmp;
	int ret = 0;

	bignum_init(&tmp);
	bn_ctx = bignum_ctx_new();
	if (!bn_ctx)
	{
		fprintf(stderr, "rsa: malloc: %s\n", strerror(errno));
		goto end;
	}
	if (bignum_is_one(data->rsa_ctx->e))
	{
		fprintf(stderr, "rsa: error: public exponent is one\n");
		goto end;
	}
	if (!bignum_is_bit_set(data->rsa_ctx->e, 0))
	{
		fprintf(stderr, "rsa: error: public exponent is even\n");
		goto end;
	}
	if (data->rsa_ctx->p
	 && !bignum_check_prime(data->rsa_ctx->p, bn_ctx, NULL))
	{
		fprintf(stderr, "rsa: error: p is not prime\n");
		goto end;
	}
	if (data->rsa_ctx->q
	 && !bignum_check_prime(data->rsa_ctx->q, bn_ctx, NULL))
	{
		fprintf(stderr, "rsa: error: q is not prime\n");
		goto end;
	}
	if (!data->rsa_ctx->p
	 || !data->rsa_ctx->q)
	{
		ret = 1;
		goto end;
	}
	if (!bignum_mul(&tmp, data->rsa_ctx->p, data->rsa_ctx->q, bn_ctx))
	{
		fprintf(stderr, "rsa: bignum_mul failed\n");
		goto end;
	}
	if (bignum_cmp(&tmp, data->rsa_ctx->n))
	{
		fprintf(stderr, "rsa: n is not equal to p * q\n");
		goto end;
	}
	if (data->rsa_ctx->dmq && !check_dmq(data, bn_ctx))
		goto end;
	if (data->rsa_ctx->dmp && !check_dmp(data, bn_ctx))
		goto end;
	if (!bignum_mod_inverse(&tmp, data->rsa_ctx->q, data->rsa_ctx->p, bn_ctx))
	{
		fprintf(stderr, "rsa: bignum_mod_inverse failed\n");
		goto end;
	}
	if (bignum_cmp(&tmp, data->rsa_ctx->coef))
	{
		fprintf(stderr, "rsa: coef is not equal to mod_inverse(q, p)\n");
		goto end;
	}

end:
	bignum_ctx_free(bn_ctx);
	return ret;
}

static int handle_inform_outform(const char *arg, enum rsa_format *format)
{
	if (!strcmp(arg, "PEM"))
	{
		*format = RSA_FORMAT_PEM;
		return 1;
	}
	if (!strcmp(arg, "DER"))
	{
		*format = RSA_FORMAT_DER;
		return 1;
	}
	fprintf(stderr, "rsa: unsupported format: %s\n", arg);
	return 0;
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
					fprintf(stderr, "rsa: open(%s): %s\n",
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
					fprintf(stderr, "rsa: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
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
	data.inform = RSA_FORMAT_PEM;
	data.outform = RSA_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	data.intype = RSA_TYPE_PRIVATE_KEY;
	data.outtype = RSA_TYPE_PRIVATE_KEY;
	data.rsa_ctx = rsa_new();
	if (!data.rsa_ctx)
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
		do_check(&data);
	if (data.text)
		print_text(&data);
	if (data.modulus)
	{
		printf("Modulus=");
		bignum_printhex(data.rsa_ctx->n, stdout);
		printf("\n");
	}
	if (!data.noout && !do_write(&data))
	{
		fprintf(stderr, "rsa: failed to write PEM key\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	rsa_free(data.rsa_ctx);
	return ret;
}
