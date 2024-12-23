#include "cmd/cmd.h"

#include <jkssl/rand.h>
#include <jkssl/ssl.h>
#include <jkssl/bio.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define HELLO_REQUEST       0
#define CLIENT_HELLO        1
#define SERVER_HELLO        2
#define CERTIFICATE         11
#define SERVER_KEY_EXCHANGE 12
#define CERTIFICATE_REQUEST 13
#define SERVER_HELLO_DONE   14
#define CERTIFICATE_VERIFY  15
#define CLIENT_KEY_EXCHANGE 16
#define FINISHED            20

#define CHANGE_CIPHER_SPEC 20
#define ALERT              21
#define HANDSHAKE          22
#define APPLICATION_DATA   23

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htonl24(v) ((htonl(v) >> 8))
#else
#define htonl24(v) v
#endif

struct handshake
{
	uint32_t msg_type : 8;
	uint32_t length : 24;
} __attribute__((packed));

struct protocol_version
{
	uint8_t major;
	uint8_t minor;
} __attribute__((packed));

struct cipher_suite
{
	uint8_t data[2];
} __attribute__((packed));

struct tls_plain_text
{
	uint8_t content_type;
	struct protocol_version protocol_version;
	uint16_t length;
} __attribute__((packed));

struct cmd_s_client_data
{
	const char *conn;
	struct ssl *ssl;
	int fd;
	uint8_t client_random[28];
	uint8_t client_id[32];
};

static void usage(void)
{
	printf("s_client [options]\n");
	printf("-help: show this help\n");
	printf("-connect host:port: connect to the given destination\n");
}

static int do_connect(struct cmd_s_client_data *data)
{
	struct addrinfo *addrs = NULL;
	char *node = NULL;
	char *service = NULL;
	char *sc;
	int ret = 0;
	int res;

	if (!data->conn)
	{
		fprintf(stderr, "s_client: no host given\n");
		goto end;
	}
	sc = strchr(data->conn, ':');
	if (sc)
	{
		if (sc != data->conn)
		{
			node = malloc(sc - data->conn + 1);
			if (!node)
			{
				fprintf(stderr, "s_client: malloc: %s\n",
				        strerror(errno));
				goto end;
			}
			memcpy(node, data->conn, sc - data->conn);
			node[sc - data->conn] = '\0';
		}
		else if (!sc[1])
		{
			fprintf(stderr, "s_client: invalid host\n");
			goto end;
		}
		if (sc[1])
		{
			service = strdup(&sc[1]);
			if (!service)
			{
				fprintf(stderr, "s_client: strdup: %s\n",
				        strerror(errno));
				goto end;
			}
		}
	}
	else
	{
		node = strdup(data->conn);
		if (!node)
		{
			fprintf(stderr, "s_client: strdup: %s\n", strerror(errno));
			goto end;
		}
		service = strdup("https");
		if (!service)
		{
			fprintf(stderr, "s_client: strdup: %s\n", strerror(errno));
			goto end;
		}
	}
	data->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (data->fd == -1)
	{
		fprintf(stderr, "s_client: socket: %s\n", strerror(errno));
		goto end;
	}
	res = getaddrinfo(node, service, NULL, &addrs);
	if (res)
	{
		fprintf(stderr, "s_client: getaddrinfo: %s\n", gai_strerror(res));
		goto end;
	}
	if (connect(data->fd, addrs->ai_addr, addrs->ai_addrlen) == -1)
	{
		fprintf(stderr, "s_client: connect: %s\n", strerror(errno));
		goto end;
	}
	ret = 1;

end:
	freeaddrinfo(addrs);
	free(node);
	free(service);
	return ret;
}

static int send_record_layer(struct cmd_s_client_data *data,
                             uint8_t content_type,
                             const uint8_t *payload, size_t length)
{
	struct bio *msg;
	struct buf_mem *mem;
	struct tls_plain_text hdr;
	int ret = 0;

	if (length > (1 << 14))
	{
		fprintf(stderr, "s_client: message too long\n");
		return 0;
	}
	msg = bio_new(bio_s_mem());
	if (!msg)
	{
		fprintf(stderr, "s_client: malloc: %s\n", strerror(errno));
		return 0;
	}
	hdr.content_type = content_type;
	hdr.protocol_version.major = 3;
	hdr.protocol_version.minor = 3;
	hdr.length = htons(length);
	if (bio_write(msg, &hdr, sizeof(hdr)) != sizeof(hdr))
	{
		fprintf(stderr, "s_client: failed to write record header\n");
		goto end;
	}
	if (bio_write(msg, payload, length) != (ssize_t)length)
	{
		fprintf(stderr, "s_client: failed to write record data\n");
		goto end;
	}
	bio_get_mem_ptr(msg, &mem);
	if (send(data->fd, mem->data, mem->size, 0) != (ssize_t)mem->size)
	{
		fprintf(stderr, "s_client: failed to send data: %s\n",
		        strerror(errno));
		goto end;
	}
	ret = 1;

end:
	bio_vfree(msg);
	return ret;
}

