#include "jkg.h"

#include <string.h>

static void
image_copy_same(void *dstv,
                const void *srcv,
                enum jkg_format format,
                const struct jkg_extent *dst_size,
                const struct jkg_extent *src_size,
                const struct jkg_extent *dst_offset,
                const struct jkg_extent *src_offset,
                const struct jkg_extent *size)
{
	const uint8_t *src;
	uint8_t *dst;
	size_t src_strides[2];
	size_t dst_strides[2];
	size_t stride;
	size_t cpy_size;

	src = srcv;
	dst = dstv;
	stride = jkg_get_stride(format);
	cpy_size = size->x * stride;
	if (size->z)
	{
		src += (src_offset->x + (src_offset->y + src_offset->z * src_size->y) * src_size->x) * stride;
		dst += (dst_offset->x + (dst_offset->y + dst_offset->z * dst_size->y) * dst_size->x) * stride;
		src_strides[0] = src_size->x * stride;
		dst_strides[0] = dst_size->x * stride;
		src_strides[1] = src_size->y * src_strides[0];
		dst_strides[1] = dst_size->y * dst_strides[0];
		for (uint32_t z = 0; z < size->z; ++z)
		{
			const uint8_t *src_line = src;
			uint8_t *dst_line = dst;
			for (uint32_t y = 0; y < size->y; ++y)
			{
				memcpy(dst_line, src_line, cpy_size);
				dst_line += dst_strides[0];
				src_line += src_strides[0];
			}
			dst += dst_strides[1];
			src += src_strides[1];
		}
		return;
	}
	if (size->y)
	{
		src += (src_offset->x + src_offset->y * src_size->x) * stride;
		dst += (dst_offset->x + dst_offset->y * dst_size->x) * stride;
		src_strides[0] = src_size->x * stride;
		dst_strides[0] = dst_size->x * stride;
		for (uint32_t y = 0; y < size->y; ++y)
		{
			memcpy(dst, src, cpy_size);
			dst += dst_strides[0];
			src += src_strides[0];
		}
		return;
	}
	if (size->x)
	{
		src += src_offset->x * stride;
		dst += dst_offset->x * stride;
		memcpy(dst, src, cpy_size);
		return;
	}
}

static void
copy_x_vec4f(uint8_t *dst,
             const uint8_t *src,
             enum jkg_format dst_format,
             enum jkg_format src_format,
             size_t *dst_strides,
             size_t *src_strides,
             const struct jkg_extent *size)
{
	for (size_t x = 0; x < size->x; ++x)
	{
		float color[4];
		jkg_get_vec4f(color, src, src_format);
		jkg_set_vec4f(dst,
		              color,
		              dst_format,
		              JKG_COLOR_MASK_R
		            | JKG_COLOR_MASK_G
		            | JKG_COLOR_MASK_B
		            | JKG_COLOR_MASK_A);
		src += src_strides[0];
		dst += dst_strides[0];
	}
}

static void
copy_x_vec4u(uint8_t *dst,
             const uint8_t *src,
             enum jkg_format dst_format,
             enum jkg_format src_format,
             size_t *dst_strides,
             size_t *src_strides,
             const struct jkg_extent *size)
{
	for (size_t x = 0; x < size->x; ++x)
	{
		uint32_t color[4];
		jkg_get_vec4u(color, src, src_format);
		jkg_set_vec4u(dst,
		              color,
		              dst_format,
		              JKG_COLOR_MASK_R
		            | JKG_COLOR_MASK_G
		            | JKG_COLOR_MASK_B
		            | JKG_COLOR_MASK_A);
		src += src_strides[0];
		dst += dst_strides[0];
	}
}

static void
copy_x_depth(uint8_t *dst,
             const uint8_t *src,
             enum jkg_format dst_format,
             enum jkg_format src_format,
             size_t *dst_strides,
             size_t *src_strides,
             const struct jkg_extent *size)
{
	for (size_t x = 0; x < size->x; ++x)
	{
		jkg_set_depth(dst, jkg_get_depth(src, src_format), dst_format);
		src += src_strides[0];
		dst += dst_strides[0];
	}
}

static void
copy_x_stencil(uint8_t *dst,
               const uint8_t *src,
               enum jkg_format dst_format,
               enum jkg_format src_format,
               size_t *dst_strides,
               size_t *src_strides,
               const struct jkg_extent *size)
{
	for (size_t x = 0; x < size->x; ++x)
	{
		jkg_set_stencil(dst, jkg_get_stencil(src, src_format), dst_format);
		src += src_strides[0];
		dst += dst_strides[0];
	}
}

#define COPY_COMP(name, id, comp, next) \
static void \
copy_##comp##_##name(uint8_t *dst, \
                     const uint8_t *src, \
                     enum jkg_format dst_format, \
                     enum jkg_format src_format, \
                     size_t *dst_strides, \
                     size_t *src_strides, \
                     const struct jkg_extent *size) \
{ \
	for (uint32_t comp = 0; comp < size->comp; ++comp) \
	{ \
		copy_##next##_##name(dst, \
		                     src, \
		                     dst_format, \
		                     src_format, \
		                     dst_strides, \
		                     src_strides, \
		                     size); \
		dst += dst_strides[id]; \
		src += src_strides[id]; \
	} \
}

COPY_COMP(vec4f, 1, y, x);
COPY_COMP(vec4f, 2, z, y);
COPY_COMP(vec4u, 1, y, x);
COPY_COMP(vec4u, 2, z, y);
COPY_COMP(depth, 1, y, x);
COPY_COMP(depth, 2, z, y);
COPY_COMP(stencil, 1, y, x);
COPY_COMP(stencil, 2, z, y);

#undef COPY_COMP

void
jkg_image_copy(void *dstv,
               const void *srcv,
               enum jkg_format dst_format,
               enum jkg_format src_format,
               const struct jkg_extent *dst_size,
               const struct jkg_extent *src_size,
               const struct jkg_extent *dst_offset,
               const struct jkg_extent *src_offset,
               const struct jkg_extent *size)
{
#define COPY_COMP(comp) \
do \
{ \
	switch (src_type) \
	{ \
		case JKG_FORMAT_TYPE_UNKNOWN: \
			break; \
		case JKG_FORMAT_TYPE_COLOR_UNORM: \
		case JKG_FORMAT_TYPE_COLOR_SNORM: \
		case JKG_FORMAT_TYPE_COLOR_USCALED: \
		case JKG_FORMAT_TYPE_COLOR_SSCALED: \
		case JKG_FORMAT_TYPE_COLOR_SFLOAT: \
		case JKG_FORMAT_TYPE_COLOR_SNORM_BLOCK: \
		case JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK: \
			copy_##comp##_vec4f(dst, \
			                    src, \
			                    dst_format, \
			                    src_format, \
			                    dst_strides, \
			                    src_strides, \
			                    size); \
			break; \
		case JKG_FORMAT_TYPE_COLOR_UINT: \
		case JKG_FORMAT_TYPE_COLOR_SINT: \
			copy_##comp##_vec4u(dst, \
			                    src, \
			                    dst_format, \
			                    src_format, \
			                    dst_strides, \
			                    src_strides, \
			                    size); \
			break; \
		case JKG_FORMAT_TYPE_DEPTH_SFLOAT: \
			copy_##comp##_depth(dst, \
			                    src, \
			                    dst_format, \
			                    src_format, \
			                    dst_strides, \
			                    src_strides, \
			                    size); \
			break; \
		case JKG_FORMAT_TYPE_STENCIL_UINT: \
			copy_##comp##_stencil(dst, \
			                      src, \
			                      dst_format, \
			                      src_format, \
			                      dst_strides, \
			                      src_strides, \
			                      size); \
			break; \
		case JKG_FORMAT_TYPE_DEPTH_UNORM_STENCIL_UINT: \
			copy_##comp##_depth(dst, \
			                    src, \
			                    dst_format, \
			                    src_format, \
			                    dst_strides, \
			                    src_strides, \
			                    size); \
			copy_##comp##_stencil(dst, \
			                      src, \
			                      dst_format, \
			                      src_format, \
			                      dst_strides, \
			                      src_strides, \
			                      size); \
			break; \
	} \
} while (0)

	const uint8_t *src;
	uint8_t *dst;
	size_t src_strides[3];
	size_t dst_strides[3];
	enum jkg_format_type src_type;
	enum jkg_format_type dst_type;

	if (dst_format == src_format)
	{
		image_copy_same(dstv,
		                srcv,
		                dst_format,
		                dst_size,
		                src_size,
		                dst_offset,
		                src_offset,
		                size);
		return;
	}
	src_type = jkg_get_format_type(src_format);
	dst_type = jkg_get_format_type(dst_format);
	src = srcv;
	dst = dstv;
	src_strides[0] = jkg_get_stride(src_format);
	dst_strides[0] = jkg_get_stride(dst_format);
	if (size->z)
	{
		src += (src_offset->x + (src_offset->y + src_offset->z * src_size->y) * src_size->x) * src_strides[0];
		dst += (dst_offset->x + (dst_offset->y + dst_offset->z * dst_size->y) * dst_size->x) * dst_strides[0];
		src_strides[1] = src_size->x * src_strides[0];
		dst_strides[1] = dst_size->x * dst_strides[0];
		src_strides[2] = src_size->y * src_strides[1];
		dst_strides[2] = dst_size->y * dst_strides[1];
		COPY_COMP(z);
		return;
	}
	if (size->y)
	{
		src += (src_offset->x + src_offset->y * src_size->x) * src_strides[0];
		dst += (dst_offset->x + dst_offset->y * dst_size->x) * dst_strides[0];
		src_strides[1] = src_size->x * src_strides[0];
		dst_strides[1] = dst_size->x * dst_strides[0];
		COPY_COMP(y);
		return;
	}
	if (size->x)
	{
		src += src_offset->x * src_strides[0];
		dst += dst_offset->x * dst_strides[0];
		COPY_COMP(x);
		return;
	}

