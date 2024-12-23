#include "utils/utils.h"
#include "cmd/cmd.h"

#include <jkssl/hmac.h>
#include <jkssl/pem.h>
#include <jkssl/evp.h>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#define DGST_FMT_BASIC     0
#define DGST_FMT_REVERSE   1
#define DGST_FMT_SEMICOLON 2
#define DGST_FMT_BINARY    3

struct cmd_dgst_data
{
	const struct evp_md *evp_md;
	const char *hmac_pass;
	FILE *out;
	int format;
	int written;
	int list;
	struct evp_pkey *sign;
	struct evp_pkey *verify;
	FILE *signature;
};

static int create_md_digest(struct cmd_dgst_data *data, FILE *fp, uint8_t *md)
{
	struct evp_md_ctx *evp_md_ctx;
	uint8_t buf[4096];
	size_t rd;
	int ret = 0;

	evp_md_ctx = evp_md_ctx_new();
	if (!evp_md_ctx)
		goto end;
	if (!evp_digest_init(evp_md_ctx, data->evp_md))
		goto end;
	while ((rd = fread(buf, 1, 4096, fp)) > 0)
	{
		if (ferror(fp))
		{
			fprintf(stderr, "dgst: read: %s\n", strerror(errno));
			goto end;
		}
		if (!evp_digest_update(evp_md_ctx, buf, rd))
			goto end;
	}
	if (ferror(fp))
	{
		fprintf(stderr, "dgst: read: %s\n", strerror(errno));
		goto end;
	}
	if (!evp_digest_final(evp_md_ctx, md))
		goto end;
	ret = 1;

end:
	evp_md_ctx_free(evp_md_ctx);
	return ret;
}

static int create_md_hmac(struct cmd_dgst_data *data, FILE *fp, uint8_t *md)
{
	struct hmac_ctx *hmac_ctx;
	uint8_t buf[4096];
	size_t rd;
	int ret = 0;

	hmac_ctx = hmac_ctx_new();
	if (!hmac_ctx)
		goto end;
	if (!hmac_init(hmac_ctx, data->hmac_pass, strlen(data->hmac_pass),
	               data->evp_md))
		goto end;
	while ((rd = fread(buf, 1, 4096, fp)) > 0)
	{
		if (ferror(fp))
		{
			fprintf(stderr, "dgst: read: %s\n", strerror(errno));
			goto end;
		}
		if (!hmac_update(hmac_ctx, buf, rd))
			goto end;
	}
	if (ferror(fp))
	{
		fprintf(stderr, "dgst: read: %s\n", strerror(errno));
		goto end;
	}
	if (!hmac_final(hmac_ctx, md, NULL))
		goto end;
	ret = 1;

end:
	hmac_ctx_free(hmac_ctx);
	return ret;
}

static int do_sign(struct cmd_dgst_data *data, const uint8_t *digest)
{
	struct evp_pkey_ctx *ctx = NULL;
	size_t siglen;
	uint8_t *sig = NULL;
	int ret = 0;

	ctx = evp_pkey_ctx_new(data->sign);
	if (!ctx)
	{
		fprintf(stderr, "dgst: sign ctx creation failed\n");
		goto end;
	}
	if (!evp_pkey_ctx_set_signature_md(ctx, data->evp_md))
	{
		fprintf(stderr, "dgst: failed to set signature md\n");
		goto end;
	}
	if (evp_pkey_sign(ctx, NULL, &siglen, NULL, 0) <= 0)
	{
		fprintf(stderr, "dgst: failed to get signature length\n");
		goto end;
	}
	sig = malloc(siglen);
	if (!sig)
	{
		fprintf(stderr, "dgst: malloc: %s\n", strerror(errno));
		return 0;
	}
	if (evp_pkey_sign(ctx, sig, &siglen, digest,
	                  evp_md_get_size(data->evp_md)) <= 0)
	{
		fprintf(stderr, "dgst: signature failed\n");
		goto end;
	}
	fwrite(sig, 1, siglen, data->out);
	ret = 1;

end:
	free(sig);
	evp_pkey_ctx_free(ctx);
	return ret;
}

static uint8_t *read_signature(FILE *fp, size_t *len)
{
	uint8_t *ret = NULL;
	size_t rd;
	*len = 0;

	if (!fp)
	{
		fprintf(stderr, "dgst: no signature given\n");
		goto err;
	}
	while (1)
	{
		uint8_t *tmp = realloc(ret, *len + 4096);
		if (!tmp)
		{
			fprintf(stderr, "dgst: malloc: %s\n", strerror(errno));
			goto err;
		}
		ret = tmp;
		rd = fread(&ret[*len], 1, 4096, fp);
		if (ferror(fp))
		{
			fprintf(stderr, "dgst: read: %s\n", strerror(errno));
			goto err;
		}
		*len += rd;
		if (feof(fp))
			break;
	}
	return ret;

err:
	free(ret);
	return NULL;
}

