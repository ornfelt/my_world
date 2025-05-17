#include "internal.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>

__thread struct gl_ctx *g_ctx;

static const GLfloat identity[16] =
{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1,
};

/* XXX verify each one */
static char *extensions[] =
{
	"GL_ARB_copy_buffer",
	"GL_ARB_depth_clamp",
	"GL_ARB_draw_elements_base_vertex",
	"GL_ARB_draw_instanced",
	"GL_ARB_instanced_arrays",
	"GL_ARB_sampler_objects",
	"GL_ARB_texture_non_power_of_two",
	"GL_ARB_texture_swizzle",
	"GL_ARB_transpose_matrix",
	"GL_ARB_vertex_array_object",
	"GL_ARB_vertex_buffer_object",
	"GL_ATI_separate_stencil",
	"GL_ATI_texture_mirror_once",
	"GL_EXT_blend_color",
	"GL_EXT_blend_equation_separate",
	"GL_EXT_blend_func_separate",
	"GL_EXT_blend_minmax",
	"GL_EXT_blend_subtract",
	"GL_EXT_draw_instanced",
	"GL_EXT_texture_compression_rgtc",
	"GL_EXT_texture_compression_s3tc",
	"GL_EXT_texture_edge_clamp",
	"GL_EXT_texture_mirror_clamp",
	"GL_EXT_texture_object",
	"GL_EXT_vertex_array",
	"GL_NV_depth_clamp",
	"GL_NV_primitive_restart",
	NULL,
};

