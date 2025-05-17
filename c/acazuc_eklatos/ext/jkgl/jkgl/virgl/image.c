#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

static enum pipe_texture_target
get_target(enum jkg_image_type type)
{
	switch (type)
	{
		default:
			assert(!"unknown image type");
			/* FALLTHROUGH */
		case JKG_IMAGE_1D:
			return PIPE_TEXTURE_1D;
		case JKG_IMAGE_1D_ARRAY:
			return PIPE_TEXTURE_1D_ARRAY;
		case JKG_IMAGE_2D:
			return PIPE_TEXTURE_2D;
		case JKG_IMAGE_2D_ARRAY:
			return PIPE_TEXTURE_2D_ARRAY;
		case JKG_IMAGE_3D:
			return PIPE_TEXTURE_3D;
	}
}

int
virgl_image_alloc(struct jkg_ctx *ctx,
                  const struct jkg_image_create_info *create_info,
                  struct jkg_image **imagep)
{
	struct virgl_res_info res_info;
	struct jkg_image *image;
	int ret;

	image = calloc(1, sizeof(*image));
	if (!image)
		return -ENOMEM;
	image->type = create_info->type;
	image->format = create_info->format;
	res_info.target = get_target(create_info->type);
	res_info.format = virgl_get_format(create_info->format);
	res_info.bind = VIRGL_BIND_RENDER_TARGET | VIRGL_BIND_DEPTH_STENCIL | VIRGL_BIND_SAMPLER_VIEW;
	res_info.size = create_info->size;
	res_info.array_size = create_info->layers;
	res_info.last_level = create_info->levels - 1;
	res_info.nr_samples = 0;
	res_info.flags = VIRGL_RESOURCE_Y_0_TOP;
	switch (create_info->type)
	{
		case JKG_IMAGE_1D:
			res_info.bytes = jkg_get_stride(create_info->format)
			               * create_info->size.x;
			break;
		case JKG_IMAGE_1D_ARRAY:
			res_info.bytes = jkg_get_stride(create_info->format)
			               * create_info->size.x
			               * create_info->layers;
			break;
		case JKG_IMAGE_2D:
			res_info.bytes = jkg_get_stride(create_info->format)
			               * create_info->size.x
			               * create_info->size.y;
			break;
		case JKG_IMAGE_2D_ARRAY:
			res_info.bytes = jkg_get_stride(create_info->format)
			               * create_info->size.x
			               * create_info->size.y
			               * create_info->layers;
			break;
		case JKG_IMAGE_3D:
			res_info.bytes = jkg_get_stride(create_info->format)
			               * create_info->size.x
			               * create_info->size.y
			               * create_info->size.z;
			break;
	}
	ret = virgl_res_alloc(ctx, &res_info, &image->res);
	if (ret)
		goto err;
	*imagep = image;
	return 0;

err:
	free(image);
	return ret;
}

int
virgl_image_read(struct jkg_ctx *ctx,
                 struct jkg_image *image,
                 uint32_t level,
                 void *data,
                 enum jkg_format format,
                 const struct jkg_extent *size,
                 const struct jkg_extent *offset)
{
	static const struct jkg_extent null_offset = {0};
	int ret;

	ret = virgl_flush(ctx);
	if (ret)
		return ret;
	ret = vgl_transfer_in(ctx,
	                      image->res,
	                      &null_offset,
	                      &image->res->info.size,
	                      0,
	                      level,
	                      0,
	                      0);
	if (ret)
		return ret;
	jkg_image_copy(data,
	               image->res->data,
	               format,
	               image->format,
	               size,
	               &image->res->info.size,
	               &null_offset,
	               offset,
	               size);
	return 0;
}

int
virgl_image_write(struct jkg_ctx *ctx,
                  struct jkg_image *image,
                  uint32_t level,
                  const void *data,
                  enum jkg_format format,
                  const struct jkg_extent *size,
                  const struct jkg_extent *offset)
{
	static const struct jkg_extent null_offset = {0};
	int ret;

	ret = virgl_flush(ctx);
	if (ret)
		return ret;
	jkg_image_copy(image->res->data,
	               data,
	               image->format,
	               format,
	               &image->res->info.size,
	               size,
	               offset,
	               &null_offset,
	               size);
	ret = vgl_transfer_out(ctx,
	                       image->res,
	                       &null_offset,
	                       &image->res->info.size,
	                       0,
	                       level,
	                       0,
	                       0);
	if (ret)
		return ret;
	return 0;
}

int
virgl_image_copy(struct jkg_ctx *ctx,
                 struct jkg_image *dst,
                 struct jkg_image *src,
                 uint32_t dst_level,
                 uint32_t src_level,
                 const struct jkg_extent *dst_off,
                 const struct jkg_extent *src_off,
                 const struct jkg_extent *size)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_RESOURCE_COPY_REGION,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_CMD_RESOURCE_COPY_REGION_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_CMD_RCR_DST_RES_HANDLE] = src->res->id;
	request[VIRGL_CMD_RCR_DST_LEVEL] = dst_level;
	request[VIRGL_CMD_RCR_DST_X] = dst_off->x;
	request[VIRGL_CMD_RCR_DST_Y] = dst_off->y;
	request[VIRGL_CMD_RCR_DST_Z] = dst_off->z;
	request[VIRGL_CMD_RCR_SRC_RES_HANDLE] = dst->res->id;
	request[VIRGL_CMD_RCR_SRC_LEVEL] = src_level;
	request[VIRGL_CMD_RCR_SRC_X] = src_off->x;
	request[VIRGL_CMD_RCR_SRC_Y] = src_off->y;
	request[VIRGL_CMD_RCR_SRC_Z] = src_off->z;
	request[VIRGL_CMD_RCR_SRC_W] = size->x;
	request[VIRGL_CMD_RCR_SRC_H] = size->y;
	request[VIRGL_CMD_RCR_SRC_D] = size->z;
	return 0;
}

void
virgl_image_free(struct jkg_ctx *ctx,
                 struct jkg_image *image)
{
	virgl_res_free(ctx, image->res);
	free(image);
}
