#include "cmd/cmd.h"

#include <jkssl/bio.h>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

struct cmd_base64_data
{
	struct bio *bio_out;
	struct bio *bio_in;
	int mode;
};

static int encode(struct cmd_base64_data *data)
{
	uint8_t in[4096];
	ssize_t inl;
	int ret = EXIT_FAILURE;
	struct bio *b64;

	b64 = bio_new(bio_f_base64());
	if (!b64)
	{
		fprintf(stderr, "base64: malloc: %s\n", strerror(errno));
		goto end;
	}
	data->bio_out = bio_push(b64, data->bio_out);
	while ((inl = bio_read(data->bio_in, in, sizeof(in))) > 0)
	{
		if (bio_write(data->bio_out, in, inl) != inl)
		{
			fprintf(stderr, "base64: write: %s\n", strerror(errno));
			goto end;
		}
	}
	if (inl < 0)
	{
		fprintf(stderr, "base64: read: %s\n", strerror(errno));
		goto end;
	}
	if (bio_flush(data->bio_out) != 1)
	{
		fprintf(stderr, "base64: write: %s\n", strerror(errno));
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	return ret;
}

static int decode(struct cmd_base64_data *data)
{
	uint8_t in[4096];
	ssize_t inl;
	int ret = EXIT_FAILURE;
	struct bio *b64;

	b64 = bio_new(bio_f_base64());
	if (!b64)
	{
		fprintf(stderr, "base64: malloc: %s\n", strerror(errno));
		goto end;
	}
	data->bio_in = bio_push(b64, data->bio_in);
	while ((inl = bio_read(data->bio_in, in, sizeof(in))) > 0)
	{
		if (bio_write(data->bio_out, in, inl) != inl)
		{
			fprintf(stderr, "base64: write: %s\n", strerror(errno));
			goto end;
		}
	}
	if (inl < 0)
	{
		fprintf(stderr, "base64: read: %s\n", strerror(errno));
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	return ret;
}

static void usage(void)
{
	printf("base64 [options]\n");
	printf("-help:     display this help\n");
	printf("-in file:  input file\n");
	printf("-out file: output file\n");
	printf("-e:        encode mode\n");
	printf("-d:        decode mode\n");
}

static int parse_args(struct cmd_base64_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help", no_argument,       NULL, 'h'},
		{"in",   required_argument, NULL, 'i'},
		{"out",  required_argument, NULL, 'o'},
		{"e",    no_argument,       NULL, 'e'},
		{"d",    no_argument,       NULL, 'd'},
		{NULL,   0,                 NULL,   0},
	};
	int c;
	opterr = 1;
	while ((c = getopt_long_only(argc, argv, "", opts, NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				usage();
				return 0;
			case 'e':
				data->mode = 0;
				break;
			case 'd':
				data->mode = 1;
				break;
			case 'i':
				bio_free(data->bio_in);
				data->bio_in = bio_new_file(optarg, "r");
				if (!data->bio_in)
				{
					fprintf(stderr, "base64: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			case 'o':
				bio_free(data->bio_out);
				data->bio_out = bio_new_file(optarg, "w");
				if (!data->bio_out)
				{
					fprintf(stderr, "base64: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			default:
				usage();
				return 0;
		}
	}
	return 1;
}

int cmd_base64(int argc, char **argv)
{
	struct cmd_base64_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	if (!parse_args(&data, argc, argv))
		return EXIT_FAILURE;
	if (!data.bio_in)
	{
		data.bio_in = bio_new_fp(stdin, 0);
		if (!data.bio_in)
		{
			fprintf(stderr, "base64: allocation failed\n");
			goto end;
		}
	}
	if (!data.bio_out)
	{
		data.bio_out = bio_new_fp(stdout, 0);
		if (!data.bio_out)
		{
			fprintf(stderr, "base64: allocation failed\n");
			goto end;
		}
	}
	if (data.mode)
		ret = decode(&data);
	else
		ret = encode(&data);

end:
	bio_free_all(data.bio_in);
	bio_free_all(data.bio_out);
	return ret;
}
