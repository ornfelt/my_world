#include "internal.h"
#include "fixed.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#if defined(__i386__) || defined(__x86_64__)
#include <cpuid.h>
#endif

struct gl_ctx *g_ctx;

struct gl_ctx *gl_ctx_new(void)
{
	struct gl_ctx *ctx = calloc(sizeof(*ctx), 1);
	if (!ctx)
		return NULL;
	mat4_init_identity(&ctx->projection_matrix[0]);
	mat4_init_identity(&ctx->modelview_matrix[0]);
	ctx->projection_max_stack_depth = PROJECTION_MAX_STACK_DEPTH;
	ctx->modelview_max_stack_depth = MODELVIEW_MAX_STACK_DEPTH;
	ctx->matrix_mode = GL_MODELVIEW;
	ctx->depth_func = GL_LESS;
	ctx->depth_write = GL_TRUE;
	ctx->blend_src_rgb = GL_ONE;
	ctx->blend_src_alpha = GL_ONE;
	ctx->blend_dst_rgb = GL_ZERO;
	ctx->blend_dst_alpha = GL_ZERO;
	ctx->blend_equation_rgb = GL_FUNC_ADD;
	ctx->blend_equation_alpha = GL_FUNC_ADD;
	ctx->clear_depth = 1;
	ctx->clear_stencil = 1;
	ctx->fog_mode = GL_EXP;
	ctx->fog_density = 1;
	ctx->fog_start = 0;
	ctx->fog_end = 1;
	ctx->fog_index = 0;
	ctx->fog_coord_src = GL_FRAGMENT_DEPTH;
	ctx->vertex_array.size = 4;
	ctx->vertex_array.type = GL_FLOAT;
	ctx->color_array.size = 4;
	ctx->color_array.type = GL_FLOAT;
	ctx->texture_coord_array.size = 3;
	ctx->texture_coord_array.type = GL_FLOAT;
	ctx->normal_array.type = GL_FLOAT;
	ctx->textures_capacity = 0;
	ctx->shade_model = GL_SMOOTH;
	ctx->point_size = 1;
	ctx->color_mask[0] = GL_TRUE;
	ctx->color_mask[1] = GL_TRUE;
	ctx->color_mask[2] = GL_TRUE;
	ctx->color_mask[3] = GL_TRUE;
	ctx->depth_range[0] = 0;
	ctx->depth_range[1] = 1;
	ctx->front_face = GL_CCW;
	ctx->cull_face = GL_BACK;
	for (GLuint i = 0; i < sizeof(ctx->lights) / sizeof(*ctx->lights); ++i)
	{
		ctx->lights[i].ambient[3] = 1;
		ctx->lights[i].position[2] = 1;
		ctx->lights[i].diffuse[3] = 1;
		ctx->lights[i].specular[3] = 1;
		ctx->lights[i].spot_direction[2] = -1;
		ctx->lights[i].attenuations[0] = 1;
		ctx->lights[i].attenuations[1] = 0;
		ctx->lights[i].attenuations[2] = 0;
	}
	ctx->lights[0].diffuse[0] = 1;
	ctx->lights[0].diffuse[1] = 1;
	ctx->lights[0].diffuse[2] = 1;
	ctx->lights[0].diffuse[3] = 1;
	ctx->lights[0].specular[0] = 1;
	ctx->lights[0].specular[1] = 1;
	ctx->lights[0].specular[2] = 1;
	ctx->lights[0].specular[3] = 1;
	ctx->max_lights = sizeof(ctx->lights) / sizeof(*ctx->lights);
	for (GLuint i = 0; i < sizeof(ctx->materials) / sizeof(*ctx->materials); ++i)
	{
		ctx->materials[i].ambient[0] = .2;
		ctx->materials[i].ambient[1] = .2;
		ctx->materials[i].ambient[2] = .2;
		ctx->materials[i].ambient[3] = 1;
		ctx->materials[i].diffuse[0] = .8;
		ctx->materials[i].diffuse[1] = .8;
		ctx->materials[i].diffuse[2] = .8;
		ctx->materials[i].diffuse[3] = 1;
		ctx->materials[i].specular[0] = 0;
		ctx->materials[i].specular[1] = 0;
		ctx->materials[i].specular[2] = 0;
		ctx->materials[i].specular[3] = 1;
		ctx->materials[i].emission[0] = 0;
		ctx->materials[i].emission[1] = 0;
		ctx->materials[i].emission[2] = 0;
		ctx->materials[i].emission[3] = 1;
		ctx->materials[i].shininess = 0;
	}
	ctx->immediate.enabled = GL_FALSE;
	ctx->immediate.color[0] = 1;
	ctx->immediate.color[1] = 1;
	ctx->immediate.color[2] = 1;
	ctx->immediate.color[3] = 1;
	ctx->immediate.normal[0] = 0;
	ctx->immediate.normal[1] = 0;
	ctx->immediate.normal[2] = 1;
	ctx->immediate.tex_coord[0] = 0;
	ctx->immediate.tex_coord[1] = 0;
	ctx->immediate.tex_coord[2] = 0;
	ctx->immediate.tex_coord[3] = 1;
	ctx->immediate.vert_len = 0;
	ctx->immediate.vert_pos = 0;
	ctx->vs.fn = fixed_vertex_shader;
	ctx->vs.attr_nb = VERT_ATTR_NZ + 1;
	ctx->vs.varying_nb = VERT_VARYING_FOG + 1;
	ctx->fs.fn = fixed_fragment_shader;
	ctx->fs.varying_nb = VERT_VARYING_FOG + 1;
	ctx->dirty = -1;
#ifdef ENABLE_GCCJIT
	ctx->jit.ctx = gcc_jit_context_acquire();
	assert(ctx->jit.ctx);
	gcc_jit_context_set_bool_allow_unreachable_blocks(ctx->jit.ctx, 1);
	gcc_jit_context_set_int_option(ctx->jit.ctx,
	                               GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL,
	                               3);
	gcc_jit_context_set_bool_option(ctx->jit.ctx,
	                                GCC_JIT_BOOL_OPTION_DUMP_GENERATED_CODE,
	                                1);
	gcc_jit_context_set_bool_option(ctx->jit.ctx,
	                                GCC_JIT_BOOL_OPTION_DUMP_INITIAL_GIMPLE,
	                                1);
#if defined(__i386__) || defined(__x86_64__)
	uint32_t eax, ebx, ecx, edx;
	if (__get_cpuid(1, &eax, &ebx, &ecx, &edx))
	{
		if (edx & (1 << 25))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-msse");
		if (edx & (1 << 26))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-msse2");
		if (ecx & (1 << 0))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-msse3");
		if (ecx & (1 << 9))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mssse3");
		if (ecx & (1 << 12))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mfma");
		if (ecx & (1 << 19))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-msse4.1");
		if (ecx & (1 << 20))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-msse4.2");
		if (ecx & (1 << 22))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mmovbe");
		if (ecx & (1 << 28))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx");
	}
	if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx))
	{
		if (ebx & (1 << 5))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx2");
		if (ebx & (1 << 16))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512f");
		if (ebx & (1 << 17))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512dq");
		if (ebx & (1 << 21))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512ifma");
		if (ebx & (1 << 26))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512pf");
		if (ebx & (1 << 27))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512er");
		if (ebx & (1 << 28))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512cd");
		if (ebx & (1 << 30))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512bw");
		if (ebx & (1 << 31))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512vl");
		if (ecx & (1 << 1))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512vbmi");
		if (ecx & (1 << 6))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512vbmi2");
		if (ecx & (1 << 10))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mvpclmulqdq");
		if (ecx & (1 << 11))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512vnni");
		if (ecx & (1 << 12))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512bitalg");
		if (ecx & (1 << 14))
			gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-mavx512vpopcntdq");
	}
