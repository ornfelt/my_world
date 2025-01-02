#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/x509.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

struct cmd_x509_data
{
	enum cmd_format inform;
	enum cmd_format outform;
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
		case CMD_FORMAT_PEM:
			return pem_read_x509(data->fpin,
			                     &data->x509) != NULL;
		case CMD_FORMAT_DER:
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
		case CMD_FORMAT_PEM:
			return pem_write_x509(data->fpout,
			                      data->x509);
		case CMD_FORMAT_DER:
			return i2d_x509_fp(data->fpout, data->x509);
		default:
			return 0;
	}
}

static void do_print(struct x509 *x509)
{
	x509_print_fp(stdout, x509);
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
				if (!cmd_handle_format("x509", optarg,
				                       &data->inform))
					return 0;
				break;
			case 'O':
				if (!cmd_handle_format("x509", optarg,
				                       &data->outform))
					return 0;
				break;
			case 'i':
				if (!cmd_handle_in("x509", optarg,
				                   &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("x509", optarg,
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

int cmd_x509(int argc, char **argv)
{
	struct cmd_x509_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.inform = CMD_FORMAT_PEM;
	data.outform = CMD_FORMAT_PEM;
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