#undef COPY_COMP
}

void
jkg_clear_vec4f(void *datav,
                uint32_t width,
                uint32_t height,
                enum jkg_format format,
                enum jkg_color_mask color_mask,
                int32_t rect[4],
                const float color[4])
{
#define CLEAR_COLOR(ncomp, r, g, b, a, type, scale) \
do \
{ \
	uint32_t stride = width * (ncomp); \
	type *data = datav; \
	data += rect[2] * stride; \
	data += rect[0] * (ncomp); \
	type vr = color[0] * (scale); \
	type vg = color[1] * (scale); \
	type vb = color[2] * (scale); \
	type va = color[3] * (scale); \
	if (color_mask == (JKG_COLOR_MASK_R | JKG_COLOR_MASK_G | JKG_COLOR_MASK_B | JKG_COLOR_MASK_A)) \
	{ \
		for (int32_t y = rect[2]; y < rect[3]; ++y) \
		{ \
			type * restrict line = data; \
			for (int32_t x = rect[0]; x < rect[1]; ++x) \
			{ \
				if ((ncomp) > 0) \
					line[r] = vr; \
				if ((ncomp) > 1) \
					line[g] = vg; \
				if ((ncomp) > 2) \
					line[b] = vb; \
				if ((ncomp) > 3) \
					line[a] = va; \
				line += (ncomp); \
			} \
			data += stride; \
		} \
	} \
	else \
	{ \
		if ((ncomp) > 0 && (color_mask & JKG_COLOR_MASK_R)) \
		{ \
			type *comp_data = data; \
			for (int32_t y = rect[2]; y < rect[3]; ++y) \
			{ \
				type * restrict line = comp_data; \
				for (int32_t x = rect[0]; x < rect[1]; ++x) \
				{ \
					line[r] = vr; \
					line += (ncomp); \
				} \
				comp_data += stride; \
			} \
		} \
		if ((ncomp) > 1 && (color_mask & JKG_COLOR_MASK_G)) \
		{ \
			type *comp_data = data; \
			for (int32_t y = rect[2]; y < rect[3]; ++y) \
			{ \
				type * restrict line = comp_data; \
				for (int32_t x = rect[0]; x < rect[1]; ++x) \
				{ \
					line[g] = vg; \
					line += (ncomp); \
				} \
				comp_data += stride; \
			} \
		} \
		if ((ncomp) > 2 && (color_mask & JKG_COLOR_MASK_B)) \
		{ \
			type *comp_data = data; \
			for (int32_t y = rect[2]; y < rect[3]; ++y) \
			{ \
				type * restrict line = comp_data; \
				for (int32_t x = rect[0]; x < rect[1]; ++x) \
				{ \
					line[b] = vb; \
					line += (ncomp); \
				} \
				comp_data += stride; \
			} \
		} \
		if ((ncomp > 3) && (color_mask & JKG_COLOR_MASK_A)) \
		{ \
			type *comp_data = data; \
			for (int32_t y = rect[2]; y < rect[3]; ++y) \
			{ \
				type * restrict line = comp_data; \
				for (int32_t x = rect[0]; x < rect[1]; ++x) \
				{ \
					line[a] = va; \
					line += (ncomp); \
				} \
				comp_data += stride; \
			} \
		} \
	} \
} while (0)

#define CLEAR_COLOR_PACK(type, mask_r, mask_g, mask_b, mask_a) \
do \
{ \
	type *data = datav; \
	data += rect[2] * width; \
	data += rect[0]; \
	if (color_mask == (JKG_COLOR_MASK_R | JKG_COLOR_MASK_G | JKG_COLOR_MASK_B | JKG_COLOR_MASK_A)) \
	{ \
		for (int32_t y = rect[2]; y < rect[3]; ++y) \
		{ \
			type * restrict line = data; \
			for (int32_t x = rect[0]; x < rect[1]; ++x) \
			{ \
				*line = value; \
				line++; \
			} \
			data += width; \
		} \
	} \
	else \
	{ \
		type mask = 0; \
		if (color_mask & JKG_COLOR_MASK_R) \
			mask |= mask_r; \
		if (color_mask & JKG_COLOR_MASK_G) \
			mask |= mask_g; \
		if (color_mask & JKG_COLOR_MASK_B) \
			mask |= mask_b; \
		if (color_mask & JKG_COLOR_MASK_A) \
			mask |= mask_a; \
		value &= mask; \
		mask = ~mask; \
		for (int32_t y = rect[2]; y < rect[3]; ++y) \
		{ \
			type * restrict line = data; \
			for (int32_t x = rect[0]; x < rect[1]; ++x) \
			{ \
				*line = (*line & mask) | value; \
				line++; \
			} \
			data += width; \
		} \
	} \
} while (0)

#define SET_1(value, offset) (((uint32_t)((value) * 0x01) & 0x01) << (offset))
#define SET_4(value, offset) (((uint32_t)((value) * 0x0F) & 0x0F) << (offset))
#define SET_5(value, offset) (((uint32_t)((value) * 0x1F) & 0x1F) << (offset))
#define SET_6(value, offset) (((uint32_t)((value) * 0x3F) & 0x3F) << (offset))

	(void)height;
	switch (format)
	{
		case JKG_FORMAT_R8_UNORM:
			CLEAR_COLOR(1, 0, 1, 2, 3, uint8_t, UINT8_MAX);
			break;
		case JKG_FORMAT_R8_SNORM:
			CLEAR_COLOR(1, 0, 1, 2, 3, int8_t, INT8_MAX);
			break;
		case JKG_FORMAT_R8_USCALED:
			CLEAR_COLOR(1, 0, 1, 2, 3, uint8_t, 1);
			break;
		case JKG_FORMAT_R8_SSCALED:
			CLEAR_COLOR(1, 0, 1, 2, 3, int8_t, 1);
			break;
		case JKG_FORMAT_R8G8_UNORM:
			CLEAR_COLOR(2, 0, 1, 2, 3, uint8_t, UINT8_MAX);
			break;
		case JKG_FORMAT_R8G8_SNORM:
			CLEAR_COLOR(2, 0, 1, 2, 3, int8_t, INT8_MAX);
			break;
		case JKG_FORMAT_R8G8_USCALED:
			CLEAR_COLOR(2, 0, 1, 2, 3, uint8_t, 1);
			break;
		case JKG_FORMAT_R8G8_SSCALED:
			CLEAR_COLOR(2, 0, 1, 2, 3, int8_t, 1);
			break;
		case JKG_FORMAT_R8G8B8_UNORM:
			CLEAR_COLOR(3, 0, 1, 2, 3, uint8_t, UINT8_MAX);
			break;
		case JKG_FORMAT_R8G8B8_SNORM:
			CLEAR_COLOR(3, 0, 1, 2, 3, int8_t, INT8_MAX);
			break;
		case JKG_FORMAT_R8G8B8_USCALED:
			CLEAR_COLOR(3, 0, 1, 2, 3, uint8_t, 1);
			break;
		case JKG_FORMAT_R8G8B8_SSCALED:
			CLEAR_COLOR(3, 0, 1, 2, 3, int8_t, 1);
			break;
		case JKG_FORMAT_B8G8R8_UNORM:
			CLEAR_COLOR(3, 2, 1, 0, 3, uint8_t, UINT8_MAX);
			break;
		case JKG_FORMAT_B8G8R8_SNORM:
			CLEAR_COLOR(3, 2, 1, 0, 3, int8_t, INT8_MAX);
			break;
		case JKG_FORMAT_B8G8R8_USCALED:
			CLEAR_COLOR(3, 2, 1, 0, 3, uint8_t, 1);
			break;
		case JKG_FORMAT_B8G8R8_SSCALED:
			CLEAR_COLOR(3, 2, 1, 0, 3, int8_t, 1);
			break;
		case JKG_FORMAT_R8G8B8A8_UNORM:
			CLEAR_COLOR(4, 0, 1, 2, 3, uint8_t, UINT8_MAX);
			break;
		case JKG_FORMAT_R8G8B8A8_SNORM:
			CLEAR_COLOR(4, 0, 1, 2, 3, int8_t, INT8_MAX);
			break;
		case JKG_FORMAT_R8G8B8A8_USCALED:
			CLEAR_COLOR(4, 0, 1, 2, 3, uint8_t, 1);
			break;
		case JKG_FORMAT_R8G8B8A8_SSCALED:
			CLEAR_COLOR(4, 0, 1, 2, 3, int8_t, 1);
			break;
		case JKG_FORMAT_B8G8R8A8_UNORM:
			CLEAR_COLOR(4, 2, 1, 0, 3, uint8_t, UINT8_MAX);
			break;
		case JKG_FORMAT_B8G8R8A8_SNORM:
			CLEAR_COLOR(4, 2, 1, 0, 3, int8_t, INT8_MAX);
			break;
		case JKG_FORMAT_B8G8R8A8_USCALED:
			CLEAR_COLOR(4, 2, 1, 0, 3, uint8_t, 1);
			break;
		case JKG_FORMAT_B8G8R8A8_SSCALED:
			CLEAR_COLOR(4, 2, 1, 0, 3, int8_t, 1);
			break;
		case JKG_FORMAT_R16_UNORM:
			CLEAR_COLOR(1, 0, 1, 2, 3, uint16_t, UINT16_MAX);
			break;
		case JKG_FORMAT_R16_SNORM:
			CLEAR_COLOR(1, 0, 1, 2, 3, int16_t, INT16_MAX);
			break;
		case JKG_FORMAT_R16_USCALED:
			CLEAR_COLOR(1, 0, 1, 2, 3, uint16_t, 1);
			break;
		case JKG_FORMAT_R16_SSCALED:
			CLEAR_COLOR(1, 0, 1, 2, 3, int16_t, 1);
			break;
		case JKG_FORMAT_R16G16_UNORM:
			CLEAR_COLOR(2, 0, 1, 2, 3, uint16_t, UINT16_MAX);
			break;
		case JKG_FORMAT_R16G16_SNORM:
			CLEAR_COLOR(2, 0, 1, 2, 3, int16_t, INT16_MAX);
			break;
		case JKG_FORMAT_R16G16_USCALED:
			CLEAR_COLOR(2, 0, 1, 2, 3, uint16_t, 1);
			break;
		case JKG_FORMAT_R16G16_SSCALED:
			CLEAR_COLOR(2, 0, 1, 2, 3, int16_t, 1);
			break;
		case JKG_FORMAT_R16G16B16_UNORM:
			CLEAR_COLOR(3, 0, 1, 2, 3, uint16_t, UINT16_MAX);
			break;
		case JKG_FORMAT_R16G16B16_SNORM:
			CLEAR_COLOR(3, 0, 1, 2, 3, int16_t, INT16_MAX);
			break;
		case JKG_FORMAT_R16G16B16_USCALED:
			CLEAR_COLOR(3, 0, 1, 2, 3, uint16_t, 1);
			break;
		case JKG_FORMAT_R16G16B16_SSCALED:
			CLEAR_COLOR(3, 0, 1, 2, 3, int16_t, 1);
			break;
		case JKG_FORMAT_R16G16B16A16_UNORM:
			CLEAR_COLOR(4, 0, 1, 2, 3, uint16_t, UINT16_MAX);
			break;
		case JKG_FORMAT_R16G16B16A16_SNORM:
			CLEAR_COLOR(4, 0, 1, 2, 3, int16_t, INT16_MAX);
			break;
		case JKG_FORMAT_R16G16B16A16_USCALED:
			CLEAR_COLOR(4, 0, 1, 2, 3, uint16_t, 1);
			break;
		case JKG_FORMAT_R16G16B16A16_SSCALED:
			CLEAR_COLOR(4, 0, 1, 2, 3, int16_t, 1);
			break;
		case JKG_FORMAT_R32_SFLOAT:
			CLEAR_COLOR(1, 0, 1, 2, 3, float, 1);
			break;
		case JKG_FORMAT_R32G32_SFLOAT:
			CLEAR_COLOR(2, 0, 1, 2, 3, float, 1);
			break;
		case JKG_FORMAT_R32G32B32_SFLOAT:
			CLEAR_COLOR(3, 0, 1, 2, 3, float, 1);
			break;
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
			CLEAR_COLOR(4, 0, 1, 2, 3, float, 1);
			break;
		case JKG_FORMAT_R64_SFLOAT:
			CLEAR_COLOR(1, 0, 1, 2, 3, double, 1);
			break;
		case JKG_FORMAT_R64G64_SFLOAT:
			CLEAR_COLOR(2, 0, 1, 2, 3, double, 1);
			break;
		case JKG_FORMAT_R64G64B64_SFLOAT:
			CLEAR_COLOR(3, 0, 1, 2, 3, double, 1);
			break;
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
			CLEAR_COLOR(4, 0, 1, 2, 3, double, 1);
			break;
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		{
			uint8_t value = SET_4(color[0], 4)
			              | SET_4(color[1], 0);
			CLEAR_COLOR_PACK(uint8_t, 0xF0, 0x0F, 0, 0);
			break;
		}
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		{
			uint16_t value = SET_4(color[0], 0xC)
			               | SET_4(color[1], 0x8)
			               | SET_4(color[2], 0x4)
			               | SET_4(color[3], 0x0);
			CLEAR_COLOR_PACK(uint16_t, 0xF000, 0x0F00, 0x00F0, 0x000F);
			break;
		}
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		{
			uint16_t value = SET_4(color[0], 0x4)
			               | SET_4(color[1], 0x8)
			               | SET_4(color[2], 0xC)
			               | SET_4(color[3], 0x0);
			CLEAR_COLOR_PACK(uint16_t, 0x00F0, 0x0F00, 0xF000, 0x000F);
			break;
		}
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		{
			uint16_t value = SET_4(color[0], 0x8)
			               | SET_4(color[1], 0x4)
			               | SET_4(color[2], 0x0)
			               | SET_4(color[3], 0xC);
			CLEAR_COLOR_PACK(uint16_t, 0x0F00, 0x00F0, 0x000F, 0xF000);
			break;
		}
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		{
			uint16_t value = SET_4(color[0], 0x0)
			               | SET_4(color[1], 0x4)
			               | SET_4(color[2], 0x8)
			               | SET_4(color[3], 0xC);
			CLEAR_COLOR_PACK(uint16_t, 0x000F, 0x00F0, 0x0F00, 0xF000);
			break;
		}
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		{
			uint16_t value = SET_5(color[0], 0xB)
			               | SET_6(color[1], 0x5)
			               | SET_5(color[2], 0x0);
			CLEAR_COLOR_PACK(uint16_t, 0xF800, 0x07E0, 0x001F, 0x0000);
			break;
		}
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		{
			uint16_t value = SET_5(color[0], 0x0)
			               | SET_6(color[1], 0x5)
			               | SET_5(color[2], 0xB);
			CLEAR_COLOR_PACK(uint16_t, 0x001F, 0x07E0, 0xF800, 0x0000);
			break;
		}
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		{
			uint16_t value = SET_5(color[0], 0xB)
			               | SET_5(color[1], 0x6)
			               | SET_5(color[2], 0x1)
			               | SET_1(color[3], 0x0);
			CLEAR_COLOR_PACK(uint16_t, 0xF800, 0x07C0, 0x003E, 0x0001);
			break;
		}
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		{
			uint16_t value = SET_5(color[0], 0x1)
			               | SET_5(color[1], 0x6)
			               | SET_5(color[2], 0xB)
			               | SET_1(color[3], 0x0);
			CLEAR_COLOR_PACK(uint16_t, 0x003E, 0x07C0, 0xF800, 0x0001);
			break;
		}
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		{
			uint16_t value = SET_5(color[0], 0xA)
			               | SET_5(color[1], 0x5)
			               | SET_5(color[2], 0x0)
			               | SET_1(color[3], 0xF);
			CLEAR_COLOR_PACK(uint16_t, 0x7C00, 0x03E0, 0x001F, 0x8000);
			break;
		}
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
		{
			uint16_t value = SET_5(color[0], 0x0)
			               | SET_5(color[1], 0x5)
			               | SET_5(color[2], 0xA)
			               | SET_1(color[3], 0xF);
			CLEAR_COLOR_PACK(uint16_t, 0x001F, 0x03E0, 0x7C00, 0x8000);
			break;
		}
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
			break;
	}

#undef SET_6
#undef SET_5
#undef SET_4
#undef SET_1
#undef CLEAR_COLOR_PACK
#undef CLEAR_COLOR
}

