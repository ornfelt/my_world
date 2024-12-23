#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/asn1.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>
#include <jkssl/ec.h>

#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>

struct cmd_ecparam_data
{
	enum cmd_format inform;
	enum cmd_format outform;
	struct ec_group *ec_group;
	struct ec_key *ec_key;
	FILE *fpout;
	FILE *fpin;
	int noout;
	int text;
	int genkey;
	int param_enc;
	int conv_form;
};

static int do_read(struct cmd_ecparam_data *data)
{
	switch (data->inform)
	{
		case CMD_FORMAT_PEM:
			return pem_read_ecpk_parameters(data->fpin,
			                                &data->ec_group) != NULL;
		case CMD_FORMAT_DER:
			return d2i_ecpk_parameters_fp(data->fpin,
			                              &data->ec_group) != NULL;
		default:
			return 0;
	}
}

static int do_write_params(struct cmd_ecparam_data *data)
{
	switch (data->outform)
	{
		case CMD_FORMAT_PEM:
			return pem_write_ecpk_parameters(data->fpout,
			                                 data->ec_group);
		case CMD_FORMAT_DER:
			return i2d_ecpk_parameters_fp(data->fpout,
			                              data->ec_group);
		default:
			return 0;
	}
}

static int do_write_key(struct cmd_ecparam_data *data)
{
	switch (data->outform)
	{
		case CMD_FORMAT_PEM:
			return pem_write_ec_private_key(data->fpout,
			                                data->ec_key,
			                                NULL,
			                                NULL,
			                                0);
		case CMD_FORMAT_DER:
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

static void print_text(struct cmd_ecparam_data *data)
{
	printf("EC-Parameters: (%d bit)\n", ec_group_get_degree(data->ec_group));
	struct asn1_oid *oid = ec_group_get_curve_name(data->ec_group);
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
}

static int genkey(struct cmd_ecparam_data *data)
{
	data->ec_key = ec_key_new();
	if (!data->ec_key)
	{
		fprintf(stderr, "ecparam: ec key allocation failed\n");
		return 0;
	}
	if (!ec_key_set_group(data->ec_key, data->ec_group))
	{
		fprintf(stderr, "ecparam: failed to set ec key group\n");
		return 0;
	}
	if (!ec_key_generate_key(data->ec_key))
	{
		fprintf(stderr, "ecparam: failed to generate key\n");
		return 0;
	}
	return 1;
}

static int list_curve(const char *name, const uint32_t *oid, size_t oid_size,
                      void *data)
{
	printf("%s\n", name);
	(void)oid;
	(void)oid_size;
	(void)data;
	return 1;
}

static void list_curves(void)
{
	ec_foreach_curve(list_curve, NULL);
}

static void usage(void)
{
	printf("ecparam [options]\n");
	printf("-help:           display this help\n");
	printf("-name name:      name of the curve to use\n");
	printf("-inform format:  input format (PEM or DER)\n");
	printf("-outform format: output format (PEM or DER)\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
	printf("-text:           print text representation of key\n");
	printf("-noout:          don't output key\n");
	printf("-genkey:         generate an EC key\n");
	printf("-list_curves:    list the supported curves\n");
	printf("-param_enc:      set the parameters encoding (named_curve or explicit)\n");
	printf("-conv_form:      set the point conversion form (compressed, uncompressed or hybrid)\n");
}

static int parse_args(struct cmd_ecparam_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",        no_argument,       NULL, 'h'},
		{"name",        required_argument, NULL, 'N'},
		{"inform",      required_argument, NULL, 'I'},
		{"outform",     required_argument, NULL, 'O'},
		{"in",          required_argument, NULL, 'i'},
		{"out",         required_argument, NULL, 'o'},
		{"text",        no_argument,       NULL, 't'},
		{"noout",       no_argument,       NULL, 'n'},
		{"genkey",      no_argument,       NULL, 'g'},
		{"list_curves", no_argument,       NULL, 'l'},
		{"param_enc",   required_argument, NULL, 'p'},
		{"conv_form",   required_argument, NULL, 'c'},
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
				if (!cmd_handle_format("ecparam", optarg,
				                       &data->inform))
					return 0;
				break;
			case 'O':
				if (!cmd_handle_format("ecparam", optarg,
				                       &data->outform))
					return 0;
				break;
			case 'i':
				if (!cmd_handle_in("ecparam", optarg,
				                   &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("ecparam", optarg,
				                    &data->fpout))
					return 0;
				break;
			case 't':
				data->text = 1;
				break;
			case 'n':
				data->noout = 1;
				break;
			case 'N':
				data->ec_group = ec_get_curvebyname(optarg);
				if (!data->ec_group)
				{
					fprintf(stderr, "ecparam: unknown curve: %s\n",
					        optarg);
					return 0;
				}
				break;
			case 'g':
				data->genkey = 1;
				break;
			case 'l':
				list_curves();
				return 0;
			case 'p':
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
			case 'c':
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
			default:
				fprintf(stderr, "ecparam: unrecognized option: %s\n",
				        argv[optind - 1]);
				usage();
				return 0;
		}
	}
	return 1;
}

int cmd_ecparam(int argc, char **argv)
{
	struct cmd_ecparam_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.inform = CMD_FORMAT_PEM;
	data.outform = CMD_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	data.param_enc = -1;
	data.conv_form = -1;
	if (!parse_args(&data, argc, argv))
		goto end;
	if (!data.ec_group)
	{
		data.ec_group = ec_group_new();
		if (!data.ec_group)
		{
			fprintf(stderr, "ecparam: group allocation failed\n");
			goto end;
		}
		if (!do_read(&data))
		{
			fprintf(stderr, "ecparam: failed to read parameters\n");
			goto end;
		}
	}
	if (data.param_enc != -1)
		ec_group_set_asn1_flag(data.ec_group, data.param_enc);
	if (data.conv_form != -1)
		ec_group_set_point_conversion_form(data.ec_group, data.conv_form);
	if (data.text)
		print_text(&data);
	if (data.genkey && !genkey(&data))
		goto end;
	if (!data.noout && !do_write_params(&data))
	{
		fprintf(stderr, "ecparam: failed to write parameters\n");
		goto end;
	}
	if (data.genkey && !do_write_key(&data))
	{
		fprintf(stderr, "ecparam: failed to write key\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	ec_group_free(data.ec_group);
	ec_key_free(data.ec_key);
	return ret;
}