#endif
	gcc_jit_context_add_command_line_option(ctx->jit.ctx, "-ffast-math");
	ctx->jit.type_void = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_VOID);
	ctx->jit.type_voidp = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_VOID_PTR);
	ctx->jit.type_float = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_FLOAT);
	ctx->jit.type_floatp = gcc_jit_type_get_pointer(ctx->jit.type_float);
	ctx->jit.type_float4 = gcc_jit_context_new_array_type(ctx->jit.ctx, NULL, ctx->jit.type_float, 4);
	ctx->jit.type_int = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_INT);
#ifdef LIBGCCJIT_HAVE_SIZED_INTEGERS
	ctx->jit.type_uint8 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_UINT8_T);
#else
	ctx->jit.type_uint8 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_UNSIGNED_CHAR);
#endif
	ctx->jit.type_uint8p = gcc_jit_type_get_pointer(ctx->jit.type_uint8);
#ifdef LIBGCCJIT_HAVE_SIZED_INTEGERS
	ctx->jit.type_int32 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_INT32_T);
#else
	ctx->jit.type_int32 = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_INT);
#endif
	ctx->jit.type_size_t = gcc_jit_context_get_type(ctx->jit.ctx, GCC_JIT_TYPE_SIZE_T);
	TAILQ_INIT(&ctx->jit.fragment_set_fifo);
	for (size_t i = 0; i < sizeof(ctx->jit.fragment_set_hash) / sizeof(*ctx->jit.fragment_set_hash); ++i)
		TAILQ_INIT(&ctx->jit.fragment_set_hash[i]);
	TAILQ_INIT(&ctx->jit.depth_test_fifo);
	for (size_t i = 0; i < sizeof(ctx->jit.depth_test_hash) / sizeof(*ctx->jit.depth_test_hash); ++i)
		TAILQ_INIT(&ctx->jit.depth_test_hash[i]);
	TAILQ_INIT(&ctx->jit.texture_fetch_fifo);
	for (size_t i = 0; i < sizeof(ctx->jit.texture_fetch_hash) / sizeof(*ctx->jit.texture_fetch_hash); ++i)
		TAILQ_INIT(&ctx->jit.texture_fetch_hash[i]);
#endif
	return ctx;
}

void gl_ctx_set(struct gl_ctx *new_ctx)
{
	g_ctx = new_ctx;
}
