#include "cmd/cmd.h"

#include "asn1/asn1.h" /* XXX */

#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <inttypes.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

enum asn1_format
{
	ASN1_FORMAT_PEM,
	ASN1_FORMAT_DER,
};

struct cmd_asn1parse_data
{
	enum asn1_format inform;
	enum asn1_format outform;
	FILE *fpout;
	FILE *fpin;
	struct asn1_object *object;
	int noout;
	int text;
};

static void do_print(struct asn1_object *object, size_t indent);

static int do_read(struct cmd_asn1parse_data *data)
{
	switch (data->inform)
	{
		case ASN1_FORMAT_PEM:
			return pem_read_asn1_object(data->fpin,
			                            &data->object) != NULL;
		case ASN1_FORMAT_DER:
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
		case ASN1_FORMAT_PEM:
			return pem_write_asn1_object(data->fpout,
			                             data->object);
		case ASN1_FORMAT_DER:
			return i2d_asn1_object_fp(data->fpout, data->object);
		default:
			return 0;
	}
}

static void print_universal(struct asn1_object *object, size_t indent)
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
	if (object->type < sizeof(names) / sizeof(*names)
	 && names[object->type])
		name = names[object->type];
	else
		name = "UNKNOWN";
	printf("%-*s ", (int)(32 - indent * 2), name);
	switch (object->type)
	{
		case ASN1_BOOLEAN:
		{
			struct asn1_boolean *boolean = (struct asn1_boolean*)object;
			printf("%s\n", boolean->value ? "true" : "false");
			break;
		}
		case ASN1_INTEGER:
		{
			struct asn1_integer *integer = (struct asn1_integer*)object;
			printf("(%lu bits)\n", (unsigned long)integer->size * 8);
			break;
		}
		case ASN1_BIT_STRING:
		{
			struct asn1_bit_string *bitstring = (struct asn1_bit_string*)object;
			printf("(%lu bits)\n", (unsigned long)bitstring->bits);
			break;
		}
		case ASN1_OCTET_STRING:
		{
			struct asn1_octet_string *octetstring = (struct asn1_octet_string*)object;
			printf("(%lu octets)\n", (unsigned long)octetstring->size);
			break;
		}
		case ASN1_OBJECT_IDENTIFIER:
		{
			struct asn1_oid *oid = (struct asn1_oid*)object;
			for (size_t i = 0; i < oid->values_count; ++i)
			{
				if (i)
					putchar('.');
				printf("%" PRIu32, oid->values[i]);
			}
			printf("\n");
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
			printf("%-.32s\n", string->data);
			break;
		}
		case ASN1_SEQUENCE:
		case ASN1_SET:
		case ASN1_CHARACTER_STRING:
		{
			struct asn1_constructed *constructed = (struct asn1_constructed*)object;
			printf("(%lu entries)\n", (unsigned long)constructed->objects_count);
			for (size_t i = 0; i < constructed->objects_count; ++i)
				do_print(constructed->objects[i], indent + 1);
			break;
		}
		case ASN1_UTC_TIME:
		case ASN1_GENERALIZED_TIME:
		{
			struct asn1_time *time = (struct asn1_time*)object;
			printf("%s", ctime(&time->value));
			break;
		}
		default:
			printf("\n");
			break;
	}
}

static void print_constructed(struct asn1_object *object, size_t indent)
{
	char name[32];
	snprintf(name, sizeof(name), "CONSTRUCTED [%" PRIu32 "]",
	         object->type);
	printf("%-*s ", (int)(32 - indent * 2), name);
	struct asn1_constructed *constructed = (struct asn1_constructed*)object;
	printf("(%lu entries)\n", (unsigned long)constructed->objects_count);
	for (size_t i = 0; i < constructed->objects_count; ++i)
		do_print(constructed->objects[i], indent + 1);
}

static void print_primitive(struct asn1_object *object, size_t indent)
{
	char name[32];
	snprintf(name, sizeof(name), "PRIMITIVE [%" PRIu32 "]", object->type);
	printf("%-*s ", (int)(32 - indent * 2), name);
	struct asn1_primitive *primitive = (struct asn1_primitive*)object;
	printf("(%lu bytes)\n", (unsigned long)primitive->size);
}

static void do_print(struct asn1_object *object, size_t indent)
{
	for (size_t i = 0; i < indent; ++i)
		printf("  ");
	switch (object->class)
	{
		case ASN1_UNIVERSAL:
			print_universal(object, indent);
			break;
		default:
			if (object->constructed)
				print_constructed(object, indent);
			else
				print_primitive(object, indent);
			break;
	}
}

static int handle_inform_outform(const char *arg, enum asn1_format *format)
{
	if (!strcmp(arg, "PEM"))
	{
		*format = ASN1_FORMAT_PEM;
		return 1;
	}
	if (!strcmp(arg, "DER"))
	{
		*format = ASN1_FORMAT_DER;
		return 1;
	}
	fprintf(stderr, "asn1parse: unsupported format: %s\n", arg);
	return 0;
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
				if (!handle_inform_outform(optarg,
				                           &data->inform))
					return 0;
				break;
			case 'O':
				if (!handle_inform_outform(optarg,
				                           &data->outform))
					return 0;
				break;
			case 'i':
				if (data->fpin != stdin)
					fclose(data->fpin);
				data->fpin = fopen(optarg, "r");
				if (!data->fpin)
				{
					fprintf(stderr, "asn1parse: open(%s): %s\n",
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
					fprintf(stderr, "asn1parse: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
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
	data.fpin = stdin;
	data.fpout = stdout;
	if (!parse_args(&data, argc, argv))
		return EXIT_FAILURE;
	if (!do_read(&data))
	{
		fprintf(stderr, "asn1parse: invalid asn1\n");
		goto end;
	}
	if (data.text)
		do_print(data.object, 0);
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
