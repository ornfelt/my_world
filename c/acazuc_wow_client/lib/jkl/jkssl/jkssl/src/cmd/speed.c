#include "cmd/cmd.h"

#include <jkssl/rand.h>
#include <jkssl/evp.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

struct cmd_speed_data
{
	const struct evp_md *evp_md;
	const struct evp_cipher *evp_cipher;
};

static int ended;

#ifdef _WIN32
static void alarm_handler(HWND p1, UINT p2, UINT_PTR p3, DWORD p4)
{
	(void)p1;
	(void)p2;
	(void)p3;
	(void)p4;
	ended = 1;
}

static void setup_alarm(void)
{
	SetTimer(NULL, 0, 3000, alarm_handler);
}
#else
static void alarm_handler(int sig)
{
	(void)sig;
	ended = 1;
}

static void setup_alarm(void)
{
	signal(SIGALRM, alarm_handler);
	alarm(3);
}
#endif

static inline void timespec_diff(struct timespec *d, const struct timespec *a,
                                 const struct timespec *b)
{
	d->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_nsec >= b->tv_nsec)
	{
		d->tv_nsec = a->tv_nsec - b->tv_nsec;
	}
	else
	{
		d->tv_nsec = 1000000000 - (b->tv_nsec - a->tv_nsec);
		d->tv_sec--;
	}
}

static int do_digest(const struct evp_md *evp_md)
{
	static const uint32_t block_sizes[] = {16, 64, 256, 1024, 8192, 16384};
	struct evp_md_ctx *evp_md_ctx;
	uint8_t buffer[16384];
	struct timespec start;
	struct timespec end;
	struct timespec diff;
	uint8_t dgst[512];
	int ret = 0;

	evp_md_ctx = evp_md_ctx_new();
	if (!evp_md_ctx)
		goto end;
	if (!evp_digest_init(evp_md_ctx, evp_md))
		goto end;
	for (size_t i = 0; i < sizeof(block_sizes) / sizeof(*block_sizes); ++i)
	{
		printf("Doing %s for 3s on %" PRIu32 " size blocks: ",
		       evp_md_get0_name(evp_md), block_sizes[i]);
		fflush(stdout);
		uint64_t n = 0;
		ended = 0;
		clock_gettime(CLOCK_MONOTONIC, &start);
		setup_alarm();
		while (!ended)
		{
			if (!evp_md_ctx_reset(evp_md_ctx)
			 || !evp_digest_update(evp_md_ctx, buffer, block_sizes[i])
			 || !evp_digest_final(evp_md_ctx, dgst))
			{
				fprintf(stderr, "speed: digest compute failed\n");
				goto end;
			}
			n++;
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		timespec_diff(&diff, &end, &start);
		printf("%" PRIu64 " %s's in %d.%02d seconds\n",
		       n, evp_md_get0_name(evp_md), (int)diff.tv_sec,
		       (int)(diff.tv_nsec / 10000000));
	}
	ret = 1;

end:
	evp_md_ctx_free(evp_md_ctx);
	return ret;
}

static int do_cipher(const struct evp_cipher *evp_cipher)
{
	static const uint32_t block_sizes[] = {16, 64, 256, 1024, 8192, 16384};
	struct evp_cipher_ctx *evp_cipher_ctx;
	uint8_t buffer[16384];
	struct timespec start;
	struct timespec end;
	struct timespec diff;
	uint8_t key[512];
	int ret = 0;

	rand_bytes(key, sizeof(key));
	evp_cipher_ctx = evp_cipher_ctx_new();
	if (!evp_cipher_ctx)
		goto end;
	if (!evp_cipher_init(evp_cipher_ctx, evp_cipher, key, key, 1))
		goto end;
	for (size_t i = 0; i < sizeof(block_sizes) / sizeof(*block_sizes); ++i)
	{
		printf("Doing %s for 3s on %" PRIu32 " size blocks: ",
		       evp_cipher_get0_name(evp_cipher), block_sizes[i]);
		fflush(stdout);
		uint64_t n = 0;
		ended = 0;
		clock_gettime(CLOCK_MONOTONIC, &start);
		setup_alarm();
		while (!ended)
		{
			size_t outl = block_sizes[i];
			if (!evp_cipher_update(evp_cipher_ctx, buffer, &outl,
			                       buffer, outl))
			{
				fprintf(stderr, "speed: cipher compute failed\n");
				goto end;
			}
			n++;
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		timespec_diff(&diff, &end, &start);
		printf("%" PRIu64 " %s's in %d.%02d seconds\n",
		       n, evp_cipher_get0_name(evp_cipher), (int)diff.tv_sec,
		       (int)(diff.tv_nsec / 10000000));
	}
	ret = 1;

end:
	evp_cipher_ctx_free(evp_cipher_ctx);
	return ret;
}

static void usage(void)
{
	printf("speed [options]\n");
	printf("-help:    display this help\n");
	printf("-evp evp: test the given evp\n");
}

static int parse_args(struct cmd_speed_data *data, int argc, char **argv)
{
	static const struct option opts[] =
	{
		{"help", no_argument,       NULL, 'h'},
		{"evp",  required_argument, NULL, 'e'},
		{NULL,   0,                 NULL,  0 },
	};
	int c;
	while ((c = getopt_long_only(argc, argv, "", opts, NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				usage();
				return 1;
			case 'e':
				if (data->evp_md || data->evp_cipher)
				{
					fprintf(stderr, "speed: only one evp\n");
					return 0;
				}
				data->evp_md = evp_get_digestbyname(optarg);
				if (data->evp_md)
					break;
				data->evp_cipher = evp_get_cipherbyname(optarg);
				if (data->evp_cipher)
					break;
				fprintf(stderr, "speed: unknown evp\n");
				return 0;
			default:
				usage();
				return 0;
		}
	}
	return 1;
}

int cmd_speed(int argc, char **argv)
{
	struct cmd_speed_data data;
	int ret = EXIT_FAILURE;

	memset(&data, 0, sizeof(data));
	if (!parse_args(&data, argc, argv))
		goto end;
	if (data.evp_md)
	{
		if (!do_digest(data.evp_md))
			goto end;
	}
	if (data.evp_cipher)
	{
		if (!do_cipher(data.evp_cipher))
			goto end;
	}
	ret = EXIT_SUCCESS;

end:
	return ret;
}