struct gl_ctx *
gl_ctx_new(void)
{
	struct jkg_ctx *(*ctx_alloc_fn)(void);
	struct gl_ctx *ctx;
	const char *driver;
	char driver_soname[64];

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return NULL;
	if (!jkg_id_init(&ctx->id_list, 1, UINT32_MAX - 1))
		goto err;
	driver = getenv("JKGL_DRIVER");
	if (!driver)
	{
#if 0
		driver = "gjit";
#else
		driver = "soft";
#endif
	}
	snprintf(driver_soname, sizeof(driver_soname), "libjkg_%s.so", driver);
	ctx->jkg_dl = dlopen(driver_soname, RTLD_NOW);
	if (!ctx->jkg_dl)
		goto err;
	ctx_alloc_fn = dlsym(ctx->jkg_dl, "jkg_ctx_alloc");
	if (!ctx_alloc_fn)
		goto err;
	ctx->jkg_ctx = ctx_alloc_fn();
	if (!ctx->jkg_ctx)
		goto err;
	ctx->jkg_op = ((void**)ctx->jkg_ctx)[0];
	ctx->jkg_caps = ((void**)ctx->jkg_ctx)[1];
	memcpy(&ctx->fixed.projection_matrix, &identity, sizeof(float) * 16);
	memcpy(&ctx->fixed.modelview_matrix, &identity, sizeof(float) * 16);
	ctx->extensions = extensions;
	while (ctx->extensions[ctx->nextensions])
		ctx->nextensions++;
	ctx->version_major = 4;
	ctx->version_minor = 6;
	ctx->clear_color[0] = 0;
	ctx->clear_color[1] = 0;
	ctx->clear_color[2] = 0;
	ctx->clear_color[3] = 0;
	ctx->clear_depth = 1;
	ctx->clear_stencil = 1;
	ctx->depth_stencil_state.depth_test_enable = GL_FALSE;
	ctx->depth_stencil_state.depth_write_enable = GL_TRUE;
	ctx->depth_stencil_state.depth_compare = GL_LESS;
	ctx->depth_stencil_state.stencil_enable = GL_FALSE;
	ctx->depth_stencil_state.front.fail_op = GL_KEEP;
	ctx->depth_stencil_state.front.pass_op = GL_KEEP;
	ctx->depth_stencil_state.front.zfail_op = GL_KEEP;
	ctx->depth_stencil_state.front.compare_op = GL_ALWAYS;
	ctx->depth_stencil_state.front.compare_mask = (GLuint)-1;
	ctx->depth_stencil_state.front.write_mask = (GLuint)-1;
	ctx->depth_stencil_state.back.fail_op = GL_KEEP;
	ctx->depth_stencil_state.back.pass_op = GL_KEEP;
	ctx->depth_stencil_state.back.zfail_op = GL_KEEP;
	ctx->depth_stencil_state.back.compare_op = GL_ALWAYS;
	ctx->depth_stencil_state.back.compare_mask = (GLuint)-1;
	ctx->depth_stencil_state.back.write_mask = (GLuint)-1;
	ctx->depth_stencil = NULL;
	ctx->stencil_ref[0] = 0;
	ctx->stencil_ref[1] = 0;
	ctx->viewport[0] = 0;
	ctx->viewport[1] = 0;
	ctx->viewport[2] = 0;
	ctx->viewport[3] = 0;
	ctx->depth_range[0] = 0;
	ctx->depth_range[1] = 1;
	ctx->scissor[0] = 0;
	ctx->scissor[1] = 0;
	ctx->scissor[2] = 0;
	ctx->scissor[3] = 0;
	for (GLuint i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
	{
		ctx->blend_state.attachments[i].enable = GL_FALSE;
		ctx->blend_state.attachments[i].src_rgb = GL_ONE;
		ctx->blend_state.attachments[i].src_alpha = GL_ONE;
		ctx->blend_state.attachments[i].dst_rgb = GL_ZERO;
		ctx->blend_state.attachments[i].dst_alpha = GL_ZERO;
		ctx->blend_state.attachments[i].equation_rgb = GL_FUNC_ADD;
		ctx->blend_state.attachments[i].equation_alpha = GL_FUNC_ADD;
		ctx->blend_state.attachments[i].color_mask[0] = GL_TRUE;
		ctx->blend_state.attachments[i].color_mask[1] = GL_TRUE;
		ctx->blend_state.attachments[i].color_mask[2] = GL_TRUE;
		ctx->blend_state.attachments[i].color_mask[3] = GL_TRUE;
	}
	ctx->blend_state.logic_enable = GL_FALSE;
	ctx->blend_state.logic_op = GL_COPY;
	ctx->blend = NULL;
	ctx->rasterizer_state.point_size = 1;
	ctx->rasterizer_state.line_width = 1;
	ctx->rasterizer_state.point_smooth_enable = GL_FALSE;
	ctx->rasterizer_state.line_smooth_enable = GL_FALSE;
	ctx->rasterizer_state.scissor_enable = GL_FALSE;
	ctx->rasterizer_state.depth_clamp_enable = GL_FALSE;
	ctx->rasterizer_state.rasterizer_discard_enable = GL_FALSE;
	ctx->rasterizer_state.multisample_enable = GL_FALSE;
	ctx->rasterizer_state.front_face = GL_CCW;
	ctx->rasterizer_state.cull_face = GL_BACK;
	ctx->rasterizer_state.cull_enable = GL_FALSE;
	ctx->rasterizer_state.fill_front = GL_FILL;
	ctx->rasterizer_state.fill_back = GL_FILL;
	ctx->rasterizer = NULL;
	ctx->active_texture = GL_TEXTURE0;
	ctx->fixed.projection_max_stack_depth = PROJECTION_MAX_STACK_DEPTH;
	ctx->fixed.modelview_max_stack_depth = MODELVIEW_MAX_STACK_DEPTH;
	ctx->fixed.matrix_mode = GL_MODELVIEW;
	ctx->fixed.block.fog_mode = GL_EXP;
	ctx->fixed.block.fog_density = 1;
	ctx->fixed.block.fog_start = 0;
	ctx->fixed.block.fog_end = 1;
	ctx->fixed.block.fog_index = 0;
	ctx->fixed.block.fog_coord_src = GL_FRAGMENT_DEPTH;
	ctx->fixed.block.texture_1d_enable = GL_FALSE;
	ctx->fixed.block.texture_2d_enable = GL_FALSE;
	ctx->fixed.block.texture_3d_enable = GL_FALSE;
	ctx->fixed.shade_model = GL_SMOOTH;
	ctx->fixed.max_lights = sizeof(ctx->fixed.block.lights) / sizeof(*ctx->fixed.block.lights);
	for (GLuint i = 0; i < ctx->fixed.max_lights; ++i)
	{
		struct light *light = &ctx->fixed.block.lights[i];
		light->ambient[3] = 1;
		light->position[2] = 1;
		light->diffuse[3] = 1;
		light->specular[3] = 1;
		light->spot_direction[2] = -1;
		light->attenuations[0] = 1;
		light->attenuations[1] = 0;
		light->attenuations[2] = 0;
	}
	ctx->fixed.block.lights[0].diffuse[0] = 1;
	ctx->fixed.block.lights[0].diffuse[1] = 1;
	ctx->fixed.block.lights[0].diffuse[2] = 1;
	ctx->fixed.block.lights[0].diffuse[3] = 1;
	ctx->fixed.block.lights[0].specular[0] = 1;
	ctx->fixed.block.lights[0].specular[1] = 1;
	ctx->fixed.block.lights[0].specular[2] = 1;
	ctx->fixed.block.lights[0].specular[3] = 1;
	for (GLuint i = 0; i < sizeof(ctx->fixed.block.materials) / sizeof(*ctx->fixed.block.materials); ++i)
	{
		struct material *material = &ctx->fixed.block.materials[i];
		material->ambient[0] = 0.2;
		material->ambient[1] = 0.2;
		material->ambient[2] = 0.2;
		material->ambient[3] = 1.0;
		material->diffuse[0] = 0.8;
		material->diffuse[1] = 0.8;
		material->diffuse[2] = 0.8;
		material->diffuse[3] = 1.0;
		material->specular[0] = 0;
		material->specular[1] = 0;
		material->specular[2] = 0;
		material->specular[3] = 1;
		material->emission[0] = 0;
		material->emission[1] = 0;
		material->emission[2] = 0;
		material->emission[3] = 1;
		material->shininess = 0;
	}
	ctx->immediate.enable = GL_FALSE;
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
	ctx->dirty = -1;
	ctx->array_buffer = NULL;
	ctx->copy_read_buffer = NULL;
	ctx->copy_write_buffer = NULL;
	ctx->default_vao = vertex_array_alloc(ctx);
	if (!ctx->default_vao)
		goto err;
	ctx->vao = ctx->default_vao;
	ctx->program = NULL;
	cache_init(&ctx->blend_cache, &blend_cache_op);
	cache_init(&ctx->rasterizer_cache, &rasterizer_cache_op);
	cache_init(&ctx->depth_stencil_cache, &depth_stencil_cache_op);
	cache_init(&ctx->input_layout_cache, &input_layout_cache_op);
	cache_init(&ctx->sampler_cache, &sampler_cache_op);
	cache_init(&ctx->fixed_cache, &fixed_cache_op);
	ctx->default_fbo = framebuffer_alloc(ctx);
	ctx->default_fbo->draw_buffers[0] = GL_BACK_LEFT;
	ctx->default_fbo->read_buffer = GL_BACK;
	update_default_fbo(ctx);
	ctx->draw_fbo = ctx->default_fbo;
	ctx->read_fbo = ctx->default_fbo;
	if (!fixed_init(ctx))
		goto err;
	return ctx;

err:
	gl_ctx_free(ctx);
	return NULL;
}

void
gl_ctx_free(struct gl_ctx *ctx)
{
	if (!ctx)
		return;
	if (ctx->jkg_ctx)
		ctx->jkg_op->destroy(ctx->jkg_ctx);
	if (ctx->jkg_dl)
		dlclose(ctx->jkg_dl);
	jkg_id_destroy(&ctx->id_list);
	free(ctx);
}

void
gl_ctx_set(struct gl_ctx *ctx)
{
	g_ctx = ctx;
}

void
gl_ctx_resize(struct gl_ctx *ctx, GLuint width, GLuint height)
{
	int ret;

	if (width == ctx->width
	 && height == ctx->height)
		return;
	ret = ctx->jkg_op->resize(ctx->jkg_ctx, width, height);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return;
	}
	ctx->width = width;
	ctx->height = height;
	update_default_fbo(ctx);
}

