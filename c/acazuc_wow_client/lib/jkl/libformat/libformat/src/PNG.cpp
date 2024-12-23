#include "PNG.h"
#include <csetjmp>
#include <cstdio>
#include <png.h>

namespace libformat
{

	bool PNG::read(std::string filename, char *&data, uint32_t &width, uint32_t &height)
	{
		png_structp png_ptr = NULL;
		png_infop info_ptr = NULL;
		png_infop end_info = NULL;
		png_bytep *row_pointers = NULL;
		png_byte header[8];
		png_byte *image_data = NULL;
		FILE *fp = NULL;
		int rowbytes;
		if (!(fp = std::fopen(filename.c_str(), "rb")))
			goto error1;
		if (std::fread(header, 1, 8, fp) != 8)
			goto error2;
		if (png_sig_cmp(header, 0, 8))
			goto error2;
		if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
			goto error2;
		if (!(info_ptr = png_create_info_struct(png_ptr)))
			goto error3;
		if (!(end_info = png_create_info_struct(png_ptr)))
			goto error3;
		if (setjmp(png_jmpbuf(png_ptr)))
			goto error3;
		png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, 8);
		png_read_info(png_ptr, info_ptr);
		int bit_depth, color_type;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
		png_set_palette_to_rgb(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
		rowbytes = png_get_rowbytes(png_ptr, info_ptr);
		image_data = new png_byte[rowbytes * height];
		row_pointers = new png_bytep[height];
		for (uint32_t i = 0; i < height; ++i)
			row_pointers[i] = image_data + i * rowbytes;
		png_read_image(png_ptr, row_pointers);
		data = reinterpret_cast<char*>(image_data);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		delete[] (row_pointers);
		std::fclose(fp);
		return true;
	error3:
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	error2:
		std::fclose(fp);
	error1:
		return false;
	}

	bool PNG::write(std::string file, char *data, uint32_t width, uint32_t height)
	{
		png_bytep row_pointers[height];
		png_structp png_ptr = NULL;
		png_infop info_ptr = NULL;
		FILE *fp = NULL;
		if (!(fp = std::fopen(file.c_str(), "wb")))
			goto error1;
		if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
			goto error2;
		if (!(info_ptr = png_create_info_struct(png_ptr)))
			goto error3;
		if (setjmp(png_jmpbuf(png_ptr)))
			goto error3;
		png_init_io(png_ptr, fp);
		if (setjmp(png_jmpbuf(png_ptr)))
			goto error3;
		png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		if (setjmp(png_jmpbuf(png_ptr)))
			goto error3;
		for (uint32_t i = 0; i < height; ++i)
			row_pointers[i] = reinterpret_cast<unsigned char*>(data + i * width * 4);
		png_write_image(png_ptr, row_pointers);
		if (setjmp(png_jmpbuf(png_ptr)))
			goto error3;
		png_write_end(png_ptr, NULL);
		std::fclose(fp);
		return true;
	error3:
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
	error2:
		fclose(fp);
	error1:
		return false;
	}

}
