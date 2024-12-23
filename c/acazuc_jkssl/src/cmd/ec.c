#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/bignum.h>
#include <jkssl/asn1.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>
#include <jkssl/evp.h>
#include <jkssl/ec.h>

#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>

struct cmd_ec_data
{
	struct ec_key *ec_key;
	enum cmd_format inform;
	enum cmd_format outform;
	char *passin;
	char *passout;
	FILE *fpout;
	FILE *fpin;
	int pubin;
	int pubout;
	int text;
	int noout;
	int check;
	int param_enc;
	int conv_form;
	int param_out;
	const struct evp_cipher *cipher;
};

static int do_read(struct cmd_ec_data *data)
{
	switch (data->inform)
	{
		case CMD_FORMAT_PEM:
			if (data->pubin)
				return pem_read_ec_pubkey(data->fpin,
				                          &data->ec_key,
				                          data->passin ? NULL : cmd_ask_pass,
				                          data->passin) != NULL;
			return pem_read_ec_private_key(data->fpin,
			                               &data->ec_key,
			                               data->passin ? NULL : cmd_ask_pass,
			                               data->passin) != NULL;
		case CMD_FORMAT_DER:
			if (data->pubin)
				return d2i_ec_pubkey_fp(data->fpin,
				                        &data->ec_key) != NULL;
			return d2i_ec_private_key_fp(data->fpin,
			                             &data->ec_key) != NULL;
		default:
			return 0;
	}
}

static int do_write(struct cmd_ec_data *data)
{
	switch (data->outform)
	{
		case CMD_FORMAT_PEM:
			if (data->param_out)
				return pem_write_ecpk_parameters(data->fpout,
				                                 ec_key_get0_group(data->ec_key));
			if (data->pubout)
				return pem_write_ec_pubkey(data->fpout,
				                           data->ec_key);
			return pem_write_ec_private_key(data->fpout,
			                                data->ec_key,
			                                data->cipher,
			                                data->passout ? NULL : cmd_ask_pass,
			                                data->passout);
		case CMD_FORMAT_DER:
			if (data->param_out)
				return i2d_ecpk_parameters_fp(data->fpout,
				                              ec_key_get0_group(data->ec_key));
			if (data->pubout)
				return i2d_ec_pubkey_fp(data->fpout,
				                        data->ec_key);
			return i2d_ec_private_key_fp(data->fpout,
			                             data->ec_key);
		default:
			return 0;
	}
}

static int print_oid(const char *name, const uint32_t *oid_data,
                     size_t oid_size, void *data)
{
	struct asn1_oid **oid = data;
	if (asn1_oid_cmp_uint32(*oid, oid_data, oid_size))
		return 1;
	printf("%s", name);
	*oid = NULL;
	return 0;
}

static void print_hex(uint8_t val)
{
	if (val >= 10)
		printf("%c", 'a' + val - 10);
	else
		printf("%c", '0' + val);
}

static int do_print_bignum_data(const struct bignum *bignum, int *n)
{
	uint8_t *data;
	int i;

	i = bignum_num_bytes(bignum);
	if (!i)
	{
		printf("00");
		return 1;
	}
	data = malloc(i);
	if (!data)
	{
		fprintf(stderr, "ec: allocation failed\n");
		return 0;
	}
	bignum_bignum2bin(bignum, data);
	while (i)
	{
		if (!(*n % 16))
			printf("    ");
		--i;
		print_hex(data[i] >> 4);
		print_hex(data[i] & 0xF);
		if (*n % 16 == 15)
			printf("\n");
		else
			printf(":");
		++(*n);
	}
	free(data);
	return 1;
}

static int do_print_bignum(const struct bignum *bignum)
{
	int n = 0;
	if (!do_print_bignum_data(bignum, &n))
		return 0;
	if (n % 16)
		printf("\n");
	return 1;
}

static int do_print_point(const struct ec_group *group,
                          const struct ec_point *point)
{
	struct bignum_ctx *bn_ctx = bignum_ctx_new();
	struct bignum *x = bignum_new();
	struct bignum *y = bignum_new();
	int n;

	if (!x
	 || !y
	 || !bn_ctx
	 || !ec_point_get_affine_coordinates(group, point, x, y, bn_ctx))
	{
		bignum_free(x);
		bignum_free(y);
		bignum_ctx_free(bn_ctx);
		fprintf(stderr, "ec: allocation failed\n");
		return 0;
	}
	n = 1;
	printf("    04:");
	if (!do_print_bignum_data(x, &n)
	 || !do_print_bignum_data(y, &n))
		return 0;
	if (n % 16)
		printf("\n");
	bignum_free(x);
	bignum_free(y);
	bignum_ctx_free(bn_ctx);
	return 1;
}

static int print_text(struct cmd_ec_data *data)
{
	const struct ec_group *group = ec_key_get0_group(data->ec_key);
	const struct bignum *priv = ec_key_get0_private_key(data->ec_key);
	const struct ec_point *pub = ec_key_get0_public_key(data->ec_key);
	if (priv)
		printf("Private-Key");
	else
		printf("Public-Key");
	printf(": (%d bit)\n", ec_group_get_degree(group));
	if (priv)
	{
		printf("priv:\n");
		if (!do_print_bignum(priv))
			return 0;
	}
	if (pub)
	{
		printf("pub:\n");
		if (!do_print_point(group, pub))
			return 0;
	}
	struct asn1_oid *oid = ec_group_get_curve_name(group);
	if (oid)
	{
		printf("ASN1 OID: ");
		struct asn1_oid *tmp = oid;
		ec_foreach_curve(print_oid, &tmp);
		asn1_oid_free(oid);
		if (tmp)
			printf("unknown");
		printf("\n");
	}
	return 1;
}