static int do_verify(struct cmd_dgst_data *data, const uint8_t *digest)
{
	struct evp_pkey_ctx *ctx = NULL;
	size_t siglen;
	uint8_t *sig = NULL;
	int ret = 0;

	sig = read_signature(data->signature, &siglen);
	if (!sig)
		goto end;
	ctx = evp_pkey_ctx_new(data->verify);
	if (!ctx)
	{
		fprintf(stderr, "dgst: verify ctx creation failed\n");
		goto end;
	}
	if (!evp_pkey_ctx_set_signature_md(ctx, data->evp_md))
	{
		fprintf(stderr, "dgst: failed to set signature md\n");
		goto end;
	}
	switch (evp_pkey_verify(ctx, sig, siglen, digest,
	                        evp_md_get_size(data->evp_md)))
	{
		default:
		case -1:
			fprintf(stderr, "dgst: verify failed\n");
			goto end;
		case 0:
			fprintf(stderr, "dgst: invalid signature\n");
			goto end;
		case 1:
			break;
	}
	ret = 1;

end:
	evp_pkey_ctx_free(ctx);
	return ret;
}

static int dgst_fp(struct cmd_dgst_data *data, FILE *fp, const char *fn)
{
	uint8_t digest[EVP_MAX_MD_SIZE];

	data->written = 1;
	if (data->hmac_pass)
	{
		if (!create_md_hmac(data, fp, digest))
			return 0;
	}
	else
	{
		if (!create_md_digest(data, fp, digest))
			return 0;
	}
	if (data->sign)
		return do_sign(data, digest);
	if (data->verify)
		return do_verify(data, digest);
	if (data->format == DGST_FMT_BINARY)
	{
		fwrite(digest, 1, evp_md_get_size(data->evp_md), data->out);
		return 1;
	}
	if (data->format == DGST_FMT_BASIC
	 || data->format == DGST_FMT_SEMICOLON)
	{
		if (data->hmac_pass)
			fprintf(data->out, "HMAC-");
		fprintf(data->out, "%s(%s)= ", evp_md_get0_name(data->evp_md), fn);
	}
	for (size_t i = 0; i < evp_md_get_size(data->evp_md); ++i)
	{
		fprintf(data->out, "%02x", digest[i]);
		if (data->format == DGST_FMT_SEMICOLON
		 && i != evp_md_get_size(data->evp_md) - 1u)
			fputc(':', data->out);
	}
	if (data->format == DGST_FMT_REVERSE)
		fprintf(data->out, " *%s", fn);
	fprintf(data->out, "\n");
	return 1;
}

static int dgst_files(struct cmd_dgst_data *data, const char *arg)
{
	FILE *fp;
	int ret;

	data->written = 1;
	fp = fopen(arg, "r");
	if (!fp)
	{
		fprintf(stderr, "dgst: open(%s): %s\n", arg, strerror(errno));
		return 0;
	}
	ret = dgst_fp(data, fp, arg);
	fclose(fp);
	return ret;
}

struct digest_list_state
{
	size_t n;
};

static int digest_foreach(const struct evp_md *md, void *data)
{
	struct digest_list_state *state = data;
	char name[64];
	snprintf(name, sizeof(name), "-%s", evp_md_get0_name(md));
	for (size_t i = 0; name[i]; ++i)
		name[i] = tolower(name[i]);
	printf("%s", name);
	if (state->n % 3 == 2)
	{
		printf("\n");
	}
	else
	{
		size_t len = 1 + strlen(evp_md_get0_name(md));
		if (len < 24)
			printf("%.*s", (int)(24 - len), "                        ");
	}
	state->n++;
	return 1;
}

static void list_digests(void)
{
	printf("Supported digests:\n");
	struct digest_list_state state;
	state.n = 0;
	evp_foreach_digest(digest_foreach, &state);
	if (state.n % 3 != 0)
		printf("\n");
}

static void usage(void)
{
	printf("dgst [options] [FILES]\n");
	printf("-help:          display this help\n");
	printf("-r:             reverse print\n");
	printf("-c:             print with semicolons\n");
	printf("-hmac pass:     create hmac with given pass\n");
	printf("-binary:        output as binary\n");
	printf("-sign file:     use given private key for signature\n");
	printf("-verify file:   use given public key for verification\n");
	printf("-prverify file: use given private key for verification\n");
	printf("-out file:      output to given file\n");
	printf("-*:             digest algorithm to use\n");
	printf("-list:          list available digests\n");
}

