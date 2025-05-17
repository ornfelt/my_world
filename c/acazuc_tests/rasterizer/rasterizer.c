#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <png.h>

struct rast_ctx
{
	int32_t width;
	int32_t height;
	uint8_t *data;
};

struct vert
{
	int32_t x;
	int32_t y;
};

struct triangle
{
	struct vert *vert[3];
};

static int32_t
mini(int32_t a, int32_t b)
{
	return a < b ? a : b;
}

static int32_t
maxi(int32_t a, int32_t b)
{
	return a > b ? a : b;
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

static void
render_block(struct rast_ctx *ctx,
             struct triangle *triangle,
             int64_t barycentric[4][4],
             int64_t area,
             int64_t x,
             int64_t y)
{
	uint32_t *data = (uint32_t*)&ctx->data[(y * ctx->width + x) * 4];
	uint32_t offsets[4] = {0, 1, ctx->width, ctx->width + 1};
	for (size_t i = 0; i < 4; ++i)
	{
		if ((barycentric[0][i] | barycentric[1][i] | barycentric[2][i]) < 0)
			continue;
		data[offsets[i]] = 0xFF000000
		                 | (((barycentric[0][i] * 0xFF) / area) << 0)
		                 | (((barycentric[1][i] * 0xFF) / area) << 8)
		                 | (((barycentric[2][i] * 0xFF) / area) << 16);
	}
	printf("%ld %ld %ld %ld %ld\n",
	       barycentric[0][0] + barycentric[1][0] + barycentric[2][0],
	       barycentric[0][1] + barycentric[1][1] + barycentric[2][1],
	       barycentric[0][2] + barycentric[1][2] + barycentric[2][2],
	       barycentric[0][3] + barycentric[1][3] + barycentric[2][3],
	       area);
}

__attribute__((noinline))
static void
draw_triangle(struct rast_ctx *ctx, struct triangle *triangle)
{
	int64_t row_barycentric[3][4];
	int64_t barycentric[3][4];
	int64_t edge_dx[3];
	int64_t edge_dy[3];
	int64_t area;
	int64_t min_x;
	int64_t max_x;
	int64_t min_y;
	int64_t max_y;
	int64_t vx[3];
	int64_t vy[3];

	for (size_t i = 0; i < 3; ++i)
	{
		vx[i] = triangle->vert[i]->x << 8;
		vy[i] = triangle->vert[i]->y << 8;
	}
	min_x = mini(vx[0], mini(vx[1], vx[2]));
	max_x = maxi(vx[0], maxi(vx[1], vx[2]));
	min_y = mini(vy[0], mini(vy[1], vy[2]));
	max_y = maxi(vy[0], maxi(vy[1], vy[2]));
	edge_dy[0] = vy[1] - vy[2];
	edge_dy[1] = vy[2] - vy[0];
	edge_dy[2] = vy[0] - vy[1];
	edge_dx[0] = vx[2] - vx[1];
	edge_dx[1] = vx[0] - vx[2];
	edge_dx[2] = vx[1] - vx[0];
	barycentric[0][0] = (min_x - vx[1]) * edge_dy[0]
	                  + (min_y - vy[1]) * edge_dx[0];
	barycentric[1][0] = (min_x - vx[2]) * edge_dy[1]
	                  + (min_y - vy[2]) * edge_dx[1];
	barycentric[2][0] = (min_x - vx[0]) * edge_dy[2]
	                  + (min_y - vy[0]) * edge_dx[2];
	barycentric[0][1] = barycentric[0][0] + edge_dy[0];
	barycentric[0][2] = barycentric[0][0] + edge_dx[0];
	barycentric[0][3] = barycentric[0][2] + edge_dy[0];
	barycentric[1][1] = barycentric[1][0] + edge_dy[1];
	barycentric[1][2] = barycentric[1][0] + edge_dx[1];
	barycentric[1][3] = barycentric[1][2] + edge_dy[1];
	barycentric[2][1] = barycentric[2][0] + edge_dy[2];
	barycentric[2][2] = barycentric[2][0] + edge_dx[2];
	barycentric[2][3] = barycentric[2][2] + edge_dy[2];
	area = edge_dy[2] * edge_dx[0] - edge_dx[2] * edge_dy[0];
	edge_dx[0] *= 2 << 8;
	edge_dx[1] *= 2 << 8;
	edge_dx[2] *= 2 << 8;
	edge_dy[0] *= 2 << 8;
	edge_dy[1] *= 2 << 8;
	edge_dy[2] *= 2 << 8;
	min_x /= 0x100;
	max_x /= 0x100;
	min_y /= 0x100;
	max_y /= 0x100;
	for (int64_t y = min_y; y <= max_y; y += 2)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			row_barycentric[0][i] = barycentric[0][i];
			row_barycentric[1][i] = barycentric[1][i];
			row_barycentric[2][i] = barycentric[2][i];
		}
		for (int64_t x = min_x; x <= max_x; x += 2)
		{
			render_block(ctx, triangle, row_barycentric, area, x, y);
			for (size_t i = 0; i < 4; ++i)
			{
				row_barycentric[0][i] += edge_dy[0];
				row_barycentric[1][i] += edge_dy[1];
				row_barycentric[2][i] += edge_dy[2];
			}
		}
		for (size_t i = 0; i < 4; ++i)
		{
			barycentric[0][i] += edge_dx[0];
			barycentric[1][i] += edge_dx[1];
			barycentric[2][i] += edge_dx[2];
		}
	}
}

int
main(int argc, char **argv)
{
	struct rast_ctx ctx;
	struct vert v[3];
	struct triangle triangle;

	(void)argc;
	ctx.width = 640;
	ctx.height = 480;
	ctx.data = calloc(ctx.width * ctx.height * 4, 1);
	if (!ctx.data)
	{
		fprintf(stderr, "%s: malloc: %s\n", argv[0], strerror(errno));
		return EXIT_FAILURE;
	}
	v[0].x = ctx.width / 2;
	v[0].y = 10;
	v[1].x = ctx.width - 10;
	v[1].y = ctx.height - 10;
	v[2].x = 10;
	v[2].y = ctx.height - 10;
	triangle.vert[0] = &v[0];
	triangle.vert[1] = &v[1];
	triangle.vert[2] = &v[2];
	draw_triangle(&ctx, &triangle);
	png_write("out.png", ctx.data, ctx.width, ctx.height);
	return EXIT_SUCCESS;
}
