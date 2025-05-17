#include <libjpeg/jpeg.h>

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

struct env
{
	const char *progname;
	long quality;
	int subsampling;
	long restart_interval;
	const char *output;
};

static int
read_file(struct env *env,
          const char *filename,
          uint8_t **data,
          uint32_t *width,
          uint32_t *height,
          uint8_t *components)
{
	FILE *fp = NULL;
	struct jpeg *jpeg = NULL;
	int ret = 1;

	fp = fopen(filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: fopen(%s): %s\n",
		        env->progname,
		        filename,
		        strerror(errno));
		goto end;
	}
	jpeg = jpeg_new();
	if (!jpeg)
	{
		fprintf(stderr, "%s: jpeg_new failed\n", env->progname);
		goto end;
	}
	jpeg_init_io(jpeg, fp);
	if (jpeg_read_headers(jpeg))
	{
		fprintf(stderr, "%s: jpeg_read_headers: %s\n",
		        env->progname,
		        jpeg_get_err(jpeg));
		goto end;
	}
	jpeg_get_info(jpeg, width, height, components);
	*data = malloc(*width * *height * *components);
	if (!*data)
	{
		fprintf(stderr, "%s: malloc: %s\n",
		        env->progname,
		        strerror(errno));
		goto end;
	}
	if (jpeg_read_data(jpeg, *data))
	{
		fprintf(stderr, "%s: jpeg_read_data: %s\n",
		        env->progname,
		        jpeg_get_err(jpeg));
		goto end;
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
	jpeg_free(jpeg);
	return ret;
}

static int
write_file(struct env *env,
           const char *filename,
           const uint8_t *data,
           uint32_t width,
           uint32_t height,
           uint8_t components,
           int quality,
           int subsampling,
           int restart_interval)
{
	FILE *fp = NULL;
	struct jpeg *jpeg = NULL;
	int ret = 1;

	fp = fopen(filename, "wr");
	if (!fp)
	{
		fprintf(stderr, "%s: fopen(%s): %s\n",
		        env->progname,
		        filename,
		        strerror(errno));
		goto end;
	}
	jpeg = jpeg_new();
	if (!jpeg)
	{
		fprintf(stderr, "%s: jpeg_new failed\n", env->progname);
		goto end;
	}
	jpeg_init_io(jpeg, fp);
	jpeg_set_quality(jpeg, quality);
	jpeg_set_restart_interval(jpeg, restart_interval);
	if (jpeg_set_subsampling(jpeg, subsampling))
	{
		fprintf(stderr, "%s: jpeg_set_subsampling: %s\n",
		        env->progname,
		        jpeg_get_err(jpeg));
		goto end;
	}
	if (jpeg_set_info(jpeg, width, height, components))
	{
		fprintf(stderr, "%s: jpeg_set_info: %s\n",
		        env->progname,
		        jpeg_get_err(jpeg));
		goto end;
	}
	if (jpeg_write_headers(jpeg))
	{
		fprintf(stderr, "%s: jpeg_write_headers: %s\n",
		        env->progname,
		        jpeg_get_err(jpeg));
		goto end;
	}
	if (jpeg_write_data(jpeg, data))
	{
		fprintf(stderr, "%s: jpeg_write_data: %s\n",
		        env->progname,
		        jpeg_get_err(jpeg));
		goto end;
	}
	ret = 0;

end:
	if (fp)
		fclose(fp);
	jpeg_free(jpeg);
	return ret;
}

static int
recode_file(struct env *env, const char *filename)
{
	uint8_t *data = NULL;
	uint32_t width;
	uint32_t height;
	uint8_t components;
	int ret = 1;

	if (read_file(env, filename, &data, &width, &height, &components))
		goto end;
	if (write_file(env, env->output, data, width, height, components,
	               env->quality, env->subsampling, env->restart_interval))
		goto end;
	ret = 0;

end:
	free(data);
	return ret;
}

static void
usage(const char *progname)
{
	printf("%s [-h] [-q quality] [-s subsampling] [-r interval] -o output FILE\n",
	       progname);
	printf("-h            : show this help\n");
	printf("-q quality    : set the output quality (1 to 100, default to 100)\n");
	printf("-s subsampling: set the output chroma subsampling:\n");
	printf("              : 4:4:4 for full sampling (default)\n");
	printf("              : 4:4:0 for halved vertical sampling\n");
	printf("              : 4:2:2 for halved horizontal sampling\n");
	printf("              : 4:2:0 for halved vertcal & horizontal sampling\n");
	printf("              : 4:1:1 for quartered horizontal sampling\n");
	printf("              : 4:1:0 for quartered horizontal & halved vertical sampling\n");
	printf("-r interval   : set the restart interval (default to 0)\n");
	printf("-o output     : set the output file\n");
}

int
main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.subsampling = JPEG_SUBSAMPLING_444;
	env.quality = 100;
	env.restart_interval = 0;
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hq:s:r:o:")) != -1)
	{
		switch (c)
		{
			case 'q':
			{
				errno = 0;
				char *endptr;
				env.quality = strtol(optarg, &endptr, 0);
				if (errno || *endptr)
				{
					fprintf(stderr, "%s: invalid quality\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 's':
				if (!strcmp(optarg, "4:4:4"))
				{
					env.subsampling = JPEG_SUBSAMPLING_444;
				}
				else if (!strcmp(optarg, "4:4:0"))
				{
					env.subsampling = JPEG_SUBSAMPLING_440;
				}
				else if (!strcmp(optarg, "4:2:2"))
				{
					env.subsampling = JPEG_SUBSAMPLING_422;
				}
				else if (!strcmp(optarg, "4:2:0"))
				{
					env.subsampling = JPEG_SUBSAMPLING_420;
				}
				else if (!strcmp(optarg, "4:1:1"))
				{
					env.subsampling = JPEG_SUBSAMPLING_411;
				}
				else if (!strcmp(optarg, "4:1:0"))
				{
					env.subsampling = JPEG_SUBSAMPLING_410;
				}
				else
				{
					fprintf(stderr, "%s: invalid subsampling\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			case 'r':
			{
				errno = 0;
				char *endptr;
				env.restart_interval = strtol(optarg, &endptr, 0);
				if (errno || *endptr
				 || env.restart_interval < 0
				 || env.restart_interval > UINT16_MAX)
				{
					fprintf(stderr, "%s: invalid interval\n",
					        argv[0]);
					return EXIT_FAILURE;
				}
				break;
			}
			case 'o':
				env.output = optarg;
				break;
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if (optind == argc)
	{
		fprintf(stderr, "%s: missing operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (optind + 1 < argc)
	{
		fprintf(stderr, "%s: extra operand\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!env.output)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	if (recode_file(&env, argv[optind]))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