static int
update_index_buffer(struct gl_ctx *ctx)
{
	int ret;

	if (!ctx->vao->index_buffer
	 || !ctx->vao->index_buffer->buffer)
		return 0;
	ret = ctx->jkg_op->set_index_buffer(ctx->jkg_ctx,
	                                    ctx->vao->index_buffer->buffer,
	                                    ctx->index_type,
	                                    0);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->vao->dirty &= ~GL_VERTEX_ARRAY_DIRTY_INDEX_BUFFER;
	ctx->dirty &= ~GL_CTX_DIRTY_INDEX_TYPE;
	return 0;
}

static int
update_vertex_buffers(struct gl_ctx *ctx)
{
	struct jkg_vertex_buffer jkg_vertex_buffers[MAX_VERTEX_ATTRIB_BINDINGS];
	int ret;

	/* XXX
	 * the buffers should be re-indexed to makes them packed
	 * it will need some code to handle this inside input layout
	 * but it should be manageable
	 */
	for (size_t i = 0; i < ctx->jkg_caps->max_vertex_attrib_bindings; ++i)
	{
		struct buffer *buffer = ctx->vao->vertex_buffers[i].buffer;
		jkg_vertex_buffers[i].stride = ctx->vao->vertex_buffers[i].stride;
		jkg_vertex_buffers[i].offset = ctx->vao->vertex_buffers[i].offset;
		jkg_vertex_buffers[i].buffer = buffer ? buffer->buffer : NULL;
		if (buffer)
			buffer->dirty = false;
	}
	ret = ctx->jkg_op->set_vertex_buffers(ctx->jkg_ctx,
	                                      jkg_vertex_buffers,
	                                      ctx->jkg_caps->max_vertex_attrib_bindings);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->vao->dirty &= ~GL_VERTEX_ARRAY_DIRTY_VERTEX_BUFFERS;
	return 0;
}

static int
update_vao_input_layout(struct gl_ctx *ctx)
{
	if (ctx->vao->input_layout)
		cache_unref(ctx, &ctx->input_layout_cache, &ctx->vao->input_layout->cache);
	ctx->vao->input_layout = (struct input_layout_entry*)cache_get(ctx,
	                                                               &ctx->input_layout_cache,
	                                                               &ctx->vao->input_layout_state);
	if (!ctx->vao->input_layout)
		return 1;
	ctx->vao->dirty &= ~GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT;
	return 0;
}

static int
update_input_layout(struct gl_ctx *ctx)
{
	int ret;

	ret = ctx->jkg_op->input_layout_bind(ctx->jkg_ctx,
	                                     ctx->vao->input_layout->input_layout);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	return 0;
}

static int
update_rasterizer_state(struct gl_ctx *ctx)
{
	int ret;

	if (ctx->rasterizer)
		cache_unref(ctx, &ctx->rasterizer_cache, &ctx->rasterizer->cache);
	ctx->rasterizer = (struct rasterizer_entry*)cache_get(ctx,
	                                                      &ctx->rasterizer_cache,
	                                                      &ctx->rasterizer_state);
	if (!ctx->rasterizer)
		return 1;
	ret = ctx->jkg_op->rasterizer_state_bind(ctx->jkg_ctx,
	                                         ctx->rasterizer->rasterizer_state);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_RASTERIZER_STATE;
	return 0;
}

static int
update_blend_state(struct gl_ctx *ctx)
{
	int ret;

	if (ctx->blend)
		cache_unref(ctx, &ctx->blend_cache, &ctx->blend->cache);
	ctx->blend = (struct blend_entry*)cache_get(ctx,
	                                            &ctx->blend_cache,
	                                            &ctx->blend_state);
	if (!ctx->blend)
		return 1;
	ret = ctx->jkg_op->blend_state_bind(ctx->jkg_ctx,
	                                    ctx->blend->blend_state);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_BLEND_STATE;
	return 0;
}

static int
update_depth_stencil_state(struct gl_ctx *ctx)
{
	int ret;

	if (ctx->depth_stencil)
		cache_unref(ctx, &ctx->depth_stencil_cache, &ctx->depth_stencil->cache);
	ctx->depth_stencil = (struct depth_stencil_entry*)cache_get(ctx,
	                                                            &ctx->depth_stencil_cache,
	                                                            &ctx->depth_stencil_state);
	if (!ctx->depth_stencil)
		return 1;
	ret = ctx->jkg_op->depth_stencil_state_bind(ctx->jkg_ctx,
	                                            ctx->depth_stencil->depth_stencil_state);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_DEPTH_STENCIL_STATE;
	return 0;
}

static int
update_viewport(struct gl_ctx *ctx)
{
	float viewport[6];
	int ret;

	viewport[0] = ctx->viewport[0];
	viewport[1] = ctx->viewport[1];
	viewport[2] = ctx->depth_range[0];
	viewport[3] = ctx->viewport[0] + ctx->viewport[2];
	viewport[4] = ctx->viewport[1] + ctx->viewport[3];
	viewport[5] = ctx->depth_range[1];
	ret = ctx->jkg_op->set_viewport(ctx->jkg_ctx,
	                                viewport);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_VIEWPORT;
	return 0;
}

static int
update_scissor(struct gl_ctx *ctx)
{
	float scissor[4];
	int ret;

	scissor[0] = ctx->scissor[0];
	scissor[1] = ctx->scissor[1];
	scissor[2] = ctx->scissor[0] + ctx->scissor[2];
	scissor[3] = ctx->scissor[1] + ctx->scissor[3];
	ret = ctx->jkg_op->set_scissor(ctx->jkg_ctx,
	                               scissor);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_SCISSOR;
	return 0;
}

static int
update_stencil_ref(struct gl_ctx *ctx)
{
	int ret;

	ret = ctx->jkg_op->set_stencil_ref(ctx->jkg_ctx,
	                                   ctx->stencil_ref[0],
	                                   ctx->stencil_ref[1]);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_STENCIL_REF;
	return 0;
}

