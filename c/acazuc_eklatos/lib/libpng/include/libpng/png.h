#ifndef LIBPNG_PNG_H
#define LIBPNG_PNG_H

#include <stdint.h>
#include <setjmp.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PNG_LIBPNG_VER_STRING "1.0"

#define PNG_COLOR_TYPE_GRAY       0
#define PNG_COLOR_TYPE_RGB        2
#define PNG_COLOR_TYPE_GA         4
#define PNG_COLOR_TYPE_GRAY_ALPHA PNG_COLOR_TYPE_GA
#define PNG_COLOR_TYPE_RGBA       6
#define PNG_COLOR_TYPE_RGB_ALPHA  PNG_COLOR_TYPE_RGBA

#define PNG_COMPRESSION_TYPE_BASE    0
#define PNG_COMPRESSION_TYPE_DEFAULT PNG_COMPRESSION_TYPE_BASE

#define PNG_INTERLACE_NONE  0
#define PNG_INTERLACE_ADAM7 1

#define PNG_FILTER_TYPE_BASE    0
#define PNG_FILTER_TYPE_DEFAULT PNG_FILTER_TYPE_BASE

typedef uint8_t png_byte;
typedef png_byte *png_bytep;
typedef png_byte **png_bytepp;
typedef const png_byte *png_const_bytep;
typedef const png_byte **png_const_bytepp;
typedef struct png_struct png_struct;
typedef png_struct *png_structp;
typedef png_struct **png_structpp;
typedef const png_struct *png_const_structp;
typedef const png_struct **png_const_structpp;
typedef struct png_info png_info;
typedef png_info *png_infop;
typedef png_info **png_infopp;
typedef const png_info *png_const_infop;
typedef const png_info **png_const_infopp;
typedef size_t png_size_t;
typedef void *png_voidp;
typedef const void *png_const_voidp;
typedef char png_char;
typedef png_char *png_charp;
typedef const png_char *png_const_charp;
typedef int16_t png_int_16;
typedef png_int_16 *png_int_16p;
typedef uint16_t png_uint_16;
typedef png_uint_16 *png_uint_16p;
typedef const png_uint_16 *png_const_uint_16p;
typedef int32_t png_int_32;
typedef png_int_32 *png_int_32p;
typedef uint32_t png_uint_32;
typedef png_uint_32 *png_uint_32p;
typedef const png_uint_32 *png_const_uint_32p;
typedef void (*png_error_ptr)(png_structp png, png_const_charp msg);
typedef png_voidp (*png_malloc_ptr)(png_structp png, png_size_t size);
typedef void (*png_free_ptr)(png_structp pmh, png_voidp ptr);

int png_sig_cmp(png_bytep sig, png_size_t start, png_size_t num);
png_structp png_create_read_struct(png_const_charp version,
                                   png_voidp err_ptr,
                                   png_error_ptr err_fn,
                                   png_error_ptr warn_fn);
png_structp png_create_write_struct(png_const_charp version,
                                    png_voidp err_ptr,
                                    png_error_ptr err_fn,
                                    png_error_ptr warn_fn);
png_infop png_create_info_struct(png_structp png);
void png_destroy_read_struct(png_structpp png, png_infopp info, png_infopp end);
void png_destroy_write_struct(png_structpp png, png_infopp info);
void png_init_io(png_structp png, FILE *fp);
void png_read_info(png_structp png, png_infop info);
void png_write_info(png_structp png, png_const_infop info);
void png_set_sig_bytes(png_structp png, int bytes);
int png_set_interlace_handling(png_structp png);
png_size_t png_get_rowbytes(png_structp png, png_infop info);
png_uint_32 png_get_IHDR(png_structp png,
                         png_infop info,
                         png_uint_32p width,
                         png_uint_32p height,
                         int *depth,
                         int *color_type,
                         int *interlace,
                         int *compression,
                         int *filter);
void png_set_IHDR(png_const_structp png,
                  png_infop info,
                  png_uint_32 width,
                  png_uint_32 height,
                  int depth,
                  int color_type,
                  int interlace,
                  int compression,
                  int filter);
void png_read_row(png_structp png, png_bytep row, png_bytep display_row);
void png_read_rows(png_structp png,
                   png_bytepp rows,
                   png_bytepp display_rows,
                   png_uint_32 count);
void png_read_image(png_structp png, png_bytepp rows);
void png_write_row(png_structp png, png_const_bytep row);
void png_write_rows(png_structp png, png_bytepp rows, png_uint_32 count);
void png_write_image(png_structp png, png_bytepp rows);
void png_write_end(png_structp png, png_infop info);
void png_read_update_info(png_structp png, png_infop info);
jmp_buf *png_jmpbufp(png_structp png);
#define png_jmpbuf(png) (*png_jmpbufp(png))
png_voidp png_get_error_ptr(png_structp png);
void png_set_error_fn(png_structp png,
                      png_voidp err_ptr,
                      png_error_ptr err_fn,
                      png_error_ptr warn_fn);
png_voidp png_get_mem_ptr(png_structp png);
void png_set_mem_fn(png_structp png,
                    png_voidp mem_ptr,
                    png_malloc_ptr malloc_fn,
                    png_free_ptr free_fn);

#ifdef __cplusplus
}
#endif

#endif