static int do_check(struct cmd_ec_data *data)
{
	if (ec_key_check_key(data->ec_key))
	{
		printf("Key OK\n");
		return 1;
	}
	else
	{
		printf("Key not OK\n");
		return 0;
	}
}

static void usage(void)
{
	printf("ec [options]\n");
	printf("-help: display this help\n");
	printf("-inform format:  input format (PEM or DER)\n");
	printf("-outform format: output format (PEM or DER)\n");
	printf("-passin pass:    input password\n");
	printf("-passout pass:   output password\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
	printf("-pubin:          input file in pubkey\n");
	printf("-pubout:         output file as pubkey\n");
	printf("-text:           print text representation of key\n");
	printf("-noout:          don't output key\n");
	printf("-check:          verify key integrity\n");
	printf("-param_enc:      set the parameters encoding (named_curve or explicit)\n");
	printf("-conv_form:      set the point conversion form (compressed, uncompressed or hybrid)\n");
	printf("-param_out:      output the curve parameters\n");
	printf("-*:              algorithm to protect private key\n");
}

static int parse_args(struct cmd_ec_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",        no_argument,       NULL, 'h'},
		{"name",        required_argument, NULL, 'N'},
		{"inform",      required_argument, NULL, 'I'},
		{"outform",     required_argument, NULL, 'O'},
		{"passin",      required_argument, NULL, 'p'},
		{"passout",     required_argument, NULL, 'P'},
		{"in",          required_argument, NULL, 'i'},
		{"out",         required_argument, NULL, 'o'},
		{"pubin",       no_argument,       NULL, 'x'},
		{"pubout",      no_argument,       NULL, 'X'},
		{"text",        no_argument,       NULL, 't'},
		{"noout",       no_argument,       NULL, 'n'},
		{"check",       no_argument,       NULL, 'c'},
		{"param_enc",   required_argument, NULL, 'e'},
		{"conv_form",   required_argument, NULL, 'f'},
		{"param_out",   no_argument,       NULL, 'C'},
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
				if (!cmd_handle_format("ec", optarg,
				                       &data->inform))
					return 0;
				break;
			case 'O':
				if (!cmd_handle_format("ec", optarg,
				                       &data->outform))
					return 0;
				break;
			case 'p':
				if (!cmd_handle_pass("ec", optarg,
				                     &data->passin))
					return 0;
				break;
			case 'P':
				if (!cmd_handle_pass("ec", optarg,
				                     &data->passout))
					return 0;
				break;
			case 'i':
				if (!cmd_handle_in("ec", optarg,
				                   &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("ec", optarg,
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
			case 'n':
				data->noout = 1;
				break;
			case 'c':
				data->check = 1;
				break;
			case 'e':
				if (!strcmp(optarg, "named_curve"))
				{
					data->param_enc = JKSSL_EC_NAMED_CURVE;
				}
				else if (!strcmp(optarg, "explicit"))
				{
					data->param_enc = JKSSL_EC_EXPLICIT_CURVE;
				}
				else
				{
					usage();
					return 0;
				}
				break;
			case 'f':
				if (!strcmp(optarg, "uncompressed"))
				{
					data->conv_form = POINT_CONVERSION_UNCOMPRESSED;
				}
				else if (!strcmp(optarg, "compressed"))
				{
					data->param_enc = POINT_CONVERSION_COMPRESSED;
				}
				else if (!strcmp(optarg, "hybrid"))
				{
					data->param_enc = POINT_CONVERSION_HYBRID;
				}
				else
				{
					usage();
					return 0;
				}
				break;
			case 'C':
				data->param_out = 1;
				break;
			default:
				data->cipher = evp_get_cipherbyname(argv[optind - 1] + 1);
				if (!data->cipher)
				{
					fprintf(stderr, "ec: unrecognized option: %s\n",
					        argv[optind - 1]);
					usage();
					return 0;
				}
				break;
		}
	}
	return 1;
}

int cmd_ec(int argc, char **argv)
{
	struct cmd_ec_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.inform = CMD_FORMAT_PEM;
	data.outform = CMD_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	data.param_enc = -1;
	data.conv_form = -1;
	data.ec_key = ec_key_new();
	if (!data.ec_key)
	{
		fprintf(stderr, "ec: key allocation failed\n");
		goto end;
	}
	if (!parse_args(&data, argc, argv))
		goto end;
	if (data.param_enc != -1)
		ec_key_set_asn1_flag(data.ec_key, data.param_enc);
	if (data.conv_form != -1)
		ec_key_set_conv_form(data.ec_key, data.conv_form);
	if (!do_read(&data))
	{
		fprintf(stderr, "ec: invalid key\n");
		goto end;
	}
	if (data.check && !do_check(&data))
		goto end;
	if (data.text && !print_text(&data))
		goto end;
	if (!data.noout && !do_write(&data))
	{
		fprintf(stderr, "ec: failed to write key\n");
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
	ec_key_free(data.ec_key);
	return ret;
}