static int
update_blend_color(struct gl_ctx *ctx)
{
	int ret;

	ret = ctx->jkg_op->set_blend_color(ctx->jkg_ctx,
	                                   ctx->blend_color);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_BLEND_COLOR;
	return 0;
}

static int
update_image_views(struct gl_ctx *ctx)
{
	struct jkg_image_view *image_views[MAX_TEXTURES];
	int ret;

	for (size_t i = 0; i < MAX_TEXTURES; ++i)
	{
		struct texture *texture = ctx->textures[i];
		image_views[i] = texture ? texture->image_view : NULL;
	}
	ret = ctx->jkg_op->image_view_bind(ctx->jkg_ctx,
	                                   image_views,
	                                   0,
	                                   MAX_TEXTURES);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_IMAGE_VIEWS;
	return 0;
}

static int
update_samplers(struct gl_ctx *ctx)
{
	struct jkg_sampler *samplers[MAX_TEXTURES];
	int ret;

	for (size_t i = 0; i < MAX_TEXTURES; ++i)
	{
		struct sampler *sampler = ctx->samplers[i];
		if (sampler)
		{
			samplers[i] = sampler->sampler->sampler;
			continue;
		}
		struct texture *texture = ctx->textures[i];
		if (texture)
		{
			samplers[i] = texture->sampler->sampler;
			continue;
		}
		samplers[i] = NULL;
	}
	ret = ctx->jkg_op->sampler_bind(ctx->jkg_ctx,
	                                samplers,
	                                0,
	                                MAX_TEXTURES);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_SAMPLERS;
	return 0;
}

static int
update_program(struct gl_ctx *ctx)
{
	struct program *program;
	int ret;

	program = ctx->program;
	if (!program)
		program = ctx->fixed.entry->program;
	ret = ctx->jkg_op->shader_state_bind(ctx->jkg_ctx,
	                                     program->shader_state);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_PROGRAM;
	return 0;
}

