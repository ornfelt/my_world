#include "cmd/common.h"
#include "cmd/cmd.h"

#include <jkssl/bignum.h>
#include <jkssl/rsa.h>
#include <jkssl/pem.h>
#include <jkssl/der.h>

#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

enum rsautl_mode
{
	RSAUTL_MODE_ENCRYPT,
	RSAUTL_MODE_DECRYPT,
	RSAUTL_MODE_SIGN,
	RSAUTL_MODE_VERIFY,
};

struct cmd_rsautl_data
{
	struct rsa *rsa;
	uint8_t *indata;
	size_t inlen;
	uint8_t *outdata;
	size_t outlen;
	FILE *fpout;
	FILE *fpin;
	FILE *fpkey;
	enum cmd_format keyform;
	int pubin;
	enum rsautl_mode mode;
	int hexdump;
	char *passin;
	size_t hex_pos;
	uint8_t hex_line[16];
	int hex_write;
	int padding;
};

static char to_hex(char c)
{
	if (c >= 10)
		return 'a' + c - 10;
	return '0' + c;
}

static void do_write_prev(struct cmd_rsautl_data *data)
{
	if (data->hex_pos % 16 == 8)
	{
		fprintf(data->fpout, "0");
	}
	else if (data->hex_pos % 16)
	{
		fprintf(data->fpout, " ");
	}
	else
	{
		fprintf(data->fpout, "%c", to_hex((data->hex_pos >> 12) & 0xF));
		fprintf(data->fpout, "%c", to_hex((data->hex_pos >> 8) & 0xF));
		fprintf(data->fpout, "%c", to_hex((data->hex_pos >> 4) & 0xF));
		fprintf(data->fpout, "%c", to_hex((data->hex_pos >> 0) & 0xF));
		fprintf(data->fpout, " - ");
	}
}

static int do_write_hex_line(struct cmd_rsautl_data *data)
{
	if (!data->hex_write)
		return 1;
	data->hex_write = 0;
	for (size_t i = 6 + ((data->hex_pos + 15) % 16 + 1) * 3; i < 5 + 4 + 16 * 3; ++i)
	{
		if (fprintf(data->fpout, " ") < 0)
			return 0;
	}
	for (size_t i = 0; i < ((data->hex_pos + 15) % 16) + 1; ++i)
	{
		if (isprint(data->hex_line[i]))
		{
			if (fprintf(data->fpout, "%c", data->hex_line[i]) < 0)
				return 0;
		}
		else
		{
			if (fprintf(data->fpout, ".") < 0)
				return 0;
		}
	}
	if (fprintf(data->fpout, "\n") < 0)
		return 0;
	return 1;
}

static int do_write_hex(struct cmd_rsautl_data *data, const uint8_t *ptr, size_t len)
{
	if (!data->hexdump)
	{
		if (fwrite(ptr, 1, len, data->fpout) != len)
			return 0;
		return 1;
	}
	for (size_t i = 0; i < len; ++i)
	{
		do_write_prev(data);
		if (fprintf(data->fpout, "%c", to_hex((ptr[i] >> 4) & 0xF)) < 0)
			return 0;
		if (fprintf(data->fpout, "%c", to_hex((ptr[i] >> 0) & 0xF)) < 0)
			return 0;
		data->hex_line[data->hex_pos % 16] = ptr[i];
		data->hex_write = 1;
		if (++data->hex_pos % 16 == 0)
		{
			if (!do_write_hex_line(data))
				return 0;
		}
	}
	return 1;
}

static int do_write(struct cmd_rsautl_data *data)
{
	if (!do_write_hex(data, data->outdata, data->outlen))
	{
		fprintf(stderr, "rsautl: write: %s\n", strerror(errno));
		return 0;
	}
	if (!do_write_hex_line(data))
	{
		fprintf(stderr, "rsautl: write: %s\n", strerror(errno));
		return 0;
	}
	return 1;
}