void
jkg_get_vec4f(float * restrict color,
              const void * restrict datav,
              enum jkg_format format)
{
#define GET_COLOR(count, r, g, b, a, type, scale) \
do \
{ \
	const type * restrict data = datav; \
	if (count > 0) \
		color[r] = data[0] * (scale); \
	else \
		color[r] = 0; \
	if (count > 1) \
		color[g] = data[1] * (scale); \
	else \
		color[g] = 0; \
	if (count > 2) \
		color[b] = data[2] * (scale); \
	else \
		color[b] = 0; \
	if (count > 3) \
		color[a] = data[3] * (scale); \
	else \
		color[a] = 1; \
} while (0)

#define GET_8_UNORM(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, uint8_t, (1.0 / UINT8_MAX))

#define GET_8_SNORM(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, int8_t, (1.0 / INT8_MAX))

#define GET_8_USCALED(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, uint8_t, 1)

#define GET_8_SSCALED(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, int8_t, 1)

#define GET_16_UNORM(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, uint16_t, (1.0 / UINT16_MAX))

#define GET_16_SNORM(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, int16_t, (1.0 / INT16_MAX))

#define GET_16_USCALED(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, uint16_t, 1)

#define GET_16_SSCALED(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, int16_t, 1)

#define GET_32_SFLOAT(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, float, 1)

#define GET_64_SFLOAT(count, r, g, b, a) \
	GET_COLOR(count, r, g, b, a, double, 1)

#define GET_1(value, offset) ((((value) >> (offset)) & 0x01) * (1.0 / 0x01))
#define GET_4(value, offset) ((((value) >> (offset)) & 0x0F) * (1.0 / 0x0F))
#define GET_5(value, offset) ((((value) >> (offset)) & 0x1F) * (1.0 / 0x1F))
#define GET_6(value, offset) ((((value) >> (offset)) & 0x3F) * (1.0 / 0x3F))

