#include "utils/utils.h"
#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/rand.h>
#include <jkssl/bio.h>
#include <jkssl/evp.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

struct cmd_enc_data
{
	const struct evp_cipher *evp_cipher;
	const struct evp_md *evp_md;
	char *buf;
	uint32_t buf_len;
	uint32_t buf_pos;
	uint8_t salt[8];
	uint8_t *key;
	uint8_t *iv;
	int base64;
	struct bio *bio_out;
	struct bio *bio_in;
	int use_pbkdf2;
	int iterations;
	int nosalt;
	int enc;
	int list;
	int opt_p;
	int opt_P;
	const char *arg_pass;
	const char *arg_salt;
	const char *arg_key;
	const char *arg_iv;
};

static int do_dec(struct cmd_enc_data *data)
{
	uint8_t buf[4096];
	ssize_t bufl;
	int ret = 0;

	if (data->base64)
	{
		struct bio *b64 = bio_new(bio_f_base64());
		if (!b64)
		{
			fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
			goto end;
		}
		data->bio_in = bio_push(b64, data->bio_in);
	}
	{
		struct bio *cip = bio_new(bio_f_cipher());
		if (!cip)
		{
			fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
			goto end;
		}
		if (!bio_set_cipher(cip, data->evp_cipher, data->key, data->iv, 0))
		{
			fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
			goto end;
		}
		data->bio_in = bio_push(cip, data->bio_in);
	}
	while ((bufl = bio_read(data->bio_in, buf, sizeof(buf))) > 0)
	{
		if (bio_write(data->bio_out, buf, bufl) != bufl)
		{
			fprintf(stderr, "enc: write: %s\n", strerror(errno));
			goto end;
		}
	}
	if (bufl < 0)
	{
		fprintf(stderr, "enc: read: %s\n", strerror(errno));
		goto end;
	}
	if (bio_flush(data->bio_out) != 1)
	{
		fprintf(stderr, "enc: write: %s\n", strerror(errno));
		goto end;
	}
	ret = 1;

end:
	return ret;
}

static int do_enc(struct cmd_enc_data *data)
{
	uint8_t buf[4096];
	ssize_t bufl;
	int ret = 0;

	if (data->base64)
	{
		struct bio *b64 = bio_new(bio_f_base64());
		if (!b64)
		{
			fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
			goto end;
		}
		data->bio_out = bio_push(b64, data->bio_out);
	}
	{
		struct bio *cip = bio_new(bio_f_cipher());
		if (!cip)
		{
			fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
			goto end;
		}
		if (!bio_set_cipher(cip, data->evp_cipher, data->key, data->iv, 1))
		{
			fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
			goto end;
		}
		data->bio_out = bio_push(cip, data->bio_out);
	}
	while ((bufl = bio_read(data->bio_in, buf, sizeof(buf))) > 0)
	{
		if (bio_write(data->bio_out, buf, bufl) != bufl)
		{
			fprintf(stderr, "enc: write: %s\n", strerror(errno));
			goto end;
		}
	}
	if (bufl == -1)
	{
		fprintf(stderr, "enc: read: %s\n", strerror(errno));
		goto end;
	}
	if (bio_flush(data->bio_out) != 1)
	{
		fprintf(stderr, "enc: write: %s\n", strerror(errno));
		goto end;
	}
	ret = 1;

end:
	return ret;
}

struct cipher_list_state
{
	size_t n;
};

static int cipher_foreach(const struct evp_cipher *cipher, void *data)
{
	struct cipher_list_state *state = data;
	char name[64];
	snprintf(name, sizeof(name), "-%s", evp_cipher_get0_name(cipher));
	for (size_t i = 0; name[i]; ++i)
		name[i] = tolower(name[i]);
	printf("%s", name);
	if (state->n % 3 == 2)
	{
		printf("\n");
	}
	else
	{
		size_t len = 1 + strlen(evp_cipher_get0_name(cipher));
		if (len < 24)
			printf("%.*s", (int)(24 - len), "                        ");
	}
	state->n++;
	return 1;
}

static int transform_bin(uint8_t *bin, const char *str, int max)
{
	int len;

	memset(bin, 0, max);
	len = strlen(str);
	if (len > max * 2)
		len = max * 2;
	if (!hex2bin(bin, str, len))
		return 0;
	return 1;
}

static void list_ciphers(void)
{
	printf("Supported ciphers:\n");
	struct cipher_list_state state;
	state.n = 0;
	evp_foreach_cipher(cipher_foreach, &state);
	if (state.n % 3 != 0)
		printf("\n");
}

