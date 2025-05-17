#include "gjit.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

int
gjit_image_alloc(struct jkg_ctx *ctx,
                 const struct jkg_image_create_info *create_info,
                 struct jkg_image **imagep)
{
	struct jkg_image *image;
	size_t size;
	int ret;

	(void)ctx;
	image = malloc(sizeof(*image));
	if (!image)
		return -ENOMEM;
	image->size = create_info->size;
	image->levels = create_info->levels;
	image->layers = create_info->layers;
	image->format = create_info->format;
	image->type = create_info->type;
	switch (image->type)
	{
		case JKG_IMAGE_1D:
			size = image->size.x;
			break;
		case JKG_IMAGE_1D_ARRAY:
			size = image->size.x * image->layers;
			break;
		case JKG_IMAGE_2D:
			size = image->size.x * image->size.y;
			break;
		case JKG_IMAGE_2D_ARRAY:
			size = image->size.x * image->size.y * image->layers;
			break;
		case JKG_IMAGE_3D:
			size = image->size.x * image->size.y * image->size.z;
			break;
		default:
			assert(!"unknown type");
			ret = -EINVAL;
			goto err;
	}
	image->data = malloc(size * jkg_get_stride(image->format));
	if (!image->data)
	{
		ret = -ENOMEM;
		goto err;
	}
	*imagep = image;
	return 0;

err:
	free(image);
	return ret;
}

void
gjit_image_free(struct jkg_ctx *ctx,
                struct jkg_image *image)
{
	(void)ctx;
	free(image->data);
	free(image);
}

int
gjit_image_read(struct jkg_ctx *ctx,
                struct jkg_image *image,
                uint32_t level,
                void *data,
                enum jkg_format format,
                const struct jkg_extent *size,
                const struct jkg_extent *offset)
{
	static const struct jkg_extent null_offset = {0};

	(void)ctx;
	(void)level; /* XXX */
	jkg_image_copy(data,
	               image->data,
	               format,
	               image->format,
	               size,
	               &image->size,
	               &null_offset,
	               offset,
	               size);
	return 0;
}

int
gjit_image_write(struct jkg_ctx *ctx,
                 struct jkg_image *image,
                 uint32_t level,
                 const void *data,
                 enum jkg_format format,
                 const struct jkg_extent *size,
                 const struct jkg_extent *offset)
{
	static const struct jkg_extent null_offset = {0};

	(void)ctx;
	(void)level; /* XXX */
	jkg_image_copy(image->data,
	               data,
	               image->format,
	               format,
	               &image->size,
	               size,
	               offset,
	               &null_offset,
	               size);
	return 0;
}

int
gjit_image_copy(struct jkg_ctx *ctx,
                struct jkg_image *dst,
                struct jkg_image *src,
                uint32_t dst_level,
                uint32_t src_level,
                const struct jkg_extent *dst_off,
                const struct jkg_extent *src_off,
                const struct jkg_extent *size)
{
	(void)ctx;
	(void)dst_level; /* XXX */
	(void)src_level; /* XXX */
	jkg_image_copy(dst->data,
	               src->data,
	               dst->format,
	               src->format,
	               &dst->size,
	               &src->size,
	               dst_off,
	               src_off,
	               size);
	return 0;
}
