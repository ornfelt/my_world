#include "jkg.h"

static const struct
{
	enum jkg_format_type type;
	uint8_t stride;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
	uint8_t depth;
	uint8_t stencil;
} format_properties[] =
{
	[JKG_FORMAT_UNKNOWN] =
	{
		.type = JKG_FORMAT_TYPE_UNKNOWN,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 1,
		.red = 8,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 1,
		.red = 8,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 1,
		.red = 8,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 1,
		.red = 8,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 1,
		.red = 8,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 1,
		.red = 8,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 2,
		.red = 8,
		.green = 8,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 2,
		.red = 8,
		.green = 8,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 8,
		.green = 8,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 2,
		.red = 8,
		.green = 8,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 2,
		.red = 8,
		.green = 8,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 2,
		.red = 8,
		.green = 8,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 3,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8A8_UINT]
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8A8_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8A8_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8A8_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8A8_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R8G8B8A8_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8A8_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8A8_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8A8_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8A8_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8A8_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B8G8R8A8_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 4,
		.red = 8,
		.green = 8,
		.blue = 8,
		.alpha = 8,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 2,
		.red = 16,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 2,
		.red = 16,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 16,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 2,
		.red = 16,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 2,
		.red = 16,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 2,
		.red = 16,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 2,
		.red = 16,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 4,
		.red = 16,
		.green = 16,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 4,
		.red = 16,
		.green = 16,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 4,
		.red = 16,
		.green = 16,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 4,
		.red = 16,
		.green = 16,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 4,
		.red = 16,
		.green = 16,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 4,
		.red = 16,
		.green = 16,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 4,
		.red = 16,
		.green = 16,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 6,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 6,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 6,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 6,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 6,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 6,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 6,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16A16_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 8,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 16,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16A16_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 8,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 16,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16A16_UNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 8,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 16,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16A16_SNORM] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM,
		.stride = 8,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 16,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16A16_USCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_USCALED,
		.stride = 8,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 16,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16A16_SSCALED] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SSCALED,
		.stride = 8,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 16,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R16G16B16A16_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 8,
		.red = 16,
		.green = 16,
		.blue = 16,
		.alpha = 16,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 4,
		.red = 32,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 4,
		.red = 32,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 4,
		.red = 32,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 8,
		.red = 32,
		.green = 32,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 8,
		.red = 32,
		.green = 32,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 8,
		.red = 32,
		.green = 32,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32B32_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 12,
		.red = 32,
		.green = 32,
		.blue = 32,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32B32_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 12,
		.red = 32,
		.green = 32,
		.blue = 32,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32B32_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 12,
		.red = 32,
		.green = 32,
		.blue = 32,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32B32A32_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 16,
		.red = 32,
		.green = 32,
		.blue = 32,
		.alpha = 32,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32B32A32_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 16,
		.red = 32,
		.green = 32,
		.blue = 32,
		.alpha = 32,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R32G32B32A32_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 16,
		.red = 32,
		.green = 32,
		.blue = 32,
		.alpha = 32,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 8,
		.red = 64,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 8,
		.red = 64,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 8,
		.red = 64,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 16,
		.red = 64,
		.green = 64,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 16,
		.red = 64,
		.green = 64,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 16,
		.red = 64,
		.green = 64,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64B64_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 24,
		.red = 64,
		.green = 64,
		.blue = 64,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64B64_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 24,
		.red = 64,
		.green = 64,
		.blue = 64,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64B64_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 24,
		.red = 64,
		.green = 64,
		.blue = 64,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64B64A64_UINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UINT,
		.stride = 32,
		.red = 64,
		.green = 64,
		.blue = 64,
		.alpha = 64,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64B64A64_SINT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SINT,
		.stride = 32,
		.red = 64,
		.green = 64,
		.blue = 64,
		.alpha = 64,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R64G64B64A64_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SFLOAT,
		.stride = 32,
		.red = 64,
		.green = 64,
		.blue = 64,
		.alpha = 64,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_D24_UNORM_S8_UINT] =
	{
		.type = JKG_FORMAT_TYPE_DEPTH_UNORM_STENCIL_UINT,
		.stride = 4,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 24,
		.stencil = 8,
	},
	[JKG_FORMAT_D32_SFLOAT] =
	{
		.type = JKG_FORMAT_TYPE_DEPTH_SFLOAT,
		.stride = 4,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 32,
		.stencil = 0,
	},
	[JKG_FORMAT_S8_UINT] =
	{
		.type = JKG_FORMAT_TYPE_STENCIL_UINT,
		.stride = 1,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 8,
	},
	[JKG_FORMAT_BC1_RGB_UNORM_BLOCK] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_BC1_RGBA_UNORM_BLOCK] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_BC2_UNORM_BLOCK] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_BC3_UNORM_BLOCK] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_BC4_UNORM_BLOCK] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_BC4_SNORM_BLOCK] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM_BLOCK,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_BC5_UNORM_BLOCK] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM_BLOCK,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_BC5_SNORM_BLOCK] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_SNORM_BLOCK,
		.stride = 0,
		.red = 0,
		.green = 0,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R4G4_UNORM_PACK8] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 1,
		.red = 4,
		.green = 4,
		.blue = 0,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R4G4B4A4_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 4,
		.green = 4,
		.blue = 4,
		.alpha = 4,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B4G4R4A4_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 4,
		.green = 4,
		.blue = 4,
		.alpha = 4,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_A4R4G4B4_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 4,
		.green = 4,
		.blue = 4,
		.alpha = 4,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_A4B4G4R4_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 4,
		.green = 4,
		.blue = 4,
		.alpha = 4,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R5G6B5_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 5,
		.green = 6,
		.blue = 5,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B5G6R5_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 5,
		.green = 6,
		.blue = 5,
		.alpha = 0,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_R5G5B5A1_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 5,
		.green = 5,
		.blue = 5,
		.alpha = 1,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_B5G5R5A1_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 5,
		.green = 5,
		.blue = 5,
		.alpha = 1,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_A1R5G5B5_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 5,
		.green = 5,
		.blue = 5,
		.alpha = 1,
		.depth = 0,
		.stencil = 0,
	},
	[JKG_FORMAT_A1B5G5R5_UNORM_PACK16] =
	{
		.type = JKG_FORMAT_TYPE_COLOR_UNORM,
		.stride = 2,
		.red = 5,
		.green = 5,
		.blue = 5,
		.alpha = 1,
		.depth = 0,
		.stencil = 0,
	},
};

enum jkg_format_type
jkg_get_format_type(enum jkg_format format)
{
	return format_properties[format].type;
}

uint32_t
jkg_get_stride(enum jkg_format format)
{
	return format_properties[format].stride;
}

uint32_t
jkg_get_red_size(enum jkg_format format)
{
	return format_properties[format].red;
}

uint32_t
jkg_get_green_size(enum jkg_format format)
{
	return format_properties[format].green;
}

uint32_t
jkg_get_blue_size(enum jkg_format format)
{
	return format_properties[format].blue;
}

uint32_t
jkg_get_alpha_size(enum jkg_format format)
{
	return format_properties[format].alpha;
}

uint32_t
jkg_get_depth_size(enum jkg_format format)
{
	return format_properties[format].depth;
}

uint32_t
jkg_get_stencil_size(enum jkg_format format)
{
	return format_properties[format].stencil;
}
