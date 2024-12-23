#include "cmd/cmd.h"

#include <jkssl/x509.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

enum x509_format
{
	X509_FORMAT_PEM,
	X509_FORMAT_DER,
};

struct cmd_x509_data
{
	enum x509_format inform;
	enum x509_format outform;
	FILE *fpout;
	FILE *fpin;
	struct x509 *x509;
	int noout;
	int text;
};

static int do_read(struct cmd_x509_data *data)
{
	switch (data->inform)
	{
		case X509_FORMAT_PEM:
			return pem_read_x509(data->fpin,
			                     &data->x509) != NULL;
		case X509_FORMAT_DER:
			return d2i_x509_fp(data->fpin,
			                   &data->x509) != NULL;
		default:
			return 0;
	}
}

static int do_write(struct cmd_x509_data *data)
{
	switch (data->outform)
	{
		case X509_FORMAT_PEM:
			return pem_write_x509(data->fpout,
			                      data->x509);
		case X509_FORMAT_DER:
			return i2d_x509_fp(data->fpout, data->x509);
		default:
			return 0;
	}
}

static void do_print(struct x509 *x509)
{
	x509_print_fp(stdout, x509);
}

static int handle_inform_outform(const char *arg, enum x509_format *format)
{
	if (!strcmp(arg, "PEM"))
	{
		*format = X509_FORMAT_PEM;
		return 1;
	}
	if (!strcmp(arg, "DER"))
	{
		*format = X509_FORMAT_DER;
		return 1;
	}
	fprintf(stderr, "x509: unsupported format: %s\n", arg);
	return 0;
}

static void usage(void)
{
	printf("x509 [options]\n");
	printf("-help:           display this help\n");
	printf("-inform format:  input format (PEM or DER)\n");
	printf("-outform format: output format (PEM or DER)\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
	printf("-noout:          don't output x509\n");
	printf("-text:           output decoded x509\n");
}

static int parse_args(struct cmd_x509_data *data, int argc, char **argv)
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
					fprintf(stderr, "x509: open(%s): %s\n",
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
					fprintf(stderr, "x509: open(%s): %s\n",
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

int cmd_x509(int argc, char **argv)
{
	struct cmd_x509_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.fpin = stdin;
	data.fpout = stdout;
	data.x509 = x509_new();
	if (!data.x509)
	{
		fprintf(stderr, "x509: malloc: %s\n", strerror(errno));
		goto end;
	}
	if (!parse_args(&data, argc, argv))
		return EXIT_FAILURE;
	if (!do_read(&data))
	{
		fprintf(stderr, "x509: invalid x509\n");
		goto end;
	}
	if (data.text)
		do_print(data.x509);
	if (!data.noout && !do_write(&data))
	{
		fprintf(stderr, "x509: failed to write x509\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	x509_free(data.x509);
	return ret;
}