	switch (format)
	{
		case JKG_FORMAT_R8_UNORM:
			GET_8_UNORM(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8_SNORM:
			GET_8_SNORM(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8_USCALED:
			GET_8_USCALED(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8_SSCALED:
			GET_8_SSCALED(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8_UNORM:
			GET_8_UNORM(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8_SNORM:
			GET_8_SNORM(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8_USCALED:
			GET_8_USCALED(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8_SSCALED:
			GET_8_SSCALED(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8_UNORM:
			GET_8_UNORM(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8_SNORM:
			GET_8_SNORM(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8_USCALED:
			GET_8_USCALED(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8_SSCALED:
			GET_8_SSCALED(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_B8G8R8_UNORM:
			GET_8_UNORM(3, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8_SNORM:
			GET_8_SNORM(3, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8_USCALED:
			GET_8_USCALED(3, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8_SSCALED:
			GET_8_SSCALED(3, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_R8G8B8A8_UNORM:
			GET_8_UNORM(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8A8_SNORM:
			GET_8_SNORM(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8A8_USCALED:
			GET_8_USCALED(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8A8_SSCALED:
			GET_8_SSCALED(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_B8G8R8A8_UNORM:
			GET_8_UNORM(4, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8A8_SNORM:
			GET_8_SNORM(4, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8A8_USCALED:
			GET_8_USCALED(4, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8A8_SSCALED:
			GET_8_SSCALED(4, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_R16_UNORM:
			GET_16_UNORM(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16_SNORM:
			GET_16_SNORM(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16_USCALED:
			GET_16_USCALED(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16_SSCALED:
			GET_16_SSCALED(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16_UNORM:
			GET_16_UNORM(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16_SNORM:
			GET_16_SNORM(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16_USCALED:
			GET_16_USCALED(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16_SSCALED:
			GET_16_SSCALED(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16_UNORM:
			GET_16_UNORM(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16_SNORM:
			GET_16_SNORM(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16_USCALED:
			GET_16_USCALED(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16_SSCALED:
			GET_16_SSCALED(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16A16_UNORM:
			GET_16_UNORM(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16A16_SNORM:
			GET_16_SNORM(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16A16_USCALED:
			GET_16_USCALED(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16A16_SSCALED:
			GET_16_SSCALED(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R32_SFLOAT:
			GET_32_SFLOAT(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R32G32_SFLOAT:
			GET_32_SFLOAT(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R32G32B32_SFLOAT:
			GET_32_SFLOAT(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
			GET_32_SFLOAT(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R64_SFLOAT:
			GET_64_SFLOAT(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R64G64_SFLOAT:
			GET_64_SFLOAT(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R64G64B64_SFLOAT:
			GET_64_SFLOAT(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
			GET_64_SFLOAT(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		{
			const uint8_t * restrict data = datav;
			color[0] = GET_4(*data, 4);
			color[1] = GET_4(*data, 0);
			color[2] = 0;
			color[3] = 1;
			return;
		}
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_4(*data, 0xC);
			color[1] = GET_4(*data, 0x8);
			color[2] = GET_4(*data, 0x4);
			color[3] = GET_4(*data, 0x0);
			return;
		}
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_4(*data, 0x4);
			color[1] = GET_4(*data, 0x8);
			color[2] = GET_4(*data, 0xC);
			color[3] = GET_4(*data, 0x0);
			return;
		}
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_4(*data, 0x8);
			color[1] = GET_4(*data, 0x4);
			color[2] = GET_4(*data, 0x0);
			color[3] = GET_4(*data, 0xC);
			return;
		}
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_4(*data, 0x0);
			color[1] = GET_4(*data, 0x4);
			color[2] = GET_4(*data, 0x8);
			color[3] = GET_4(*data, 0xC);
			return;
		}
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_5(*data, 0xB);
			color[1] = GET_6(*data, 0x5);
			color[2] = GET_5(*data, 0x0);
			color[3] = 1;
			return;
		}
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_5(*data, 0x0);
			color[1] = GET_6(*data, 0x5);
			color[2] = GET_5(*data, 0xB);
			color[3] = 1;
			return;
		}
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_5(*data, 0xB);
			color[1] = GET_5(*data, 0x6);
			color[2] = GET_5(*data, 0x1);
			color[3] = GET_1(*data, 0x0);
			return;
		}
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_5(*data, 0x1);
			color[1] = GET_5(*data, 0x6);
			color[2] = GET_5(*data, 0xB);
			color[3] = GET_1(*data, 0x0);
			return;
		}
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_5(*data, 0xA);
			color[1] = GET_5(*data, 0x5);
			color[2] = GET_5(*data, 0x0);
			color[3] = GET_1(*data, 0xF);
			return;
		}
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
		{
			const uint16_t * restrict data = datav;
			color[0] = GET_5(*data, 0x0);
			color[1] = GET_5(*data, 0x5);
			color[2] = GET_5(*data, 0xA);
			color[3] = GET_1(*data, 0xF);
			return;
		}
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
			break;
	}
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	color[3] = 1;

#undef GET_6
#undef GET_5
#undef GET_4
#undef GET_1
#undef GET_64_SFLOAT
#undef GET_32_SFLOAT
#undef GET_16_SSCALED
#undef GET_16_USCALED
#undef GET_16_SNORM
#undef GET_16_UNORM
#undef GET_8_SSCALED
#undef GET_8_USCALED
#undef GET_8_SNORM
#undef GET_8_UNORM
#undef GET_COLOR
}

void
jkg_set_vec4f(void * restrict datav,
              const float * restrict color,
              enum jkg_format format,
              enum jkg_color_mask mask)
{
#define SET_COLOR(count, r, g, b, a, type, scale) \
do \
{ \
	type * restrict data = datav; \
	if (count > 0 && (mask & JKG_COLOR_MASK_R)) \
		data[r] = color[0] * (scale); \
	if (count > 1 && (mask & JKG_COLOR_MASK_G)) \
		data[g] = color[1] * (scale); \
	if (count > 2 && (mask & JKG_COLOR_MASK_B)) \
		data[b] = color[2] * (scale); \
	if (count > 3 && (mask & JKG_COLOR_MASK_A)) \
		data[a] = color[3] * (scale); \
} while (0)

#define SET_8_UNORM(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, uint8_t, UINT8_MAX)

#define SET_8_SNORM(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, int8_t, INT8_MAX)

#define SET_8_USCALED(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, uint8_t, 1)

#define SET_8_SSCALED(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, int8_t, 1)

#define SET_16_UNORM(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, uint16_t, UINT16_MAX)

#define SET_16_SNORM(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, int16_t, INT16_MAX)

#define SET_16_USCALED(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, uint16_t, 1)

#define SET_16_SSCALED(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, int16_t, 1)

#define SET_32_SFLOAT(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, float, 1)

#define SET_64_SFLOAT(count, r, g, b, a) \
	SET_COLOR(count, r, g, b, a, double, 1)

#define SET_1(value, offset) (((uint32_t)((value) * 0x01) & 0x01) << (offset))
#define SET_4(value, offset) (((uint32_t)((value) * 0x0F) & 0x0F) << (offset))
#define SET_5(value, offset) (((uint32_t)((value) * 0x1F) & 0x1F) << (offset))
#define SET_6(value, offset) (((uint32_t)((value) * 0x3F) & 0x3F) << (offset))

	switch (format)
	{
		case JKG_FORMAT_R8_UNORM:
			SET_8_UNORM(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8_SNORM:
			SET_8_SNORM(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8_USCALED:
			SET_8_USCALED(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8_SSCALED:
			SET_8_SSCALED(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8_UNORM:
			SET_8_UNORM(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8_SNORM:
			SET_8_SNORM(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8_USCALED:
			SET_8_USCALED(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8_SSCALED:
			SET_8_SSCALED(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8_UNORM:
			SET_8_UNORM(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8_SNORM:
			SET_8_SNORM(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8_USCALED:
			SET_8_USCALED(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8_SSCALED:
			SET_8_SSCALED(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_B8G8R8_UNORM:
			SET_8_UNORM(3, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8_SNORM:
			SET_8_SNORM(3, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8_USCALED:
			SET_8_USCALED(3, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8_SSCALED:
			SET_8_SSCALED(3, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_R8G8B8A8_UNORM:
			SET_8_UNORM(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8A8_SNORM:
			SET_8_SNORM(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8A8_USCALED:
			SET_8_USCALED(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R8G8B8A8_SSCALED:
			SET_8_SSCALED(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_B8G8R8A8_UNORM:
			SET_8_UNORM(4, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8A8_SNORM:
			SET_8_SNORM(4, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8A8_USCALED:
			SET_8_USCALED(4, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_B8G8R8A8_SSCALED:
			SET_8_SSCALED(4, 2, 1, 0, 3);
			return;
		case JKG_FORMAT_R16_UNORM:
			SET_16_UNORM(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16_SNORM:
			SET_16_SNORM(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16_USCALED:
			SET_16_USCALED(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16_SSCALED:
			SET_16_SSCALED(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16_UNORM:
			SET_16_UNORM(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16_SNORM:
			SET_16_SNORM(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16_USCALED:
			SET_16_USCALED(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16_SSCALED:
			SET_16_SSCALED(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16_UNORM:
			SET_16_UNORM(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16_SNORM:
			SET_16_SNORM(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16_USCALED:
			SET_16_USCALED(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16_SSCALED:
			SET_16_SSCALED(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16A16_UNORM:
			SET_16_UNORM(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16A16_SNORM:
			SET_16_SNORM(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16A16_USCALED:
			SET_16_USCALED(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R16G16B16A16_SSCALED:
			SET_16_SSCALED(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R32_SFLOAT:
			SET_32_SFLOAT(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R32G32_SFLOAT:
			SET_32_SFLOAT(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R32G32B32_SFLOAT:
			SET_32_SFLOAT(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
			SET_32_SFLOAT(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R64_SFLOAT:
			SET_64_SFLOAT(1, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R64G64_SFLOAT:
			SET_64_SFLOAT(2, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R64G64B64_SFLOAT:
			SET_64_SFLOAT(3, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
			SET_64_SFLOAT(4, 0, 1, 2, 3);
			return;
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		{
			uint8_t *data = datav;
			*data = SET_4(color[0], 4)
			      | SET_4(color[1], 0);
			return;
		}
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_4(color[0], 0xC)
			      | SET_4(color[1], 0x8)
			      | SET_4(color[2], 0x4)
			      | SET_4(color[3], 0x0);
			return;
		}
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_4(color[0], 0x4)
			      | SET_4(color[1], 0x8)
			      | SET_4(color[2], 0xC)
			      | SET_4(color[3], 0x0);
			return;
		}
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_4(color[0], 0x8)
			      | SET_4(color[1], 0x4)
			      | SET_4(color[2], 0x0)
			      | SET_4(color[3], 0xC);
			return;
		}
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_4(color[0], 0x0)
			      | SET_4(color[1], 0x4)
			      | SET_4(color[2], 0x8)
			      | SET_4(color[3], 0xC);
			return;
		}
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_5(color[0], 0xB)
			      | SET_6(color[1], 0x5)
			      | SET_5(color[2], 0x0);
			return;
		}
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_5(color[0], 0x0)
			      | SET_6(color[1], 0x5)
			      | SET_5(color[2], 0xB);
			return;
		}
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_5(color[0], 0xB)
			      | SET_5(color[1], 0x6)
			      | SET_5(color[2], 0x1)
			      | SET_1(color[3], 0x0);
			return;
		}
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_5(color[0], 0x1)
			      | SET_5(color[1], 0x6)
			      | SET_5(color[2], 0xB)
			      | SET_1(color[3], 0x0);
			return;
		}
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_5(color[0], 0xA)
			      | SET_5(color[1], 0x5)
			      | SET_5(color[2], 0x0)
			      | SET_1(color[3], 0xF);
			return;
		}
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
		{
			uint16_t *data = datav;
			*data = SET_5(color[0], 0x0)
			      | SET_5(color[1], 0x5)
			      | SET_5(color[2], 0xA)
			      | SET_1(color[3], 0xF);
			return;
		}
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
			return;
	}

#undef SET_6
#undef SET_5
#undef SET_4
#undef SET_1
#undef SET_64_SFLOAT
#undef SET_32_SFLOAT
#undef SET_16_SSCALED
#undef SET_16_USCALED
#undef SET_16_SNORM
#undef SET_16_UNORM
#undef SET_8_SSCALED
#undef SET_8_USCALED
#undef SET_8_SNORM
#undef SET_8_UNORM
#undef SET_COLOR
}

void
jkg_clear_vec4u(void *datav,
                uint32_t width,
                uint32_t height,
                enum jkg_format format,
                enum jkg_color_mask color_mask,
                int32_t rect[4],
                const uint32_t color[4])
{
#define CLEAR_COLOR(ncomp, r, g, b, a, type) \
do \
{ \
	uint32_t stride = width * (ncomp); \
	type *data = datav; \
	data += rect[2] * stride; \
	data += rect[0] * (ncomp); \
	type vr = color[0]; \
	type vg = color[1]; \
	type vb = color[2]; \
	type va = color[3]; \
	if (color_mask == (JKG_COLOR_MASK_R | JKG_COLOR_MASK_G | JKG_COLOR_MASK_B | JKG_COLOR_MASK_A)) \
	{ \
		for (int32_t y = rect[2]; y < rect[3]; ++y) \
		{ \
			type * restrict line = data; \
			for (int32_t x = rect[0]; x < rect[1]; ++x) \
			{ \
				if ((ncomp) > 0) \
					line[r] = vr; \
				if ((ncomp) > 1) \
					line[g] = vg; \
				if ((ncomp) > 2) \
					line[b] = vb; \
				if ((ncomp) > 3) \
					line[a] = va; \
				line += (ncomp); \
			} \
			data += stride; \
		} \
	} \
	else \
	{ \
		if ((ncomp) > 0 && (color_mask & JKG_COLOR_MASK_R)) \
		{ \
			type *comp_data = data; \
			for (int32_t y = rect[2]; y < rect[3]; ++y) \
			{ \
				type * restrict line = comp_data; \
				for (int32_t x = rect[0]; x < rect[1]; ++x) \
				{ \
					line[r] = vr; \
					line += (ncomp); \
				} \
				comp_data += stride; \
			} \
		} \
		if ((ncomp) > 1 && (color_mask & JKG_COLOR_MASK_G)) \
		{ \
			type *comp_data = data; \
			for (int32_t y = rect[2]; y < rect[3]; ++y) \
			{ \
				type * restrict line = comp_data; \
				for (int32_t x = rect[0]; x < rect[1]; ++x) \
				{ \
					line[g] = vg; \
					line += (ncomp); \
				} \
				comp_data += stride; \
			} \
		} \
		if ((ncomp) > 2 && (color_mask & JKG_COLOR_MASK_B)) \
		{ \
			type *comp_data = data; \
			for (int32_t y = rect[2]; y < rect[3]; ++y) \
			{ \
				type * restrict line = comp_data; \
				for (int32_t x = rect[0]; x < rect[1]; ++x) \
				{ \
					line[b] = vb; \
					line += (ncomp); \
				} \
				comp_data += stride; \
			} \
		} \
		if ((ncomp > 3) && (color_mask & JKG_COLOR_MASK_A)) \
		{ \
			type *comp_data = data; \
			for (int32_t y = rect[2]; y < rect[3]; ++y) \
			{ \
				type * restrict line = comp_data; \
				for (int32_t x = rect[0]; x < rect[1]; ++x) \
				{ \
					line[a] = va; \
					line += (ncomp); \
				} \
				comp_data += stride; \
			} \
		} \
	} \
} while (0)

	(void)height;
	switch (format)
	{
		case JKG_FORMAT_R8_UINT:
			CLEAR_COLOR(1, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8_SINT:
			CLEAR_COLOR(1, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8_UINT:
			CLEAR_COLOR(2, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8_SINT:
			CLEAR_COLOR(2, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8B8_UINT:
			CLEAR_COLOR(3, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8B8_SINT:
			CLEAR_COLOR(3, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_B8G8R8_UINT:
			CLEAR_COLOR(3, 2, 1, 0, 3, uint8_t);
			return;
		case JKG_FORMAT_B8G8R8_SINT:
			CLEAR_COLOR(3, 2, 1, 0, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8B8A8_UINT:
			CLEAR_COLOR(4, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8B8A8_SINT:
			CLEAR_COLOR(4, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_B8G8R8A8_UINT:
			CLEAR_COLOR(4, 2, 1, 0, 3, uint8_t);
			return;
		case JKG_FORMAT_B8G8R8A8_SINT:
			CLEAR_COLOR(4, 2, 1, 0, 3, int8_t);
			return;
		case JKG_FORMAT_R16_UINT:
			CLEAR_COLOR(1, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16_SINT:
			CLEAR_COLOR(1, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16_UINT:
			CLEAR_COLOR(2, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16_SINT:
			CLEAR_COLOR(2, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16B16_UINT:
			CLEAR_COLOR(3, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16B16_SINT:
			CLEAR_COLOR(3, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16B16A16_UINT:
			CLEAR_COLOR(4, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16B16A16_SINT:
			CLEAR_COLOR(4, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R32_UINT:
			CLEAR_COLOR(1, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32_SINT:
			CLEAR_COLOR(1, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32_UINT:
			CLEAR_COLOR(2, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32_SINT:
			CLEAR_COLOR(2, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32B32_UINT:
			CLEAR_COLOR(3, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32B32_SINT:
			CLEAR_COLOR(3, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32B32A32_UINT:
			CLEAR_COLOR(4, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32B32A32_SINT:
			CLEAR_COLOR(4, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R64_UINT:
			CLEAR_COLOR(1, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64_SINT:
			CLEAR_COLOR(1, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64_UINT:
			CLEAR_COLOR(2, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64_SINT:
			CLEAR_COLOR(2, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64B64_UINT:
			CLEAR_COLOR(3, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64B64_SINT:
			CLEAR_COLOR(3, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64B64A64_UINT:
			CLEAR_COLOR(4, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64B64A64_SINT:
			CLEAR_COLOR(4, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			return;
	}

#undef CLEAR_COLOR
}

void
jkg_get_vec4u(uint32_t * restrict color,
              const void * restrict datav,
              enum jkg_format format)
{
#define GET_COLOR(count, r, g, b, a, type) \
do \
{ \
	const type * restrict data = datav; \
	if (count > 0) \
		color[r] = data[0]; \
	else \
		color[r] = 0; \
	if (count > 1) \
		color[g] = data[1]; \
	else \
		color[g] = 0; \
	if (count > 2) \
		color[b] = data[2]; \
	else \
		color[b] = 0; \
	if (count > 3) \
		color[a] = data[3]; \
	else \
		color[a] = 1; \
} while (0)

	switch (format)
	{
		case JKG_FORMAT_R8_UINT:
			GET_COLOR(1, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8_SINT:
			GET_COLOR(1, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8_UINT:
			GET_COLOR(2, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8_SINT:
			GET_COLOR(2, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8B8_UINT:
			GET_COLOR(3, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8B8_SINT:
			GET_COLOR(3, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_B8G8R8_UINT:
			GET_COLOR(3, 2, 1, 0, 3, uint8_t);
			return;
		case JKG_FORMAT_B8G8R8_SINT:
			GET_COLOR(3, 2, 1, 0, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8B8A8_UINT:
			GET_COLOR(4, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8B8A8_SINT:
			GET_COLOR(4, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_B8G8R8A8_UINT:
			GET_COLOR(4, 2, 1, 0, 3, uint8_t);
			return;
		case JKG_FORMAT_B8G8R8A8_SINT:
			GET_COLOR(4, 2, 1, 0, 3, int8_t);
			return;
		case JKG_FORMAT_R16_UINT:
			GET_COLOR(1, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16_SINT:
			GET_COLOR(1, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16_UINT:
			GET_COLOR(2, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16_SINT:
			GET_COLOR(2, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16B16_UINT:
			GET_COLOR(3, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16B16_SINT:
			GET_COLOR(3, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16B16A16_UINT:
			GET_COLOR(4, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16B16A16_SINT:
			GET_COLOR(4, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R32_UINT:
			GET_COLOR(1, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32_SINT:
			GET_COLOR(1, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32_UINT:
			GET_COLOR(2, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32_SINT:
			GET_COLOR(2, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32B32_UINT:
			GET_COLOR(3, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32B32_SINT:
			GET_COLOR(3, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32B32A32_UINT:
			GET_COLOR(4, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32B32A32_SINT:
			GET_COLOR(4, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R64_UINT:
			GET_COLOR(1, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64_SINT:
			GET_COLOR(1, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64_UINT:
			GET_COLOR(2, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64_SINT:
			GET_COLOR(2, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64B64_UINT:
			GET_COLOR(3, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64B64_SINT:
			GET_COLOR(3, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64B64A64_UINT:
			GET_COLOR(4, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64B64A64_SINT:
			GET_COLOR(4, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			return;
	}

#undef GET_COLOR
}

void
jkg_set_vec4u(void * restrict datav,
              const uint32_t * restrict color,
              enum jkg_format format,
              enum jkg_color_mask mask)
{
#define SET_COLOR(count, r, g, b, a, type) \
do \
{ \
	type * restrict data = datav; \
	if (count > 0 && (mask & JKG_COLOR_MASK_R)) \
		data[r] = color[0]; \
	if (count > 1 && (mask & JKG_COLOR_MASK_G)) \
		data[g] = color[1]; \
	if (count > 2 && (mask & JKG_COLOR_MASK_B)) \
		data[b] = color[2]; \
	if (count > 3 && (mask & JKG_COLOR_MASK_A)) \
		data[a] = color[3]; \
} while (0)

	switch (format)
	{
		case JKG_FORMAT_R8_UINT:
			SET_COLOR(1, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8_SINT:
			SET_COLOR(1, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8_UINT:
			SET_COLOR(2, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8_SINT:
			SET_COLOR(2, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8B8_UINT:
			SET_COLOR(3, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8B8_SINT:
			SET_COLOR(3, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_B8G8R8_UINT:
			SET_COLOR(3, 2, 1, 0, 3, uint8_t);
			return;
		case JKG_FORMAT_B8G8R8_SINT:
			SET_COLOR(3, 2, 1, 0, 3, int8_t);
			return;
		case JKG_FORMAT_R8G8B8A8_UINT:
			SET_COLOR(4, 0, 1, 2, 3, uint8_t);
			return;
		case JKG_FORMAT_R8G8B8A8_SINT:
			SET_COLOR(4, 0, 1, 2, 3, int8_t);
			return;
		case JKG_FORMAT_B8G8R8A8_UINT:
			SET_COLOR(4, 2, 1, 0, 3, uint8_t);
			return;
		case JKG_FORMAT_B8G8R8A8_SINT:
			SET_COLOR(4, 2, 1, 0, 3, int8_t);
			return;
		case JKG_FORMAT_R16_UINT:
			SET_COLOR(1, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16_SINT:
			SET_COLOR(1, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16_UINT:
			SET_COLOR(2, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16_SINT:
			SET_COLOR(2, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16B16_UINT:
			SET_COLOR(3, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16B16_SINT:
			SET_COLOR(3, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R16G16B16A16_UINT:
			SET_COLOR(4, 0, 1, 2, 3, uint16_t);
			return;
		case JKG_FORMAT_R16G16B16A16_SINT:
			SET_COLOR(4, 0, 1, 2, 3, int16_t);
			return;
		case JKG_FORMAT_R32_UINT:
			SET_COLOR(1, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32_SINT:
			SET_COLOR(1, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32_UINT:
			SET_COLOR(2, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32_SINT:
			SET_COLOR(2, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32B32_UINT:
			SET_COLOR(3, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32B32_SINT:
			SET_COLOR(3, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R32G32B32A32_UINT:
			SET_COLOR(4, 0, 1, 2, 3, uint32_t);
			return;
		case JKG_FORMAT_R32G32B32A32_SINT:
			SET_COLOR(4, 0, 1, 2, 3, int32_t);
			return;
		case JKG_FORMAT_R64_UINT:
			SET_COLOR(1, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64_SINT:
			SET_COLOR(1, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64_UINT:
			SET_COLOR(2, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64_SINT:
			SET_COLOR(2, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64B64_UINT:
			SET_COLOR(3, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64B64_SINT:
			SET_COLOR(3, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_R64G64B64A64_UINT:
			SET_COLOR(4, 0, 1, 2, 3, uint64_t);
			return;
		case JKG_FORMAT_R64G64B64A64_SINT:
			SET_COLOR(4, 0, 1, 2, 3, int64_t);
			return;
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			return;
	}

#undef SET_COLOR
}

void
jkg_clear_depth(void *datav,
                uint32_t width,
                uint32_t height,
                enum jkg_format format,
                int32_t rect[4],
                float depth)
{
	(void)height;
	switch (format)
	{
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		{
			uint32_t v32 = (uint32_t)(depth * 0xFFFFFF) & 0xFFFFFF;
			uint32_t *data = datav;
			data += rect[2] * width;
			data += rect[0];
			for (int32_t y = rect[2]; y < rect[3]; ++y)
			{
				uint32_t *line = data;
				for (int32_t x = rect[0]; x < rect[1]; ++x)
				{
					*line = (*line & 0xFF000000) | v32;
					line++;
				}
				data += width;
			}
			break;
		}
		case JKG_FORMAT_D32_SFLOAT:
		{
			float *data = datav;
			data += rect[2] * width;
			data += rect[0];
			for (int32_t y = rect[2]; y < rect[3]; ++y)
			{
				float *line = data;
				for (int32_t x = rect[0]; x < rect[1]; ++x)
				{
					*line = depth;
					line++;
				}
				data += width;
			}
			break;
		}
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			break;
	}
}

float
jkg_get_depth(const void *data,
              enum jkg_format format)
{
	switch (format)
	{
		case JKG_FORMAT_D24_UNORM_S8_UINT:
			return *((uint32_t*)data) * (1.0 / 0xFFFFFF);
		case JKG_FORMAT_D32_SFLOAT:
			return *(float*)data;
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			break;
	}
	return 0;
}

void
jkg_set_depth(void *datav,
              float value,
              enum jkg_format format)
{
	switch (format)
	{
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		{
			uint32_t *data = datav;
			*data = (*data & 0xFF000000) | ((uint32_t)(value * 0xFFFFFF) & 0xFFFFFF);
			return;
		}
		case JKG_FORMAT_D32_SFLOAT:
		{
			float *data = datav;
			*data = value;
			return;
		}
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			break;
	}
}

void
jkg_clear_stencil(void *datav,
                  uint32_t width,
                  uint32_t height,
                  enum jkg_format format,
                  int32_t rect[4],
                  uint8_t stencil)
{
	(void)height;
	switch (format)
	{
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		{
			uint32_t v32 = (uint32_t)stencil << 24;
			uint32_t *data = datav;
			data += rect[2] * width;
			data += rect[0];
			for (int32_t y = rect[2]; y < rect[3]; ++y)
			{
				uint32_t *line = data;
				for (int32_t x = rect[0]; x < rect[1]; ++x)
				{
					*line = (*line & 0x00FFFFFF) | v32;
					line++;
				}
				data += width;
			}
			break;
		}
		case JKG_FORMAT_S8_UINT:
		{
			uint8_t *data = datav;
			data += rect[2] * width;
			data += rect[0];
			for (int32_t y = rect[2]; y < rect[3]; ++y)
			{
				uint8_t *line = data;
				for (int32_t x = rect[0]; x < rect[1]; ++x)
				{
					*line = stencil;
					line++;
				}
				data += width;
			}
			break;
		}
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			break;
	}
}

uint8_t
jkg_get_stencil(const void *data,
                enum jkg_format format)
{
	switch (format)
	{
		case JKG_FORMAT_D24_UNORM_S8_UINT:
			return (*((uint32_t*)data) & 0xFF000000) >> 24;
		case JKG_FORMAT_S8_UINT:
			return *(uint8_t*)data;
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			break;
	}
	return 0;
}

void
jkg_set_stencil(void *datav,
                uint8_t value,
                enum jkg_format format)
{
	switch (format)
	{
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		{
			uint32_t *data = datav;
			*data = (*data & 0x00FFFFFF) | (value << 24);
			return;
		}
		case JKG_FORMAT_S8_UINT:
		{
			uint8_t *data = datav;
			*data = value;
			return;
		}
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			break;
	}
}

void
jkg_clear_depth_stencil(void *datav,
                        uint32_t width,
                        uint32_t height,
                        enum jkg_format format,
                        int32_t rect[4],
                        float depth,
                        uint8_t stencil)
{
	(void)height;
	switch (format)
	{
		case JKG_FORMAT_D24_UNORM_S8_UINT:
		{
			uint32_t v32 = ((uint32_t)(depth * 0xFFFFFF) & 0xFFFFFF) | (stencil << 24);
			uint32_t *data = datav;
			data += rect[2] * width;
			data += rect[0];
			for (int32_t y = rect[2]; y < rect[3]; ++y)
			{
				uint32_t *line = data;
				for (int32_t x = rect[0]; x < rect[1]; ++x)
				{
					*line = v32;
					line++;
				}
				data += width;
			}
			break;
		}
		case JKG_FORMAT_UNKNOWN:
		case JKG_FORMAT_R8_UINT:
		case JKG_FORMAT_R8_SINT:
		case JKG_FORMAT_R8_UNORM:
		case JKG_FORMAT_R8_SNORM:
		case JKG_FORMAT_R8_USCALED:
		case JKG_FORMAT_R8_SSCALED:
		case JKG_FORMAT_R8G8_UINT:
		case JKG_FORMAT_R8G8_SINT:
		case JKG_FORMAT_R8G8_UNORM:
		case JKG_FORMAT_R8G8_SNORM:
		case JKG_FORMAT_R8G8_USCALED:
		case JKG_FORMAT_R8G8_SSCALED:
		case JKG_FORMAT_R8G8B8_UINT:
		case JKG_FORMAT_R8G8B8_SINT:
		case JKG_FORMAT_R8G8B8_UNORM:
		case JKG_FORMAT_R8G8B8_SNORM:
		case JKG_FORMAT_R8G8B8_USCALED:
		case JKG_FORMAT_R8G8B8_SSCALED:
		case JKG_FORMAT_B8G8R8_UINT:
		case JKG_FORMAT_B8G8R8_SINT:
		case JKG_FORMAT_B8G8R8_UNORM:
		case JKG_FORMAT_B8G8R8_SNORM:
		case JKG_FORMAT_B8G8R8_USCALED:
		case JKG_FORMAT_B8G8R8_SSCALED:
		case JKG_FORMAT_R8G8B8A8_UINT:
		case JKG_FORMAT_R8G8B8A8_SINT:
		case JKG_FORMAT_R8G8B8A8_UNORM:
		case JKG_FORMAT_R8G8B8A8_SNORM:
		case JKG_FORMAT_R8G8B8A8_USCALED:
		case JKG_FORMAT_R8G8B8A8_SSCALED:
		case JKG_FORMAT_B8G8R8A8_UINT:
		case JKG_FORMAT_B8G8R8A8_SINT:
		case JKG_FORMAT_B8G8R8A8_UNORM:
		case JKG_FORMAT_B8G8R8A8_SNORM:
		case JKG_FORMAT_B8G8R8A8_USCALED:
		case JKG_FORMAT_B8G8R8A8_SSCALED:
		case JKG_FORMAT_R16_UINT:
		case JKG_FORMAT_R16_SINT:
		case JKG_FORMAT_R16_UNORM:
		case JKG_FORMAT_R16_SNORM:
		case JKG_FORMAT_R16_USCALED:
		case JKG_FORMAT_R16_SSCALED:
		case JKG_FORMAT_R16_SFLOAT:
		case JKG_FORMAT_R16G16_UINT:
		case JKG_FORMAT_R16G16_SINT:
		case JKG_FORMAT_R16G16_UNORM:
		case JKG_FORMAT_R16G16_SNORM:
		case JKG_FORMAT_R16G16_USCALED:
		case JKG_FORMAT_R16G16_SSCALED:
		case JKG_FORMAT_R16G16_SFLOAT:
		case JKG_FORMAT_R16G16B16_UINT:
		case JKG_FORMAT_R16G16B16_SINT:
		case JKG_FORMAT_R16G16B16_UNORM:
		case JKG_FORMAT_R16G16B16_SNORM:
		case JKG_FORMAT_R16G16B16_USCALED:
		case JKG_FORMAT_R16G16B16_SSCALED:
		case JKG_FORMAT_R16G16B16_SFLOAT:
		case JKG_FORMAT_R16G16B16A16_UINT:
		case JKG_FORMAT_R16G16B16A16_SINT:
		case JKG_FORMAT_R16G16B16A16_UNORM:
		case JKG_FORMAT_R16G16B16A16_SNORM:
		case JKG_FORMAT_R16G16B16A16_USCALED:
		case JKG_FORMAT_R16G16B16A16_SSCALED:
		case JKG_FORMAT_R16G16B16A16_SFLOAT:
		case JKG_FORMAT_R32_UINT:
		case JKG_FORMAT_R32_SINT:
		case JKG_FORMAT_R32_SFLOAT:
		case JKG_FORMAT_R32G32_UINT:
		case JKG_FORMAT_R32G32_SINT:
		case JKG_FORMAT_R32G32_SFLOAT:
		case JKG_FORMAT_R32G32B32_UINT:
		case JKG_FORMAT_R32G32B32_SINT:
		case JKG_FORMAT_R32G32B32_SFLOAT:
		case JKG_FORMAT_R32G32B32A32_UINT:
		case JKG_FORMAT_R32G32B32A32_SINT:
		case JKG_FORMAT_R32G32B32A32_SFLOAT:
		case JKG_FORMAT_R64_UINT:
		case JKG_FORMAT_R64_SINT:
		case JKG_FORMAT_R64_SFLOAT:
		case JKG_FORMAT_R64G64_UINT:
		case JKG_FORMAT_R64G64_SINT:
		case JKG_FORMAT_R64G64_SFLOAT:
		case JKG_FORMAT_R64G64B64_UINT:
		case JKG_FORMAT_R64G64B64_SINT:
		case JKG_FORMAT_R64G64B64_SFLOAT:
		case JKG_FORMAT_R64G64B64A64_UINT:
		case JKG_FORMAT_R64G64B64A64_SINT:
		case JKG_FORMAT_R64G64B64A64_SFLOAT:
		case JKG_FORMAT_D32_SFLOAT:
		case JKG_FORMAT_S8_UINT:
		case JKG_FORMAT_BC1_RGB_UNORM_BLOCK:
		case JKG_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case JKG_FORMAT_BC2_UNORM_BLOCK:
		case JKG_FORMAT_BC3_UNORM_BLOCK:
		case JKG_FORMAT_BC4_UNORM_BLOCK:
		case JKG_FORMAT_BC4_SNORM_BLOCK:
		case JKG_FORMAT_BC5_UNORM_BLOCK:
		case JKG_FORMAT_BC5_SNORM_BLOCK:
		case JKG_FORMAT_R4G4_UNORM_PACK8:
		case JKG_FORMAT_R4G4B4A4_UNORM_PACK16:
		case JKG_FORMAT_B4G4R4A4_UNORM_PACK16:
		case JKG_FORMAT_A4R4G4B4_UNORM_PACK16:
		case JKG_FORMAT_A4B4G4R4_UNORM_PACK16:
		case JKG_FORMAT_R5G6B5_UNORM_PACK16:
		case JKG_FORMAT_B5G6R5_UNORM_PACK16:
		case JKG_FORMAT_R5G5B5A1_UNORM_PACK16:
		case JKG_FORMAT_B5G5R5A1_UNORM_PACK16:
		case JKG_FORMAT_A1R5G5B5_UNORM_PACK16:
		case JKG_FORMAT_A1B5G5R5_UNORM_PACK16:
			break;
	}
}