static int send_client_hello(struct cmd_s_client_data *data)
{
	struct bio *msg = bio_new(bio_s_mem());
	struct handshake hdr;
	struct protocol_version protocol_version;
	uint32_t gmt;
	uint8_t client_id_length;
	uint16_t cipher_suites_length;
	struct cipher_suite cipher_suites[2];
	uint8_t compression_length;
	uint8_t compression;
	struct buf_mem *mem;
	int ret = 0;
	if (!msg)
	{
		fprintf(stderr, "s_client: malloc: %s\n", strerror(errno));
		return 0;
	}
	hdr.msg_type = CLIENT_HELLO;
	hdr.length = 2  /* protocol version */
	           + 4  /* gmt */
	           + 28 /* random */
	           + 1  /* session id length */
	           + 32 /* session id */
	           + 2  /* cipher suites length */
	           + 4  /* cipher suites */
	           + 1  /* compression length */
	           + 1; /* compression */
	hdr.length = htonl24(hdr.length);
	if (bio_write(msg, &hdr, sizeof(hdr)) != sizeof(hdr))
	{
		fprintf(stderr, "s_client: failed to write client hello header\n");
		goto end;
	}
	protocol_version.major = 3;
	protocol_version.minor = 3;
	if (bio_write(msg, &protocol_version, sizeof(protocol_version)) != sizeof(protocol_version))
	{
		fprintf(stderr, "s_client: failed to write protocol version\n");
		goto end;
	}
	gmt = htonl(time(NULL));
	if (bio_write(msg, &gmt, sizeof(gmt)) != sizeof(gmt))
	{
		fprintf(stderr, "s_client: failed to write gmt\n");
		goto end;
	}
	if (rand_bytes(data->client_random, sizeof(data->client_random)) != 1)
	{
		fprintf(stderr, "s_client: failed to generate client random\n");
		goto end;
	}
	if (bio_write(msg, data->client_random, sizeof(data->client_random)) != sizeof(data->client_random))
	{
		fprintf(stderr, "s_client: failed to write client random\n");
		goto end;
	}
	client_id_length = sizeof(data->client_id);
	if (bio_write(msg, &client_id_length, sizeof(client_id_length)) != sizeof(client_id_length))
	{
		fprintf(stderr, "s_client: failed to write client id length\n");
		goto end;
	}
	if (rand_bytes(data->client_id, sizeof(data->client_id)) != 1)
	{
		fprintf(stderr, "s_client: failed to generate client id\n");
		goto end;
	}
	if (bio_write(msg, data->client_id, sizeof(data->client_id)) != sizeof(data->client_id))
	{
		fprintf(stderr, "s_client: failed to write client id\n");
		goto end;
	}
	cipher_suites_length = htons(4);
	if (bio_write(msg, &cipher_suites_length, sizeof(cipher_suites_length)) != sizeof(cipher_suites_length))
	{
		fprintf(stderr, "s_client: failed to write cipher suites length\n");
		goto end;
	}
	/* TLS_DHE_RSA_WITH_AES_128_CBC_SHA256 */
	cipher_suites[0].data[0] = 0x00;
	cipher_suites[0].data[1] = 0x67;
	/* TLS_DHE_RSA_WITH_AES_256_CBC_SHA256 */
	cipher_suites[1].data[0] = 0x00;
	cipher_suites[1].data[1] = 0x6B;
	if (bio_write(msg, cipher_suites, sizeof(cipher_suites)) != sizeof(cipher_suites))
	{
		fprintf(stderr, "s_client: failed to write cipher suites\n");
		goto end;
	}
	compression_length = 1;
	if (bio_write(msg, &compression_length, sizeof(compression_length)) != sizeof(compression_length))
	{
		fprintf(stderr, "s_client: failed to write compression length\n");
		goto end;
	}
	compression = 0;
	if (bio_write(msg, &compression, sizeof(compression)) != sizeof(compression))
	{
		fprintf(stderr, "s_client: failed to write compression\n");
		goto end;
	}
	bio_get_mem_ptr(msg, &mem);
	ret = send_record_layer(data, HANDSHAKE, mem->data, mem->size);

end:
	bio_vfree(msg);
	return ret;
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
	data.fd = -1;
	if (!parse_args(&data, argc, argv))
		goto end;
	if (!do_connect(&data))
		goto end;
	if (!send_client_hello(&data))
		goto end;
	ret = EXIT_SUCCESS;

end:
	close(data.fd);
	ssl_free(data.ssl);
	return ret;
}
