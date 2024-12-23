#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/bignum.h>
#include <jkssl/asn1.h>
#include <jkssl/oid.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <inttypes.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

struct cmd_asn1parse_data
{
	enum cmd_format inform;
	enum cmd_format outform;
	FILE *fpout;
	FILE *fpin;
	struct asn1_object *object;
	int noout;
	int text;
};

static int do_print(struct asn1_object *object, size_t indent);

static int do_read(struct cmd_asn1parse_data *data)
{
	switch (data->inform)
	{
		case CMD_FORMAT_PEM:
			return pem_read_asn1_object(data->fpin,
			                            &data->object) != NULL;
		case CMD_FORMAT_DER:
			return d2i_asn1_object_fp(data->fpin,
			                          &data->object) != NULL;
		default:
			return 0;
	}
}

static int do_write(struct cmd_asn1parse_data *data)
{
	switch (data->outform)
	{
		case CMD_FORMAT_PEM:
			return pem_write_asn1_object(data->fpout,
			                             data->object);
		case CMD_FORMAT_DER:
			return i2d_asn1_object_fp(data->fpout, data->object);
		default:
			return 0;
	}
}

static int print_universal(struct asn1_object *object, size_t indent)
{
	static const char *names[] =
	{
#define NAME(name) \
		[ASN1_##name] = #name
		NAME(EOC),
		NAME(BOOLEAN),
		NAME(INTEGER),
		NAME(BIT_STRING),
		NAME(OCTET_STRING),
		NAME(NULL),
		NAME(OBJECT_IDENTIFIER),
		NAME(OBJECT_DESCRIPTOR),
		NAME(EXTERNAL),
		NAME(REAL),
		NAME(ENUMERATED),
		NAME(EMBEDDED_PDV),
		NAME(UTF8_STRING),
		NAME(RELATIVE_OID),
		NAME(TIME),
		NAME(SEQUENCE),
		NAME(SET),
		NAME(NUMERIC_STRING),
		NAME(PRINTABLE_STRING),
		NAME(T61_STRING),
		NAME(VIDEOTEX_STRING),
		NAME(IA5_STRING),
		NAME(UTC_TIME),
		NAME(GENERALIZED_TIME),
		NAME(GRAPHIC_STRING),
		NAME(VISIBLE_STRING),
		NAME(GENERAL_STRING),
		NAME(UNIVERSAL_STRING),
		NAME(CHARACTER_STRING),
		NAME(BMP_STRING),
		NAME(DATE),
		NAME(TIME_OF_DAY),
		NAME(DATE_TIME),
		NAME(DURATION),
		NAME(OID_IRI),
		NAME(RELATIVE_OID_IRI),
#undef NAME
	};
	const char *name;
	uint32_t type = asn1_object_get_type(object);
	if (type < sizeof(names) / sizeof(*names)
	 && names[type])
		name = names[type];
	else
		name = "UNKNOWN";
	printf("%-*s ", (int)(32 - indent * 2), name);
	switch (type)
	{
		case ASN1_BOOLEAN:
		{
			struct asn1_boolean *boolean = (struct asn1_boolean*)object;
			printf("%s\n", asn1_boolean_get(boolean) ? "true" : "false");
			break;
		}
		case ASN1_INTEGER:
		{
			struct bignum *bignum = NULL;
			struct asn1_integer *integer = (struct asn1_integer*)object;
			if (!asn1_integer_get_bignum(integer, &bignum))
			{
				fprintf(stderr, "asn1parse: failed to get integer value\n");
				return 0;
			}
			printf("(%d bits)\n", bignum_num_bits(bignum));
			bignum_free(bignum);
			break;
		}
		case ASN1_BIT_STRING:
		{
			size_t bits;
			struct asn1_bit_string *bitstring = (struct asn1_bit_string*)object;
			if (!asn1_bit_string_get(bitstring, NULL, &bits))
			{
				fprintf(stderr, "asn1parse: failed to get bitstring length\n");
				return 0;
			}
			printf("(%lu bits)\n", (unsigned long)bits);
			break;
		}
		case ASN1_OCTET_STRING:
		{
			size_t size;
			struct asn1_octet_string *octetstring = (struct asn1_octet_string*)object;
			if (!asn1_octet_string_get(octetstring, NULL, &size))
			{
				fprintf(stderr, "asn1parse: failed to get octetstring length\n");
				return 0;
			}
			printf("(%lu octets)\n", (unsigned long)size);
			break;
		}
		case ASN1_OBJECT_IDENTIFIER:
		{
			struct asn1_oid *oid = (struct asn1_oid*)object;
			char buf[512];
			printf("%s\n", oid2txt(oid, buf, sizeof(buf)));
			break;
		}
		case ASN1_UTF8_STRING:
		case ASN1_NUMERIC_STRING:
		case ASN1_PRINTABLE_STRING:
		case ASN1_T61_STRING:
		case ASN1_VIDEOTEX_STRING:
		case ASN1_IA5_STRING:
		case ASN1_GRAPHIC_STRING:
		case ASN1_VISIBLE_STRING:
		case ASN1_GENERAL_STRING:
		case ASN1_UNIVERSAL_STRING:
		case ASN1_BMP_STRING:
		{
			struct asn1_string *string = (struct asn1_string*)object;
			size_t size;
			if (!asn1_string_get(string, NULL, &size))
			{
				fprintf(stderr, "asn1parse: failed to get string length\n");
				return 0;
			}
			char *data = malloc(size + 1);
			if (!data)
			{
				fprintf(stderr, "asn1parse: allocation failed\n");
				return 0;
			}
			if (!asn1_string_get(string, data, &size))
			{
				free(data);
				fprintf(stderr, "asn1parse: failed to get string data\n");
				return 0;
			}
			data[size] = '\0';
			printf("%-.32s\n", data);
			free(data);
			break;
		}
		case ASN1_SEQUENCE:
		case ASN1_SET:
		case ASN1_CHARACTER_STRING:
		{
			struct asn1_constructed *constructed = (struct asn1_constructed*)object;
			struct asn1_object **objects;
			size_t size;
			if (!asn1_constructed_get0(constructed, &objects, &size))
			{
				fprintf(stderr, "asn1parse: failed to get constructed objects\n");
				return 0;
			}
			printf("(%lu entries)\n", (unsigned long)size);
			for (size_t i = 0; i < size; ++i)
			{
				if (!do_print(objects[i], indent + 1))
					return 0;
			}
			break;
		}
		case ASN1_UTC_TIME:
		case ASN1_GENERALIZED_TIME:
		{
			struct asn1_time *time = (struct asn1_time*)object;
			time_t t = asn1_time_get(time);
			printf("%s", ctime(&t));
			break;
		}
		default:
			printf("\n");
			break;
	}
	return 1;
}

static int print_constructed(struct asn1_object *object, size_t indent)
{
	char name[32];
	snprintf(name, sizeof(name), "CONSTRUCTED [%" PRIu32 "]",
	         asn1_object_get_type(object));
	printf("%-*s ", (int)(32 - indent * 2), name);
	struct asn1_constructed *constructed = (struct asn1_constructed*)object;
	struct asn1_object **objects;
	size_t size;
	if (!asn1_constructed_get0(constructed, &objects, &size))
	{
		fprintf(stderr, "asn1parse: failed to get constructed objects\n");
		return 0;
	}
	printf("(%lu entries)\n", (unsigned long)size);
	for (size_t i = 0; i < size; ++i)
	{
		if (!do_print(objects[i], indent + 1))
			return 0;
	}
	return 1;
}

static int print_primitive(struct asn1_object *object, size_t indent)
{
	char name[32];
	snprintf(name, sizeof(name), "PRIMITIVE [%" PRIu32 "]",
	         asn1_object_get_type(object));
	printf("%-*s ", (int)(32 - indent * 2), name);
	size_t size;
	struct asn1_primitive *primitive = (struct asn1_primitive*)object;
	if (!asn1_primitive_get(primitive, NULL, &size))
	{
		fprintf(stderr, "asn1parse: failed to get primitive length\n");
		return 0;
	}
	printf("(%lu bytes)\n", (unsigned long)size);
	return 1;
}

static int do_print(struct asn1_object *object, size_t indent)
{
	for (size_t i = 0; i < indent; ++i)
		printf("  ");
	switch (asn1_object_get_class(object))
	{
		case ASN1_UNIVERSAL:
			return print_universal(object, indent);
		default:
			if (asn1_object_get_constructed(object))
				return print_constructed(object, indent);
			return print_primitive(object, indent);
	}
	return 1;
}

static void usage(void)
{
	printf("asn1parse [options]\n");
	printf("-help:           display this help\n");
	printf("-inform format:  input format (PEM or DER)\n");
	printf("-outform format: output format (PEM or DER)\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
	printf("-noout:          don't output asn1\n");
	printf("-text:           output decoded asn1\n");
}

static int parse_args(struct cmd_asn1parse_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",        no_argument,       NULL, 'h'},
		{"inform",      required_argument, NULL, 'I'},
		{"outform",     required_argument, NULL, 'O'},
		{"in",          required_argument, NULL, 'i'},
		{"out",         required_argument, NULL, 'o'},
		{"noout",       no_argument,       NULL, 'n'},
		{"text",        no_argument,       NULL, 't'},
		{NULL,          0,                 NULL,  0 },
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
				if (!cmd_handle_format("asn1parse", optarg,
				                       &data->inform))
					return 0;
				break;
			case 'O':
				if (!cmd_handle_format("asn1parse", optarg,
				                       &data->outform))
					return 0;
				break;
			case 'i':
				if (!cmd_handle_in("asn1parse", optarg,
				                   &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("asn1parse", optarg,
				                    &data->fpout))
					return 0;
				break;
			case 'n':
				data->noout = 1;
				break;
			case 't':
				data->text = 1;
				break;
			default:
				return 0;
		}
	}
	return 1;
}

int cmd_asn1parse(int argc, char **argv)
{
	struct cmd_asn1parse_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.inform = CMD_FORMAT_PEM;
	data.outform = CMD_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	if (!parse_args(&data, argc, argv))
		return EXIT_FAILURE;
	if (!do_read(&data))
	{
		fprintf(stderr, "asn1parse: invalid asn1\n");
		goto end;
	}
	if (data.text && !do_print(data.object, 0))
		goto end;
	if (!data.noout && !do_write(&data))
	{
		fprintf(stderr, "asn1parse: failed to write asn1\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	asn1_object_free(data.object);
	return ret;
}