static int generate_key(struct cmd_enc_data *data)
{
	uint8_t dummy[EVP_MAX_IV_LENGTH];
	char *pass;
	int ret;

	if (!data->arg_pass)
		pass = cmd_ask_password_confirm();
	else
		pass = strdup(data->arg_pass);
	if (!pass)
	{
		fprintf(stderr, "enc: failed to get password\n");
		return 0;
	}
	if (data->use_pbkdf2)
	{
		uint8_t key_iv[EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH];
		size_t key_size = evp_cipher_get_key_length(data->evp_cipher);
		size_t iv_size = evp_cipher_get_iv_length(data->evp_cipher);
		ret = pkcs5_pbkdf2_hmac(pass, strlen(pass),
		                        data->nosalt ? NULL : data->salt,
		                        data->nosalt ? 0 : sizeof(data->salt),
		                        data->iterations ? data->iterations : 10000,
		                        data->evp_md, key_size + iv_size, key_iv);
		if (ret)
		{
			memcpy(data->key, key_iv, key_size);
			if (!data->arg_iv)
				memcpy(data->iv, &key_iv[key_size], iv_size);
		}
	}
	else
	{
		ret = evp_bytestokey(data->evp_cipher, data->evp_md,
		                     data->nosalt ? NULL : data->salt,
		                     (uint8_t*)pass, strlen(pass), 1,
		                     data->key, data->arg_iv ? dummy : data->iv);
	}
	free(pass);
	return ret;
}

static int handle_key_iv(struct cmd_enc_data *data)
{
	data->key = malloc(evp_cipher_get_key_length(data->evp_cipher));
	if (!data->key)
	{
		fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
		return 0;
	}
	data->iv = malloc(evp_cipher_get_iv_length(data->evp_cipher));
	if (!data->iv)
	{
		fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
		return 0;
	}
	if (data->arg_iv)
	{
		if (!transform_bin(data->iv, data->arg_iv,
		                   evp_cipher_get_iv_length(data->evp_cipher)))
		{
			fprintf(stderr, "enc: invalid iv\n");
			return 0;
		}
	}
	if (!data->arg_key)
	{
		if (data->arg_salt)
		{
			if (!transform_bin(data->salt, data->arg_salt,
			                   sizeof(data->salt)))
			{
				fprintf(stderr, "enc invalid salt\n");
				return 0;
			}
		}
		else if (rand_bytes(data->salt, sizeof(data->salt)) != 1)
		{
			fprintf(stderr, "enc: failed to get random\n");
			return 0;
		}
		if (!generate_key(data))
		{
			fprintf(stderr, "enc: failed to generate password\n");
			return 0;
		}
	}
	else if (!transform_bin(data->key, data->arg_key,
	                        evp_cipher_get_key_length(data->evp_cipher)))
	{
		fprintf(stderr, "enc: invalid key\n");
		return 0;
	}
	return 1;
}

static void print_key_iv(struct cmd_enc_data *data)
{
	printf("salt=");
	for (size_t i = 0; i < 8; ++i)
		printf("%02X", data->salt[i]);
	printf("\n");
	printf("key=");
	for (size_t i = 0; i < evp_cipher_get_key_length(data->evp_cipher); ++i)
		printf("%02X", data->key[i]);
	printf("\n");
	printf("iv =");
	for (size_t i = 0; i < evp_cipher_get_iv_length(data->evp_cipher); ++i)
		printf("%02X", data->iv[i]);
	printf("\n");
}

static void usage(void)
{
	printf("cipher [options]\n");
	printf("-help:       display this help\n");
	printf("-in file:    input file\n");
	printf("-out file:   output file\n");
	printf("-iv iv:      initialization vector\n");
	printf("-e:          encrypt mode\n");
	printf("-d:          decrypt mode\n");
	printf("-a:          base64 input / output given the mode\n");
	printf("-base64:     same as -a\n");
	printf("-K:          key as hex string\n");
	printf("-k:          passphrase\n");
	printf("-pass pass:  the password to derive key from\n");
	printf("-S salt:     salt\n");
	printf("-list:       list available ciphers\n");
	printf("-ciphers:    alias for -list\n");
	printf("-p:          print iv and key\n");
	printf("-P:          print iv and key then exit\n");
	printf("-pbkdf2:     use pbkdf2 for key derivation\n");
	printf("-md digest:  digest algorithm to derive key\n");
	printf("-iter count: use pbkdf2 with the given number of iterations\n");
	printf("-*:          cipher algorithm to use\n");
}