static int
update_draw_fbo(struct gl_ctx *ctx)
{
	struct jkg_surface *color_surfaces[MAX_DRAW_BUFFERS];
	struct framebuffer *fbo;
	int ret;

	fbo = ctx->draw_fbo;
	if (fbo == ctx->default_fbo)
	{
		for (size_t i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
		{
			switch (fbo->draw_buffers[i])
			{
				/* XXX split more */
				case GL_FRONT:
				case GL_FRONT_LEFT:
				case GL_FRONT_RIGHT:
				case GL_LEFT:
				case GL_RIGHT:
					color_surfaces[i] = fbo->color_attachments[0].surface;
					break;
				case GL_BACK:
				case GL_BACK_LEFT:
				case GL_BACK_RIGHT:
					color_surfaces[i] = fbo->color_attachments[1].surface;
					break;
				default:
					assert(!"unknown draw buffer");
					/* FALLTHROUGH */
				case GL_NONE:
					color_surfaces[i] = NULL;
					break;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < ctx->jkg_caps->max_draw_buffers; ++i)
			color_surfaces[i] = fbo->draw_buffers[i] == GL_NONE ? NULL : fbo->color_attachments[fbo->draw_buffers[i] - GL_COLOR_ATTACHMENT0].surface;
	}
	ret = ctx->jkg_op->surface_bind(ctx->jkg_ctx,
	                                color_surfaces,
	                                ctx->jkg_caps->max_draw_buffers,
	                                fbo->depth_attachment.surface,
	                                fbo->stencil_attachment.surface);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_DRAW_FBO;
	return 0;
}

static enum jkg_swizzle
get_swizzle(GLenum swizzle)
{
	switch (swizzle)
	{
		default:
			assert(!"unknown swizzle");
			/* FALLTHROUGH */
		case GL_RED:
			return JKG_SWIZZLE_R;
		case GL_GREEN:
			return JKG_SWIZZLE_G;
		case GL_BLUE:
			return JKG_SWIZZLE_B;
		case GL_ALPHA:
			return JKG_SWIZZLE_A;
		case GL_ZERO:
			return JKG_SWIZZLE_ZERO;
		case GL_ONE:
			return JKG_SWIZZLE_ONE;
	}
}

static int
update_texture_image_view(struct gl_ctx *ctx,
                          struct texture *texture)
{
	struct jkg_image_view_create_info create_info;
	struct jkg_image_view *image_view;
	int ret;

	create_info.image = texture->image;
	create_info.min_level = texture->image_view_state.min_level;
	create_info.num_levels = texture->image_view_state.max_level;
	create_info.min_layer = texture->image_view_state.min_layer;
	create_info.num_layers = texture->image_view_state.num_layers;
	create_info.format = texture->jkg_format;
	create_info.swizzle[0] = get_swizzle(texture->image_view_state.swizzle[0]);
	create_info.swizzle[1] = get_swizzle(texture->image_view_state.swizzle[1]);
	create_info.swizzle[2] = get_swizzle(texture->image_view_state.swizzle[2]);
	create_info.swizzle[3] = get_swizzle(texture->image_view_state.swizzle[3]);
	ret = ctx->jkg_op->image_view_alloc(ctx->jkg_ctx,
	                                    &create_info,
	                                    &image_view);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	if (texture->image_view)
		ctx->jkg_op->image_view_free(ctx->jkg_ctx, texture->image_view);
	texture->image_view = image_view;
	return 0;
}

static int
update_texture_sampler(struct gl_ctx *ctx,
                       struct texture *texture)
{
	struct sampler_entry *entry;

	entry = (struct sampler_entry*)cache_get(ctx,
	                                         &ctx->sampler_cache,
	                                         &texture->sampler_state);
	if (!entry)
		return 1;
	if (texture->sampler)
		cache_unref(ctx, &ctx->sampler_cache, &texture->sampler->cache);
	texture->sampler = entry;
	return 0;
}

static int
update_texture_sampled(struct gl_ctx *ctx,
                       struct texture *texture)
{
	if (!texture || !texture->dirty)
		return 0;
	if (texture->dirty & GL_TEXTURE_DIRTY_IMAGE_VIEW)
	{
		if (update_texture_image_view(ctx, texture))
			return 1;
		texture->dirty &= ~GL_TEXTURE_DIRTY_IMAGE_VIEW;
		ctx->dirty |= GL_CTX_DIRTY_IMAGE_VIEWS;
	}
	if (texture->dirty & GL_TEXTURE_DIRTY_SAMPLER)
	{
		if (update_texture_sampler(ctx, texture))
			return 1;
		texture->dirty &= ~GL_TEXTURE_DIRTY_SAMPLER;
		ctx->dirty |= GL_CTX_DIRTY_SAMPLERS;
	}
	return 0;
}

static int
update_sampler(struct gl_ctx *ctx,
               struct sampler *sampler)
{
	struct sampler_entry *entry;

	if (!sampler || !sampler->dirty)
		return 0;
	entry = (struct sampler_entry*)cache_get(ctx,
	                                         &ctx->sampler_cache,
	                                         &sampler->state);
	if (!entry)
		return 1;
	if (sampler->sampler)
		cache_unref(ctx, &ctx->sampler_cache, &sampler->sampler->cache);
	sampler->sampler = entry;
	sampler->dirty = 0;
	ctx->dirty |= GL_CTX_DIRTY_SAMPLERS;
	return 0;
}

static void
update_vao_buffers(struct vertex_array *vao)
{
	if (vao->index_buffer
	 && (vao->index_buffer->dirty & GL_BUFFER_DIRTY_INDEX))
	{
		vao->dirty |= GL_VERTEX_ARRAY_DIRTY_INDEX_BUFFER;
		vao->index_buffer->dirty &= ~GL_BUFFER_DIRTY_INDEX;
	}
	for (size_t i = 0; i < MAX_VERTEX_ATTRIB_BINDINGS; ++i)
	{
		struct buffer *buffer = vao->vertex_buffers[i].buffer;
		if (buffer
		 && (buffer->dirty & GL_BUFFER_DIRTY_VERTEX))
		{
			vao->dirty |= GL_VERTEX_ARRAY_DIRTY_VERTEX_BUFFERS;
			buffer->dirty &= ~GL_BUFFER_DIRTY_VERTEX;
		}
	}
}

static int
update_fbo(struct gl_ctx *ctx,
           struct framebuffer *fbo)
{
	if (framebuffer_commit(ctx, fbo))
		return 1;
	if (fbo->dirty)
	{
		ctx->dirty |= GL_CTX_DIRTY_DRAW_FBO;
		fbo->dirty = false;
	}
	return 0;
}

static int
update_uniform_buffers(struct gl_ctx *ctx)
{
	for (size_t i = 0; i < ctx->jkg_caps->max_uniform_blocks; ++i)
	{
		if (!ctx->uniform_blocks[i].buffer)
			continue;
		if (ctx->uniform_blocks[i].buffer->dirty & GL_BUFFER_DIRTY_UNIFORM)
		{
			ctx->dirty |= GL_CTX_DIRTY_UNIFORM_BLOCKS;
			ctx->uniform_blocks[i].buffer->dirty &= ~GL_BUFFER_DIRTY_UNIFORM;
		}
	}
	return 0;
}

static int
update_uniform_blocks(struct gl_ctx *ctx)
{
	struct jkg_uniform_block blocks[MAX_UNIFORM_BLOCKS];
	int ret;

	for (size_t i = 0; i < ctx->jkg_caps->max_uniform_blocks; ++i)
	{
		uint32_t offset;
		uint32_t length;
		uint32_t end;

		if (!ctx->uniform_blocks[i].buffer
		 || !ctx->uniform_blocks[i].buffer->buffer)
		{
			blocks[i].buffer = NULL;
			continue;
		}
		offset = ctx->uniform_blocks[i].offset;
		if (offset >= ctx->uniform_blocks[i].buffer->size)
		{
			blocks[i].buffer = NULL;
			continue;
		}
		if (ctx->uniform_blocks[i].length == (uint32_t)-1)
		{
			length = ctx->uniform_blocks[i].buffer->size - offset;
		}
		else
		{
			length = ctx->uniform_blocks[i].length;
			if (__builtin_add_overflow(offset, length, &end)
			 || end > ctx->uniform_blocks[i].buffer->size)
			{
				blocks[i].buffer = NULL;
				continue;
			}
		}
		blocks[i].buffer = ctx->uniform_blocks[i].buffer->buffer;
		blocks[i].offset = offset;
		blocks[i].length = length;
	}
	ret = ctx->jkg_op->set_uniform_blocks(ctx->jkg_ctx,
	                                      blocks,
	                                      ctx->jkg_caps->max_uniform_blocks);
	if (ret)
	{
		GL_SET_ERR(ctx, get_jkg_error(ret));
		return 1;
	}
	ctx->dirty &= ~GL_CTX_DIRTY_UNIFORM_BLOCKS;
	return 0;
}

int
gl_ctx_commit(struct gl_ctx *ctx, GLuint mask)
{
	if ((mask & GL_CTX_DIRTY_VERTEX_ARRAY)
	 && !(ctx->dirty & GL_CTX_DIRTY_VERTEX_ARRAY))
	{
		update_vao_buffers(ctx->vao);
		if (ctx->vao->dirty & GL_VERTEX_ARRAY_DIRTY_INDEX_BUFFER)
		{
			if (update_index_buffer(ctx))
				return 1;
		}
		if (ctx->vao->dirty & GL_VERTEX_ARRAY_DIRTY_VERTEX_BUFFERS)
		{
			if (update_vertex_buffers(ctx))
				return 1;
		}
		if (ctx->vao->dirty & GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT)
		{
			if (update_vao_input_layout(ctx))
				return 1;
			if (update_input_layout(ctx))
				return 1;
		}
	}
	if (mask & GL_CTX_DIRTY_IMAGE_VIEWS)
	{
		for (size_t i = 0; i < MAX_TEXTURES; ++i)
		{
			if (update_texture_sampled(ctx, ctx->textures[i]))
				return 1;
		}
	}
	if (mask & GL_CTX_DIRTY_SAMPLERS)
	{
		for (size_t i = 0; i < MAX_TEXTURES; ++i)
		{
			if (update_sampler(ctx, ctx->samplers[i]))
				return 1;
		}
	}
	if (mask & GL_CTX_DIRTY_DRAW_FBO)
	{
		if (update_fbo(ctx, ctx->draw_fbo))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_UNIFORM_BLOCKS)
	{
		if (update_uniform_buffers(ctx))
			return 1;
	}
	mask &= ctx->dirty;
	if (!mask)
		return 0;
	if (mask & GL_CTX_DIRTY_VERTEX_ARRAY)
	{
		if (update_index_buffer(ctx))
			return 1;
		if (update_vertex_buffers(ctx))
			return 1;
		if (ctx->vao->dirty & GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT)
		{
			if (update_vao_input_layout(ctx))
				return 1;
		}
		if (update_input_layout(ctx))
			return 1;
		ctx->dirty &= ~GL_CTX_DIRTY_VERTEX_ARRAY;
	}
	if (mask & GL_CTX_DIRTY_INDEX_TYPE)
	{
		if (update_index_buffer(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_BLEND_COLOR)
	{
		if (update_blend_color(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_VIEWPORT)
	{
		if (update_viewport(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_SCISSOR)
	{
		if (update_scissor(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_STENCIL_REF)
	{
		if (update_stencil_ref(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_BLEND_STATE)
	{
		if (update_blend_state(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_DEPTH_STENCIL_STATE)
	{
		if (update_depth_stencil_state(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_RASTERIZER_STATE)
	{
		if (update_rasterizer_state(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_IMAGE_VIEWS)
	{
		if (update_image_views(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_SAMPLERS)
	{
		if (update_samplers(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_PROGRAM)
	{
		if (update_program(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_DRAW_FBO)
	{
		if (update_draw_fbo(ctx))
			return 1;
	}
	if (mask & GL_CTX_DIRTY_UNIFORM_BLOCKS)
	{
		if (update_uniform_blocks(ctx))
			return 1;
	}
	return 0;
}

void
gl_ctx_swap_buffers(struct gl_ctx *ctx)
{
	ctx->back_buffer = !ctx->back_buffer;
	update_default_fbo(ctx);
}

int
get_attrib_format(struct gl_ctx *ctx,
                  GLint size,
                  GLenum type,
                  enum conv_type conv,
                  enum jkg_format *formatp)
{
	enum jkg_format format;

	switch (size)
	{
		case 1:
		case 2:
		case 3:
		case 4:
			size--;
			break;
		case GL_BGRA:
			size = 4;
			break;
		default:
			GL_SET_ERR(ctx, GL_INVALID_VALUE);
			return 1;
	}
	switch (type)
	{
		case GL_BYTE:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R8_SINT,
				JKG_FORMAT_R8G8_SINT,
				JKG_FORMAT_R8G8B8_SINT,
				JKG_FORMAT_R8G8B8A8_SINT,
				JKG_FORMAT_B8G8R8A8_SINT,
				JKG_FORMAT_R8_SSCALED,
				JKG_FORMAT_R8G8_SSCALED,
				JKG_FORMAT_R8G8B8_SSCALED,
				JKG_FORMAT_R8G8B8A8_SSCALED,
				JKG_FORMAT_B8G8R8A8_SSCALED,
				JKG_FORMAT_R8_SNORM,
				JKG_FORMAT_R8G8_SNORM,
				JKG_FORMAT_R8G8B8_SNORM,
				JKG_FORMAT_R8G8B8A8_SNORM,
				JKG_FORMAT_B8G8R8A8_SNORM,
			};
			format = formats[conv * 5 + size];
			break;
		}
		case GL_UNSIGNED_BYTE:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R8_UINT,
				JKG_FORMAT_R8G8_UINT,
				JKG_FORMAT_R8G8B8_UINT,
				JKG_FORMAT_R8G8B8A8_UINT,
				JKG_FORMAT_B8G8R8A8_UINT,
				JKG_FORMAT_R8_USCALED,
				JKG_FORMAT_R8G8_USCALED,
				JKG_FORMAT_R8G8B8_USCALED,
				JKG_FORMAT_R8G8B8A8_USCALED,
				JKG_FORMAT_B8G8R8A8_USCALED,
				JKG_FORMAT_R8_UNORM,
				JKG_FORMAT_R8G8_UNORM,
				JKG_FORMAT_R8G8B8_UNORM,
				JKG_FORMAT_R8G8B8A8_UNORM,
				JKG_FORMAT_B8G8R8A8_UNORM,
			};
			format = formats[conv * 5 + size];
			break;
		}
		case GL_SHORT:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R16_SINT,
				JKG_FORMAT_R16G16_SINT,
				JKG_FORMAT_R16G16B16_SINT,
				JKG_FORMAT_R16G16B16A16_SINT,
				JKG_FORMAT_R16_SSCALED,
				JKG_FORMAT_R16G16_SSCALED,
				JKG_FORMAT_R16G16B16_SSCALED,
				JKG_FORMAT_R16G16B16A16_SSCALED,
				JKG_FORMAT_R16_SNORM,
				JKG_FORMAT_R16G16_SNORM,
				JKG_FORMAT_R16G16B16_SNORM,
				JKG_FORMAT_R16G16B16A16_SNORM,
			};
			if (size == 4)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return 1;
			}
			format = formats[conv * 4 + size];
			break;
		}
		case GL_UNSIGNED_SHORT:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R16_UINT,
				JKG_FORMAT_R16G16_UINT,
				JKG_FORMAT_R16G16B16_UINT,
				JKG_FORMAT_R16G16B16A16_UINT,
				JKG_FORMAT_R16_USCALED,
				JKG_FORMAT_R16G16_USCALED,
				JKG_FORMAT_R16G16B16_USCALED,
				JKG_FORMAT_R16G16B16A16_USCALED,
				JKG_FORMAT_R16_UNORM,
				JKG_FORMAT_R16G16_UNORM,
				JKG_FORMAT_R16G16B16_UNORM,
				JKG_FORMAT_R16G16B16A16_UNORM,
			};
			if (size == 4)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return 1;
			}
			format = formats[conv * 4 + size];
			break;
		}
		case GL_INT:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R32_SINT,
				JKG_FORMAT_R32G32_SINT,
				JKG_FORMAT_R32G32B32_SINT,
				JKG_FORMAT_R32G32B32A32_SINT,
			};
			if (size == 4)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return 1;
			}
			if (conv != CONV_INT)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return 1;
			}
			format = formats[size];
			break;
		}
		case GL_UNSIGNED_INT:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R32_UINT,
				JKG_FORMAT_R32G32_UINT,
				JKG_FORMAT_R32G32B32_UINT,
				JKG_FORMAT_R32G32B32A32_UINT,
			};
			if (size == 4)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return 1;
			}
			if (conv != CONV_INT)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return 1;
			}
			format = formats[size];
			break;
		}
		case GL_HALF_FLOAT:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R16_SFLOAT,
				JKG_FORMAT_R16G16_SFLOAT,
				JKG_FORMAT_R16G16B16_SFLOAT,
				JKG_FORMAT_R16G16B16A16_SFLOAT,
			};
			if (size == 4)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return 1;
			}
			if (conv != CONV_INT)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return 1;
			}
			format = formats[size];
			break;
		}
		case GL_FLOAT:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R32_SFLOAT,
				JKG_FORMAT_R32G32_SFLOAT,
				JKG_FORMAT_R32G32B32_SFLOAT,
				JKG_FORMAT_R32G32B32A32_SFLOAT,
			};
			if (size == 4)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return 1;
			}
			if (conv != CONV_SCALED)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return 1;
			}
			format = formats[size];
			break;
		}
		case GL_DOUBLE:
		{
			static const enum jkg_format formats[] =
			{
				JKG_FORMAT_R64_SFLOAT,
				JKG_FORMAT_R64G64_SFLOAT,
				JKG_FORMAT_R64G64B64_SFLOAT,
				JKG_FORMAT_R64G64B64A64_SFLOAT,
			};
			if (size == 4)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return 1;
			}
			if (conv != CONV_SCALED)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return 1;
			}
			format = formats[size];
			break;
		}
		case GL_UNSIGNED_INT_24_8:
		{
			if (size != 1)
			{
				GL_SET_ERR(ctx, GL_INVALID_VALUE);
				return 1;
			}
			if (conv != CONV_INT)
			{
				GL_SET_ERR(ctx, GL_INVALID_ENUM);
				return 1;
			}
			format = JKG_FORMAT_D24_UNORM_S8_UINT;
			break;
		}
		default:
			GL_SET_ERR(ctx, GL_INVALID_ENUM);
			return 1;
	}
	*formatp = format;
	return 0;
}