static int do_readkey(struct cmd_rsautl_data *data)
{
	switch (data->keyform)
	{
		case CMD_FORMAT_PEM:
			if (data->pubin)
				return pem_read_rsa_pubkey(data->fpkey,
				                           &data->rsa,
				                           data->passin ? NULL : cmd_ask_pass,
				                           data->passin) != NULL;
			return pem_read_rsa_private_key(data->fpkey,
			                                &data->rsa,
			                                data->passin ? NULL : cmd_ask_pass,
			                                data->passin) != NULL;
		case CMD_FORMAT_DER:
			if (data->pubin)
				return d2i_rsa_pubkey_fp(data->fpkey,
				                         &data->rsa) != NULL;
			return d2i_rsa_private_key_fp(data->fpkey,
			                              &data->rsa) != NULL;
		default:
			return 0;
	}
}

static int do_read(struct cmd_rsautl_data *data)
{
	ssize_t len = rsa_size(data->rsa);
	if (len == -1)
	{
		fprintf(stderr, "rsautl: invalid RSA ctx\n");
		return 0;
	}
	data->indata = malloc(len + 1);
	if (!data->indata)
	{
		fprintf(stderr, "rsautl: malloc: %s\n", strerror(errno));
		return 0;
	}
	size_t rd = fread(data->indata, 1, len + 1, data->fpin);
	if (ferror(data->fpin))
	{
		fprintf(stderr, "rsautl: read: %s\n", strerror(errno));
		return 0;
	}
	data->inlen = rd;
	return 1;
}

static void usage(void)
{
	printf("rsautl [options]\n");
	printf("-help:           display this help\n");
	printf("-in file:        input file\n");
	printf("-out file:       output file\n");
	printf("-pubin:          parse input as pubkey\n");
	printf("-encrypt:        encrypt data with public key\n");
	printf("-decrypt:        decrypt data with private key\n");
	printf("-hexdump:        output as hexdump\n");
	printf("-passin pass:    use password as private key password\n");
	printf("-keyform format: key format (PEM or DER)\n");
	printf("-inkey file:     input key\n");
	printf("-sign:           sign data with private key\n");
	printf("-verify:         verify data with public key\n");
	printf("-pkcs:           use PKCS#1 v1.5 padding\n");
	printf("-oaep:           use PKCS#1 OAEP padding\n");
	printf("-raw:            don't use padding\n");
}

static int parse_args(struct cmd_rsautl_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",    no_argument,       NULL, 'h'},
		{"in",      required_argument, NULL, 'i'},
		{"out",     required_argument, NULL, 'o'},
		{"pubin",   no_argument,       NULL, 'p'},
		{"encrypt", no_argument,       NULL, 'e'},
		{"decrypt", no_argument,       NULL, 'd'},
		{"hexdump", no_argument,       NULL, 'H'},
		{"passin",  required_argument, NULL, 'x'},
		{"keyform", required_argument, NULL, 'K'},
		{"inkey",   required_argument, NULL, 'k'},
		{"sign",    no_argument,       NULL, 's'},
		{"verify",  no_argument,       NULL, 'v'},
		{"pkcs",    no_argument,       NULL, 'P'},
		{"oaep",    no_argument,       NULL, 'O'},
		{"raw",     no_argument,       NULL, 'R'},
		{NULL,      0,                 NULL,  0 },
	};
	int c;
	opterr = 1;
	while ((c = getopt_long_only(argc, argv, "", opts, NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				usage();
				return 1;
			case 'i':
				if (!cmd_handle_in("rsautl", optarg,
				                   &data->fpin))
					return 0;
				break;
			case 'o':
				if (!cmd_handle_out("rsautl", optarg,
				                    &data->fpout))
					return 0;
				break;
			case 'p':
				data->pubin = 1;
				break;
			case 'e':
				data->mode = RSAUTL_MODE_ENCRYPT;
				break;
			case 'd':
				data->mode = RSAUTL_MODE_DECRYPT;
				break;
			case 'H':
				data->hexdump = 1;
				break;
			case 'x':
				if (!cmd_handle_pass("rsautl", optarg,
				                     &data->passin))
					return 0;
				break;
			case 'K':
				if (!cmd_handle_format("rsautl", optarg,
				                       &data->keyform))
					return 0;
				break;
			case 'k':
				if (!cmd_handle_in("rsautl", optarg,
				                   &data->fpkey))
					return 0;
				break;
			case 's':
				data->mode = RSAUTL_MODE_SIGN;
				break;
			case 'v':
				data->mode = RSAUTL_MODE_VERIFY;
				break;
			case 'P':
				data->padding = RSA_PKCS1_PADDING;
				break;
			case 'O':
				data->padding = RSA_PKCS1_OAEP_PADDING;
				break;
			case 'R':
				data->padding = RSA_NO_PADDING;
				break;
			default:
				usage();
				return 0;
		}
	}
	return 1;
}

