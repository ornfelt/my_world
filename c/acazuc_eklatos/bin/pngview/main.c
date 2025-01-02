#include <libpng/png.h>

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define OPT_c (1 << 0)

struct env
{
	const char *progname;
	int opt;
};

static void display_image_colored(uint32_t width, uint32_t height,
                                  const uint8_t *data)
{
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			printf("\033[48;2;%" PRIu8 ";%" PRIu8 ";%" PRIu8 "m ",
			       data[0] * data[3] / 255,
			       data[1] * data[3] / 255,
			       data[2] * data[3] / 255);
			data += 4;
		}
		printf("\n");
	}
	printf("\033[0m");
}

static void display_image_ascii(uint32_t width, uint32_t height,
                                const uint8_t *data)
{
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			if ((data[3] & 0xFF) >= 0x80)
			{
				static const char chars[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";
				uint8_t lumi = (((data[0] & 0xFF) * 76 / 255)
				              + ((data[1] & 0xFF) * 150 / 255)
				              + ((data[2] & 0xFF)) * 29 / 255);
				putchar(chars[(uint32_t)(255 - lumi) * 68 / 255]);
			}
			else
			{
				printf(" ");
			}
			data += 4;
		}
		printf("\n");
	}
}

static int load_png(const char *file, uint8_t **data, uint32_t *width,
                    uint32_t *height)
{
	int ret = 1;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_infop end_info = NULL;
	png_bytep *row_pointers = NULL;
	FILE *fp = NULL;
	int row_bytes;
	fp = fopen(file, "rb");
	if (!fp)
		goto end;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		goto end;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		goto end;
	end_info = png_create_info_struct(png_ptr);
	if (!end_info)
		goto end;
	if (setjmp(png_jmpbuf(png_ptr)))
		goto end;
	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, width, height, NULL, NULL, NULL, NULL, NULL);
	png_read_update_info(png_ptr, info_ptr);
	row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	*data = malloc(row_bytes * *height);
	if (!*data)
		goto end;
	row_pointers = malloc(sizeof(png_bytep) * *height);
	if (!row_pointers)
		goto end;
	for (uint32_t i = 0; i < *height; ++i)
		row_pointers[i] = &(*data)[i * row_bytes];
	png_read_image(png_ptr, row_pointers);
	ret = 0;

end:
	free(row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	if (fp)
		fclose(fp);
	return ret;

}

static int print_png(struct env *env, const char *file)
{
	int ret = 1;
	uint32_t width;
	uint32_t height;
	uint8_t *data = NULL;
	if (load_png(file, &data, &width, &height))
		goto end;
	if (env->opt & OPT_c)
		display_image_colored(width, height, data);
	else
		display_image_ascii(width, height, data);
	free(data);
	ret = 0;

end:
	return ret;
}

static void usage(const char *progname)
{
	printf("%s [-h] [-c] FILES\n", progname);
	printf("-h: display this help\n");
	printf("-c: display colored\n");
}

int main(int argc, char **argv)
{
	struct env env;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "hc")) != -1)
	{
		switch (c)
		{
			case 'c':
				env.opt |= OPT_c;
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
	for (int i = optind; i < argc; ++i)
	{
		if (print_png(&env, argv[i]))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