static int parse_args(struct cmd_dgst_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help",      no_argument,       NULL, 'h'},
		{"r",         no_argument,       NULL, 'r'},
		{"c",         no_argument,       NULL, 'c'},
		{"hmac",      required_argument, NULL, 'H'},
		{"list",      no_argument,       NULL, 'l'},
		{"binary",    no_argument,       NULL, 'b'},
		{"sign",      required_argument, NULL, 's'},
		{"verify",    required_argument, NULL, 'v'},
		{"prverify",  required_argument, NULL, 'V'},
		{"out",       required_argument, NULL, 'o'},
		{"signature", required_argument, NULL, 'S'},
		{NULL,        0,                 NULL,  0 },
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
			case 'r':
				data->format = DGST_FMT_REVERSE;
				break;
			case 'c':
				data->format = DGST_FMT_SEMICOLON;
				break;
			case 'H':
				data->hmac_pass = optarg;
				break;
			case 'l':
				data->list = 1;
				break;
			case 'b':
				data->format = DGST_FMT_BINARY;
				break;
			case 's':
			{
				if (data->sign || data->verify)
				{
					fprintf(stderr, "dgst: only one of -sign, -verify or -prverify can be used\n");
					return 0;
				}
				FILE *fp = fopen(optarg, "r");
				if (!fp)
				{
					fprintf(stderr, "dgst: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				data->sign = pem_read_private_key(fp, NULL, NULL);
				fclose(fp);
				if (!data->sign)
				{
					fprintf(stderr, "dgst: failed to read private key\n");
					return 0;
				}
				break;
			}
			case 'v':
			{
				if (data->sign || data->verify)
				{
					fprintf(stderr, "dgst: only one of -sign, -verify or -prverify can be used\n");
					return 0;
				}
				FILE *fp = fopen(optarg, "r");
				if (!fp)
				{
					fprintf(stderr, "dgst: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				data->verify = pem_read_pubkey(fp, NULL, NULL);
				fclose(fp);
				if (!data->verify)
				{
					fprintf(stderr, "dgst: failed to read pubkey\n");
					return 0;
				}
				break;
			}
			case 'V':
			{
				if (data->sign || data->verify)
				{
					fprintf(stderr, "dgst: only one of -sign, -verify or -prverify can be used\n");
					return 0;
				}
				FILE *fp = fopen(optarg, "r");
				if (!fp)
				{
					fprintf(stderr, "dgst: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				data->verify = pem_read_private_key(fp, NULL, NULL);
				fclose(fp);
				if (!data->verify)
				{
					fprintf(stderr, "dgst: failed to read private key\n");
					return 0;
				}
				break;
			}
			case 'o':
				if (data->out != stdout)
					fclose(data->out);
				data->out = fopen(optarg, "w");
				if (!data->out)
				{
					fprintf(stderr, "dgst: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			case 'S':
				if (data->signature)
					fclose(data->signature);
				data->signature = fopen(optarg, "r");
				if (!data->signature)
				{
					fprintf(stderr, "dgst: open(%s): %s\n",
					        optarg, strerror(errno));
					return 0;
				}
				break;
			default:
				data->evp_md = evp_get_digestbyname(argv[optind - 1] + 1);
				if (!data->evp_md)
				{
					fprintf(stderr, "dgst: unrecognized option: %s\n",
					        argv[optind - 1]);
					usage();
					return 0;
				}
				break;
		}
	}
	return 1;
}

int cmd_dgst(int argc, char **argv)
{
	struct cmd_dgst_data data;
	int err = 0;

	memset(&data, 0, sizeof(data));
	data.out = stdout;
	if (!parse_args(&data, argc, argv))
		return EXIT_FAILURE;
	if (data.list)
	{
		list_digests();
		goto end;
	}
	if (!data.evp_md)
	{
		fprintf(stderr, "dgst: no hash given\n");
		goto end;
	}
	for (int i = optind; i < argc; ++i)
		err |= !dgst_files(&data, argv[i]);
	if (!data.written)
		err = !dgst_fp(&data, stdin, "stdin");

end:
	if (data.out && data.out != stdout)
		fclose(data.out);
	if (data.signature)
		fclose(data.signature);
	evp_pkey_free(data.sign);
	evp_pkey_free(data.verify);
	return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
