#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <png.h>

static void
dither(uint8_t *out, const uint8_t *in, uint32_t width, uint32_t height)
{
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			uint32_t index = 4 * (y * width + x);
			const uint8_t *pixel_in = &in[index];
			uint8_t *pixel_out = &out[index];
			for (size_t i = 0; i < 3; ++i)
			{
				int32_t oldv = pixel_out[i] + pixel_in[i];
				int32_t newv = oldv / 64 * 64;
				int32_t error = oldv - newv;
				pixel_out[i] = newv;
				if (x < width - 1)
					pixel_out[i + 4] += error * 7 / 16;
				if (x && y < height - 1)
					pixel_out[i + 4 * (width - 1)] += error * 3 / 16;
				if (y < height - 1)
					pixel_out[i + 4 * width] += error * 5 / 16;
				if (x < width - 1 && y < height - 1)
					pixel_out[i + 4 * (width + 1)] += error * 1 / 16;
			}
			pixel_out[3] = pixel_in[3];
		}
	}
}

static int
png_read(const char *file, uint8_t **data, uint32_t *width, uint32_t *height)
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

static void
png_write(const char *file,
          const void *data,
          uint32_t width,
          uint32_t height)
{
	png_bytep row_pointers[height];
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	FILE *fp = NULL;

	fp = fopen(file, "wb");
	if (!fp)
		goto error1;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		goto error2;
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		goto error3;
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	png_init_io(png_ptr, fp);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	png_set_IHDR(png_ptr,
	             info_ptr,
	             width,
	             height,
	             8,
	             PNG_COLOR_TYPE_RGBA,
	             PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT,
	             PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	for (uint32_t i = 0; i < height; ++i)
		row_pointers[i] = (uint8_t*)data + i * width * 4;
	png_write_image(png_ptr, row_pointers);
	if (setjmp(png_jmpbuf(png_ptr)))
		goto error3;
	png_write_end(png_ptr, NULL);
	fclose(fp);
	return;

error3:
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
error2:
	fclose(fp);
error1:
	return;
}

int
main(int argc, char **argv)
{
	uint8_t *dithered;
	uint8_t *data;
	uint32_t width;
	uint32_t height;

	if (argc != 2)
	{
		fprintf(stderr, "%s file.png\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (png_read(argv[1], &data, &width, &height))
	{
		fprintf(stderr, "%s: failed to read file\n", argv[0]);
		return EXIT_FAILURE;
	}
	dithered = calloc(4, width * height);
	if (!dithered)
	{
		fprintf(stderr, "%s: dithered allocation failed\n", argv[0]);
		return EXIT_FAILURE;
	}
	dither(dithered, data, width, height);
	png_write("out.png", dithered, width, height);
	return EXIT_SUCCESS;
}
