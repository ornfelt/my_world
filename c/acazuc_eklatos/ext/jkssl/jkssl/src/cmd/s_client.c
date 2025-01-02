#include "cmd/cmd.h"

#include <jkssl/rand.h>
#include <jkssl/ssl.h>
#include <jkssl/bio.h>

#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct cmd_s_client_data
{
	const char *conn;
	struct ssl_ctx *ssl_ctx;
	struct ssl *ssl;
	struct bio *bio;
};

static void usage(void)
{
	printf("s_client [options]\n");
	printf("-help: show this help\n");
	printf("-connect host:port: connect to the given destination\n");
}

static int parse_args(struct cmd_s_client_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",    no_argument,       NULL, 'h'},
		{"connect", required_argument, NULL, 'c'},
		{NULL,      0,                 NULL,  0 },
	};
	int c;
	while ((c = getopt_long_only(argc, argv, "", opts, NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				usage();
				return 1;
			case 'c':
				data->conn = optarg;
				break;
			default:
				usage();
				return 0;
		}
	}
	return 1;
}

int cmd_s_client(int argc, char **argv)
{
	struct cmd_s_client_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	if (!parse_args(&data, argc, argv))
		goto end;
	data.ssl_ctx = ssl_ctx_new(sslv23_method());
	if (!data.ssl_ctx)
	{
		fprintf(stderr, "s_client: failed to create ssl ctx\n");
		goto end;
	}
	data.bio = bio_new_ssl_connect(data.ssl_ctx);
	if (!data.bio)
	{
		fprintf(stderr, "s_client: failed to create ssl bio\n");
		goto end;
	}
	if (!bio_set_conn_hostname(data.bio, data.conn))
	{
		fprintf(stderr, "s_client: failed to set hostname\n");
		goto end;
	}
	if (!bio_get_ssl(data.bio, &data.ssl))
	{
		fprintf(stderr, "s_client: failed to get ssl\n");
		goto end;
	}
	if (!bio_do_connect(data.bio))
	{
		fprintf(stderr, "s_client: failed to connect\n");
		goto end;
	}
	if (!bio_do_handshake(data.bio))
	{
		fprintf(stderr, "s_client: handshake failed\n");
		goto end;
	}
	ret = EXIT_SUCCESS;

end:
	if (data.bio)
		bio_vfree(data.bio);
	if (data.ssl_ctx)
		ssl_ctx_free(data.ssl_ctx);
	if (data.ssl)
		ssl_free(data.ssl);
	return ret;
}