static int do_op(struct cmd_rsautl_data *data)
{
	switch (data->mode)
	{
		case RSAUTL_MODE_ENCRYPT:
		{
			int ret = rsa_public_encrypt(data->inlen, data->indata,
			                             data->outdata, data->rsa,
			                             data->padding);
			if (ret == -1)
				return 0;
			data->outlen = ret;
			return 1;
		}
		case RSAUTL_MODE_DECRYPT:
		{
			int ret = rsa_private_decrypt(data->inlen, data->indata,
			                              data->outdata, data->rsa,
			                              data->padding);
			if (ret == -1)
				return 0;
			data->outlen = ret;
			return 1;
		}
		case RSAUTL_MODE_SIGN:
		{
			int ret = rsa_private_encrypt(data->inlen, data->indata,
			                              data->outdata, data->rsa,
			                              data->padding);
			if (ret == -1)
				return 0;
			data->outlen = ret;
			return 1;
		}
		case RSAUTL_MODE_VERIFY:
		{
			int ret = rsa_public_decrypt(data->inlen, data->indata,
			                             data->outdata, data->rsa,
			                             data->padding);
			if (ret == -1)
				return 0;
			data->outlen = ret;
			return 1;
		}
		default:
			return 0;
	}
}

int cmd_rsautl(int argc, char **argv)
{
	struct cmd_rsautl_data data;
	int ret = EXIT_FAILURE;
	ssize_t tmp;

	memset(&data, 0, sizeof(data));
	data.keyform = CMD_FORMAT_PEM;
	data.fpin = stdin;
	data.fpout = stdout;
	data.mode = RSAUTL_MODE_DECRYPT;
	data.padding = RSA_PKCS1_PADDING;
	data.rsa = rsa_new();
	if (!data.rsa)
	{
		fprintf(stderr, "rsautl: malloc: %s\n", strerror(errno));
		goto end;
	}
	if (!parse_args(&data, argc, argv))
		goto end;
	if (!data.fpkey)
	{
		fprintf(stderr, "rsautl: no key file given\n");
		goto end;
	}
	if (data.pubin && data.mode == RSAUTL_MODE_DECRYPT)
	{
		fprintf(stderr, "rsautl: RSA decrypt mode require private key\n");
		goto end;
	}
	if (data.pubin && data.mode == RSAUTL_MODE_SIGN)
	{
		fprintf(stderr, "rsautl: RSA sign mode require private key\n");
		goto end;
	}
	if ((data.mode == RSAUTL_MODE_SIGN || data.mode == RSAUTL_MODE_VERIFY)
	 && (data.padding == RSA_PKCS1_OAEP_PADDING))
	{
		fprintf(stderr, "rsautl OAEP padding isn't available for sign or verify\n");
		goto end;
	}
	if (!do_readkey(&data))
	{
		fprintf(stderr, "rsautl: invalid key\n");
		goto end;
	}
	if (!do_read(&data))
		goto end;
	tmp = rsa_size(data.rsa);
	if (tmp == -1)
	{
		fprintf(stderr, "rsautl: invalid RSA ctx\n");
		goto end;
	}
	data.outlen = tmp;
	data.outdata = malloc(data.outlen);
	if (!data.outdata)
	{
		fprintf(stderr, "rsautl: malloc: %s\n", strerror(errno));
		goto end;
	}
	memset(data.outdata, 0, data.outlen);
	if (!do_op(&data))
	{
		fprintf(stderr, "rsautl: rsa operation failed\n");
		goto end;
	}
	if (!do_write(&data))
		goto end;
	ret = EXIT_SUCCESS;

end:
	if (data.fpin && data.fpin != stdin)
		fclose(data.fpin);
	if (data.fpout && data.fpout != stdout)
		fclose(data.fpout);
	if (data.fpkey)
		fclose(data.fpkey);
	free(data.indata);
	free(data.outdata);
	free(data.passin);
	rsa_free(data.rsa);
	return ret;
}