void
gl_sample(uint32_t id, const float coord[3], float color[4])
{
	struct gl_ctx *ctx = g_ctx;

	ctx->jkg_op->sample(ctx->jkg_ctx, id, coord, color);
}

GLenum
get_jkg_error(int ret)
{
	switch (ret)
	{
		case -ENOMEM:
			return GL_OUT_OF_MEMORY;
		case -EINVAL:
			return GL_INVALID_VALUE;
		default:
			return GL_INVALID_OPERATION;
	}
}

bool
get_image_internal_format(GLenum internalformat,
                          enum jkg_format *formatp)
{
	switch (internalformat)
	{
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_STENCIL:
		case GL_DEPTH24_STENCIL8:
			*formatp = JKG_FORMAT_D24_UNORM_S8_UINT;
			return true;
		case GL_RED:
			*formatp = JKG_FORMAT_R8_UNORM;
			return true;
		case GL_RG:
			*formatp = JKG_FORMAT_R8G8_UNORM;
			return true;
		case GL_RGB:
			*formatp = JKG_FORMAT_R8G8B8_UNORM;
			return true;
		case GL_RGBA:
			*formatp = JKG_FORMAT_R8G8B8A8_UNORM;
			return true;
		case GL_R8:
			*formatp = JKG_FORMAT_R8_UNORM;
			return true;
		case GL_R8_SNORM:
			*formatp = JKG_FORMAT_R8_SNORM;
			return true;
		case GL_R16:
			*formatp = JKG_FORMAT_R16_UNORM;
			return true;
		case GL_R16_SNORM:
			*formatp = JKG_FORMAT_R16_SNORM;
			return true;
		case GL_RG8:
			*formatp = JKG_FORMAT_R8G8_UNORM;
			return true;
		case GL_RG8_SNORM:
			*formatp = JKG_FORMAT_R8G8_SNORM;
			return true;
		case GL_RG16:
			*formatp = JKG_FORMAT_R16G16_UNORM;
			return true;
		case GL_RG16_SNORM:
			*formatp = JKG_FORMAT_R16G16_SNORM;
			return true;
		case GL_R3_G3_B2:
			/* XXX */
			return false;
		case GL_RGB4:
			/* XXX */
			return false;
		case GL_RGB5:
			/* XXX */
			return false;
		case GL_RGB8:
			*formatp = JKG_FORMAT_R8G8B8_UNORM;
			return true;
		case GL_RGB8_SNORM:
			*formatp = JKG_FORMAT_R8G8B8_SNORM;
			return true;
		case GL_RGB10:
			/* XXX */
			return false;
		case GL_RGB12:
			/* XXX */
			return false;
		case GL_RGB16_SNORM:
			*formatp = JKG_FORMAT_R16G16B16_SNORM;
			return true;
		case GL_RGBA2:
			/* XXX */
			return false;
		case GL_RGBA4:
			*formatp = JKG_FORMAT_R4G4B4A4_UNORM_PACK16;
			return true;
		case GL_RGB5_A1:
			*formatp = JKG_FORMAT_R5G5B5A1_UNORM_PACK16;
			return true;
		case GL_RGBA8:
			*formatp = JKG_FORMAT_R8G8B8A8_UNORM;
			return true;
		case GL_RGBA8_SNORM:
			*formatp = JKG_FORMAT_R8G8B8A8_SNORM;
			return true;
		case GL_RGB10_A2:
			/* XXX */
			return false;
		case GL_RGB10_A2UI:
			/* XXX */
			return false;
		case GL_RGBA12:
			/* XXX */
			return false;
		case GL_RGBA16:
			*formatp = JKG_FORMAT_R16G16B16A16_UNORM;
			return true;
		case GL_SRGB8:
			/* XXX */
			return false;
		case GL_SRGB8_ALPHA8:
			/* XXX */
			return false;
		case GL_R16F:
			*formatp = JKG_FORMAT_R16_SFLOAT;
			return true;
		case GL_RG16F:
			*formatp = JKG_FORMAT_R16G16_SFLOAT;
			return true;
		case GL_RGB16F:
			*formatp = JKG_FORMAT_R16G16B16_SFLOAT;
			return true;
		case GL_RGBA16F:
			*formatp = JKG_FORMAT_R16G16B16A16_SFLOAT;
			return true;
		case GL_R32F:
			*formatp = JKG_FORMAT_R32_SFLOAT;
			return true;
		case GL_RG32F:
			*formatp = JKG_FORMAT_R32G32_SFLOAT;
			return true;
		case GL_RGB32F:
			*formatp = JKG_FORMAT_R32G32B32_SFLOAT;
			return true;
		case GL_RGBA32F:
			*formatp = JKG_FORMAT_R32G32B32A32_SFLOAT;
			return true;
		case GL_R11F_G11F_B10F:
			/* XXX */
			return false;
		case GL_RGB9_E5:
			/* XXX */
			return false;
		case GL_R8I:
			*formatp = JKG_FORMAT_R8_SINT;
			return true;
		case GL_R8UI:
			*formatp = JKG_FORMAT_R8_UINT;
			return true;
		case GL_R16I:
			*formatp = JKG_FORMAT_R16_SINT;
			return true;
		case GL_R16UI:
			*formatp = JKG_FORMAT_R16_UINT;
			return true;
		case GL_R32I:
			*formatp = JKG_FORMAT_R32_SINT;
			return true;
		case GL_R32UI:
			*formatp = JKG_FORMAT_R32_UINT;
			return true;
		case GL_RG8I:
			*formatp = JKG_FORMAT_R8G8_SINT;
			return true;
		case GL_RG8UI:
			*formatp = JKG_FORMAT_R8G8_UINT;
			return true;
		case GL_RG16I:
			*formatp = JKG_FORMAT_R16G16_SINT;
			return true;
		case GL_RG16UI:
			*formatp = JKG_FORMAT_R16G16_UINT;
			return true;
		case GL_RG32I:
			*formatp = JKG_FORMAT_R32G32_SINT;
			return true;
		case GL_RG32UI:
			*formatp = JKG_FORMAT_R32G32_UINT;
			return true;
		case GL_RGB8I:
			*formatp = JKG_FORMAT_R8G8B8_SINT;
			return true;
		case GL_RGB8UI:
			*formatp = JKG_FORMAT_R8G8B8_UINT;
			return true;
		case GL_RGB16I:
			*formatp = JKG_FORMAT_R16G16B16_SINT;
			return true;
		case GL_RGB16UI:
			*formatp = JKG_FORMAT_R16G16B16_UINT;
			return true;
		case GL_RGB32I:
			*formatp = JKG_FORMAT_R32G32B32_SINT;
			return true;
		case GL_RGB32UI:
			*formatp = JKG_FORMAT_R32G32B32_UINT;
			return true;
		case GL_RGBA8I:
			*formatp = JKG_FORMAT_R8G8B8A8_SINT;
			return true;
		case GL_RGBA8UI:
			*formatp = JKG_FORMAT_R8G8B8A8_UINT;
			return true;
		case GL_RGBA16I:
			*formatp = JKG_FORMAT_R16G16B16A16_SINT;
			return true;
		case GL_RGBA16UI:
			*formatp = JKG_FORMAT_R16G16B16A16_UINT;
			return true;
		case GL_RGBA32I:
			*formatp = JKG_FORMAT_R32G32B32A32_SINT;
			return true;
		case GL_RGBA32UI:
			*formatp = JKG_FORMAT_R32G32B32A32_UINT;
			return true;
		case GL_COMPRESSED_RED:
			/* XXX */
			return false;
		case GL_COMPRESSED_RG:
			/* XXX */
			return false;
		case GL_COMPRESSED_RGB:
			/* XXX */
			return false;
		case GL_COMPRESSED_RGBA:
			/* XXX */
			return false;
		case GL_COMPRESSED_SRGB:
			/* XXX */
			return false;
		case GL_COMPRESSED_SRGB_ALPHA:
			/* XXX */
			return false;
		case GL_COMPRESSED_RED_RGTC1:
			/* XXX */
			return false;
		case GL_COMPRESSED_SIGNED_RED_RGTC1:
			/* XXX */
			return false;
		case GL_COMPRESSED_RG_RGTC2:
			/* XXX */
			return false;
		case GL_COMPRESSED_SIGNED_RG_RGTC2:
			/* XXX */
			return false;
		case GL_COMPRESSED_RGBA_BPTC_UNORM:
			/* XXX */
			return false;
		case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
			/* XXX */
			return false;
		case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
			/* XXX */
			return false;
		case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
			/* XXX */
			return false;
		default:
			return false;
	}
}

void
gl_set_err(struct gl_ctx *ctx, GLenum err
#ifndef NDEBUG
, const char *func, const char *file, int line
#endif
)
{
	ctx->err = err;
#ifndef NDEBUG
	ctx->err_func = func;
	ctx->err_file = file;
	ctx->err_line = line;
#endif
}
