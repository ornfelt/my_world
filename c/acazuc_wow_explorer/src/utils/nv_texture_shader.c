#include "shaders.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#define GL_NONE                                      0x0000
#define GL_TEXTURE_1D                                0x0DE0
#define GL_TEXTURE_2D                                0x0DE1
#define GL_TEXTURE0                                  0x84C0
#define GL_TEXTURE_RECTANGLE_NV                      0x84F5
#define GL_TEXTURE_CUBE_MAP_ARB                      0x8513
#define GL_UNSIGNED_IDENTITY_NV                      0x8536
#define GL_EXPAND_NORMAL_NV                          0x8538
#define GL_OFFSET_TEXTURE_RECTANGLE_NV               0x864C
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV         0x864D
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV          0x864E
#define GL_OFFSET_TEXTURE_2D_SCALE_NV                0x86E2
#define GL_PASS_THROUGH_NV                           0x86E6
#define GL_CULL_FRAGMENT_NV                          0x86E7
#define GL_OFFSET_TEXTURE_2D_NV                      0x86E8
#define GL_DEPENDENT_AR_TEXTURE_2D_NV                0x86E9
#define GL_DEPENDENT_GB_TEXTURE_2D_NV                0x86EA
#define GL_DOT_PRODUCT_NV                            0x86EC
#define GL_DOT_PRODUCT_DEPTH_REPLACE_NV              0x86ED
#define GL_DOT_PRODUCT_TEXTURE_2D_NV                 0x86EE
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV           0x86F0
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV           0x86F1
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV           0x86F2
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV 0x86F3

struct texture_unit
{
	uint32_t operation;
	uint8_t cull_modes[4];
	uint32_t mapping;
	uint32_t previous_texture;
};

struct texture_shader
{
	uint32_t magic;
	struct texture_unit units[4];
};

__attribute__((format(printf, 3, 4)))
static void append(char **buffer, size_t *buffer_size, const char *fmt, ...)
{
	va_list ap;

	if (*buffer_size <= 1)
		return;
	va_start(ap, fmt);
	size_t len = vsnprintf(*buffer, *buffer_size, fmt, ap);
	if (len >= *buffer_size)
		len = *buffer_size - 1;
	*buffer += len;
	*buffer_size -= len;
	va_end(ap);
}

static void decode_texture_operation(char **buffer, size_t *buffer_size, uint32_t op)
{
	switch (op)
	{
		case GL_NONE:
			append(buffer, buffer_size, "NONE");
			return;
		case GL_TEXTURE_1D:
			append(buffer, buffer_size, "TEXTURE_1D");
			return;
		case GL_TEXTURE_2D:
			append(buffer, buffer_size, "TEXTURE_2D");
			return;
		case GL_TEXTURE_RECTANGLE_NV:
			append(buffer, buffer_size, "TEXTURE_RECTANGLE_NV");
			return;
		case GL_TEXTURE_CUBE_MAP_ARB:
			append(buffer, buffer_size, "TEXTURE_CUBE_MAP_ARB");
			return;
		case GL_PASS_THROUGH_NV:
			append(buffer, buffer_size, "PASS_THROUGH_NV");
			return;
		case GL_CULL_FRAGMENT_NV:
			append(buffer, buffer_size, "CULL_FRAGMENT_NV");
			return;
		case GL_OFFSET_TEXTURE_2D_NV:
			append(buffer, buffer_size, "OFFSET_TEXTURE_2D_NV");
			return;
		case GL_OFFSET_TEXTURE_2D_SCALE_NV:
			append(buffer, buffer_size, "OFFSET_TEXTURE_2D_SCALE_NV");
			return;
		case GL_OFFSET_TEXTURE_RECTANGLE_NV:
			append(buffer, buffer_size, "OFFSET_TEXTURE_RECTANGLE_NV");
			return;
		case GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV:
			append(buffer, buffer_size, "OFFSET_TEXTURE_RECTANGLE_SCALE_NV");
			return;
		case GL_DEPENDENT_AR_TEXTURE_2D_NV:
			append(buffer, buffer_size, "DEPENDENT_AR_TEXTURE_2D_NV");
			return;
		case GL_DEPENDENT_GB_TEXTURE_2D_NV:
			append(buffer, buffer_size, "DEPENDENT_GB_TEXTURE_2D_NV");
			return;
		case GL_DOT_PRODUCT_NV:
			append(buffer, buffer_size, "DOT_PRODUCT_NV");
			return;
		case GL_DOT_PRODUCT_DEPTH_REPLACE_NV:
			append(buffer, buffer_size, "DOT_PRODUCT_DEPTH_REPLACE_NV");
			return;
		case GL_DOT_PRODUCT_TEXTURE_2D_NV:
			append(buffer, buffer_size, "DOT_PRODUCT_TEXTURE_2D_NV");
			return;
		case GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV:
			append(buffer, buffer_size, "DOT_PRODUCT_TEXTURE_RECTANGLE_NV");
			return;
		case GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV:
			append(buffer, buffer_size, "DOT_PRODUCT_TEXTURE_CUBE_MAP_NV");
			return;
		case GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV:
			append(buffer, buffer_size, "DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV");
			return;
		case GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV:
			append(buffer, buffer_size, "DOT_PRODUCT_REFLECT_CUBE_MAP_NV");
			return;
		case GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV:
			append(buffer, buffer_size, "DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV");
			return;
	}
	append(buffer, buffer_size, "unk(%d)", op);
}

static void decode_texture_cull_modes(char **buffer, size_t *buffer_size, const uint8_t *modes)
{
	for (int i = 0; i < 4; ++i)
		append(buffer, buffer_size, modes[i] ? " GL_LESS" : " GL_GEQUAL");
}

static void decode_texture_mapping(char **buffer, size_t *buffer_size, uint32_t mapping)
{
	switch (mapping)
	{
		case GL_UNSIGNED_IDENTITY_NV:
			append(buffer, buffer_size, "UNSIGNED_IDENTITY_NV");
			return;
		case GL_EXPAND_NORMAL_NV:
			append(buffer, buffer_size, "EXPAND_NORMAL_NV");
			return;
	}
	append(buffer, buffer_size, "unk(%d)", mapping);
}

static void decode_texture_unit(char **buffer, size_t *buffer_size, const struct texture_unit *unit, size_t i)
{
	append(buffer, buffer_size, ";unit %d\nop: ", (int)i);
	decode_texture_operation(buffer, buffer_size, unit->operation);
	append(buffer, buffer_size, "\ncull modes: ");
	decode_texture_cull_modes(buffer, buffer_size, unit->cull_modes);
	append(buffer, buffer_size, "\nmapping: ");
	decode_texture_mapping(buffer, buffer_size, unit->mapping);
	append(buffer, buffer_size, "\n");
	if (i > GL_TEXTURE0 + unit->previous_texture)
		append(buffer, buffer_size, "previous texture: %d\n", unit->previous_texture - GL_TEXTURE0);
}

void decode_nv_texture_shader(char *buffer, size_t buffer_size, const void *data, size_t size)
{
	if (size < sizeof(struct texture_shader))
	{
		append(&buffer, &buffer_size, "invalid texture shader length: %u / %u", (unsigned)size, (unsigned)sizeof(struct texture_shader));
		return;
	}
	const struct texture_shader *shader = data;
	append(&buffer, &buffer_size, "; texture shader\n");
	for (size_t i = 0; i < 4; ++i)
	{
		size_t old = buffer_size;
		decode_texture_unit(&buffer, &buffer_size, &shader->units[i], i);
		if (old != buffer_size)
			append(&buffer, &buffer_size, "\n");
	}
}