static int parse_args(struct cmd_enc_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",    no_argument      , NULL, 'h'},
		{"in",      required_argument, NULL, 'i'},
		{"out",     required_argument, NULL, 'o'},
		{"iv",      required_argument, NULL, 'v'},
		{"e",       no_argument,       NULL, 'e'},
		{"d",       no_argument,       NULL, 'd'},
		{"a",       no_argument,       NULL, 'a'},
		{"base64",  no_argument,       NULL, 'a'},
		{"K",       required_argument, NULL, 'K'},
		{"k",       required_argument, NULL, 'k'},
		{"pass",    required_argument, NULL, 'm'},
		{"S",       required_argument, NULL, 'S'},
		{"list",    no_argument,       NULL, 'l'},
		{"ciphers", no_argument,       NULL, 'l'},
		{"p",       no_argument,       NULL, 'p'},
		{"P",       no_argument,       NULL, 'P'},
		{"salt",    no_argument,       NULL, '0'},
		{"nosalt",  no_argument,       NULL, '1'},
		{"pbkdf2",  no_argument,       NULL, 'H'},
		{"md",      required_argument, NULL, 'M'},
		{"iter",    required_argument, NULL, 'I'},
		{NULL,      0,                 NULL,  0 },
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
			case 'i':
				bio_free(data->bio_in);
				data->bio_in = bio_new_file(optarg, "r");
				if (!data->bio_in)
				{
					fprintf(stderr, "enc: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			case 'o':
				bio_free(data->bio_out);
				data->bio_out = bio_new_file(optarg, "w");
				if (!data->bio_out)
				{
					fprintf(stderr, "enc: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			case 'v':
				data->arg_iv = optarg;
				break;
			case 'e':
				data->enc = 1;
				break;
			case 'd':
				data->enc = 0;
				break;
			case 'a':
				data->base64 = 1;
				break;
			case 'K':
				data->arg_key = optarg;
				break;
			case 'k':
				data->arg_pass = optarg;
				break;
			case 'm':
				data->arg_pass = optarg;
				break;
			case 'S':
				data->arg_salt = optarg;
				break;
			case 'l':
				data->list = 1;
				break;
			case 'p':
				data->opt_p = 1;
				break;
			case 'P':
				data->opt_P = 1;
				break;
			case '0':
				data->nosalt = 0;
				break;
			case '1':
				data->nosalt = 1;
				break;
			case 'H':
				data->use_pbkdf2 = 1;
				break;
			case 'M':
				data->evp_md = evp_get_digestbyname(optarg);
				if (!data->evp_md)
				{
					fprintf(stderr, "enc: unknown digest: %s\n",
					        optarg);
					usage();
					return 0;
				}
				break;
			case 'I':
			{
				errno = 0;
				long iterations = strtol(optarg, NULL, 10);
				if (errno || iterations < 0 || iterations >= INT_MAX)
				{
					fprintf(stderr, "enc: invalid iterations count: %s\n",
					        optarg);
					usage();
					return 0;
				}
				data->use_pbkdf2 = 1;
				data->iterations = iterations;
				break;
			}
			default:
				data->evp_cipher = evp_get_cipherbyname(argv[optind - 1] + 1);
				if (!data->evp_cipher)
				{
					fprintf(stderr, "enc: unrecognized option: %s\n",
					        argv[optind - 1]);
					usage();
					return 0;
				}
				break;
		}
	}
	return 1;
}

int cmd_enc(int argc, char **argv)
{
	struct cmd_enc_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	data.buf_len = 1024 * 16;
	data.buf = malloc(data.buf_len);
	data.evp_md = evp_sha256();
	data.enc = 1;
	if (!data.buf)
	{
		fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
		goto end;
	}
	if (!parse_args(&data, argc, argv))
		goto end;
	if (data.list)
	{
		list_ciphers();
		goto end;
	}
	if (!handle_key_iv(&data))
		goto end;
	if (data.opt_P)
	{
		print_key_iv(&data);
		ret = EXIT_SUCCESS;
		goto end;
	}
	if (data.opt_p)
		print_key_iv(&data);
	if (!data.bio_in)
	{
		data.bio_in = bio_new_fp(stdin, 0);
		if (!data.bio_in)
		{
			fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
			goto end;
		}
	}
	if (!data.bio_out)
	{
		data.bio_out = bio_new_fp(stdout, 0);
		if (!data.bio_out)
		{
			fprintf(stderr, "enc: malloc: %s\n", strerror(errno));
			goto end;
		}
	}
	if (!data.evp_cipher)
	{
		fprintf(stderr, "enc: no cipher given\n");
		goto end;
	}
	if (data.enc)
	{
		if (!do_enc(&data))
			goto end;
	}
	else
	{
		if (!do_dec(&data))
			goto end;
	}
	ret = EXIT_SUCCESS;

end:
	bio_free_all(data.bio_in);
	bio_free_all(data.bio_out);
	free(data.buf);
	free(data.key);
	free(data.iv);
	return ret;
}
