#ifndef LIBJPEG_JPEG_H
#define LIBJPEG_JPEG_H

#include <stdint.h>
#include <stdio.h>

#define JPEG_SUBSAMPLING_444 0x1
#define JPEG_SUBSAMPLING_440 0x2
#define JPEG_SUBSAMPLING_422 0x3
#define JPEG_SUBSAMPLING_420 0x4
#define JPEG_SUBSAMPLING_411 0x5
#define JPEG_SUBSAMPLING_410 0x6

struct jpeg;

struct jpeg *jpeg_new(void);
void jpeg_free(struct jpeg *jpeg);
void jpeg_init_io(struct jpeg *jpeg, FILE *fp);
const uint8_t *jpeg_get_thumbnail(struct jpeg *jpeg,
                                  uint8_t *width,
                                  uint8_t *height);
int jpeg_set_thumbnail(struct jpeg *jpeg,
                       const uint8_t *data,
                       uint8_t width,
                       uint8_t height);
void jpeg_set_quality(struct jpeg *jpeg, int quality);
void jpeg_set_restart_interval(struct jpeg *jpeg, uint16_t restart_interval);
int jpeg_set_subsampling(struct jpeg *jpeg, int subsampling);
void jpeg_get_info(struct jpeg *jpeg,
                   uint32_t *width,
                   uint32_t *height,
                   uint8_t *components);
int jpeg_set_info(struct jpeg *jpeg,
                  uint32_t width,
                  uint32_t height,
                  uint8_t components);
const char *jpeg_get_err(struct jpeg *jpeg);

int jpeg_write_headers(struct jpeg *jpeg);
int jpeg_write_data(struct jpeg *jpeg, const void *data);

int jpeg_read_headers(struct jpeg *jpeg);
int jpeg_read_data(struct jpeg *jpeg, void *data);

#endif
