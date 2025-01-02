#include "virgl.h"

#include <sys/mman.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

struct virgl_attrib
{
	uint32_t stride;
	uint32_t offset;
	uint32_t handle;
};

struct shader_output
{
	uint32_t stride;
	uint32_t attr;
	uint32_t bind;
};

struct virgl_viewport
{
	float scale_x;
	float scale_y;
	float scale_z;
	float translate_x;
	float translate_y;
	float translate_z;
};

struct virgl_scissor
{
	uint16_t min_x;
	uint16_t max_x;
	uint16_t min_y;
	uint16_t max_y;
};

struct blend_attachment
{
	uint32_t enable;
	uint32_t rgb_eq;
	uint32_t rgb_src;
	uint32_t rgb_dst;
	uint32_t alpha_eq;
	uint32_t alpha_src;
	uint32_t alpha_dst;
	uint32_t color_mask;
};

struct blend_state_create_info
{
	uint32_t logic_enable;
	uint32_t logic_op;
	const struct blend_attachment *attachments;
	uint32_t nattachments;
};

struct blend_state
{
	uint32_t id;
};

struct shader_create_info
{
	enum pipe_shader_type type;
	const void *code;
	uint32_t code_size;
	const struct shader_output *outputs;
	uint32_t noutputs;
};

struct shader
{
	uint32_t id;
};

struct stencil_op_state
{
	uint32_t compare_func;
	uint32_t compare_mask;
	uint32_t fail_op;
	uint32_t pass_op;
	uint32_t zfail_op;
	uint32_t write_mask;
};

struct depth_stencil_state_create_info
{
	uint32_t depth_write_enable;
	uint32_t depth_test_enable;
	uint32_t depth_func;
	uint32_t stencil_enable;
	struct stencil_op_state front;
	struct stencil_op_state back;
};

struct depth_stencil_state
{
	uint32_t id;
};

struct rasterizer_state_create_info
{
	uint32_t depth_clamp;
	uint32_t discard_enable;
	uint32_t cull_face;
	uint32_t fill_mode;
	uint32_t front_ccw;
	float line_width;
	float point_size;
	uint32_t point_smooth;
	uint32_t line_smooth;
	uint32_t multisample;
	/* XXX more? */
};

struct rasterizer_state
{
	uint32_t id;
};

struct texture_create_info
{
	uint32_t target;
	uint32_t format;
	uint32_t bind;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t array_size;
	uint32_t last_level;
	uint32_t nr_samples;
	uint32_t flags;
	uint32_t bytes;
};

struct sampler_create_info
{
	uint32_t wrap_s;
	uint32_t wrap_t;
	uint32_t wrap_r;
	uint32_t min_filter;
	uint32_t mag_filter;
	uint32_t mip_filter;
	uint32_t compare_enable;
	uint32_t compare_func;
	uint32_t seamless_cubemap;
	uint32_t anisotropy;
	float lod_bias;
	uint32_t min_lod;
	uint32_t max_lod;
	float border_color[4];
};

struct texture
{
	struct res *res;
	uint32_t sampler_id;
	uint32_t view_id;
};

struct input_layout_entry
{
	uint32_t offset;
	uint32_t instance_divisor;
	uint32_t buffer_index;
	uint32_t format;
};

struct input_layout_create_info
{
	const struct input_layout_entry *entries;
	uint32_t nentries;
};

struct input_layout
{
	uint32_t id;
};

static inline uint32_t fconv(float f)
{
	union
	{
		float f;
		uint32_t u;
	} u =
	{
		.f = f,
	};
	return u.u;
}

static inline uint64_t dconv(double d)
{
	union
	{
		double d;
		uint64_t u;
	} u =
	{
		.d = d,
	};
	return u.u;
}

int blend_state_alloc(struct ctx *ctx,
                      const struct blend_state_create_info *create_info,
                      struct blend_state **statep);
int blend_state_bind(struct ctx *ctx, struct blend_state *state);
void blend_state_free(struct ctx *ctx, struct blend_state *state);
int shader_alloc(struct ctx *ctx,
                 const struct shader_create_info *create_info,
                 struct shader **shaderp);
void shader_free(struct ctx *ctx, struct shader *shader);
int depth_stencil_state_alloc(struct ctx *ctx,
                              const struct depth_stencil_state_create_info *create_info,
                              struct depth_stencil_state **statep);
int depth_stencil_state_bind(struct ctx *ctx, struct depth_stencil_state *state);
void depth_stencil_state_free(struct ctx *ctx, struct depth_stencil_state *state);
int rasterizer_state_alloc(struct ctx *ctx,
                           const struct rasterizer_state_create_info *create_info,
                           struct rasterizer_state **statep);
int rasterizer_state_bind(struct ctx *ctx, struct rasterizer_state *state);
void rasterizer_state_free(struct ctx *ctx, struct rasterizer_state *state);
int texture_alloc(struct ctx *ctx,
                  const struct texture_create_info *texture_create_info,
                  const struct sampler_create_info *sampler_create_info,
                  struct texture **texturep);
int texture_bind(struct ctx *ctx, struct texture *texture,
                 enum pipe_shader_type shader_type, uint32_t slot);
void texture_free(struct ctx *ctx, struct texture *texture);
int input_layout_alloc(struct ctx *ctx,
                       const struct input_layout_create_info *create_info,
                       struct input_layout **input_layoutp);
int input_layout_bind(struct ctx *ctx, struct input_layout *input_layout);
void input_layout_free(struct ctx *ctx, struct input_layout *input_layout);

void print_caps_fmt_mask(struct virgl_supported_format_mask *mask)
{
	for (size_t i = 0; i < 32 * 16; ++i)
	{
		if (mask->bitmask[i / 32] & (1 << (i % 32)))
			printf(" %zu", i);
	}
}

void print_caps_v1(struct virgl_caps_v1 *caps)
{
	printf("max_version: %" PRIu32 "\n", caps->max_version);
	printf("sampler:");
	print_caps_fmt_mask(&caps->sampler);
	printf("\n");
	printf("render:");
	print_caps_fmt_mask(&caps->render);
	printf("\n");
	printf("depthstencil:");
	print_caps_fmt_mask(&caps->depthstencil);
	printf("\n");
	printf("vertexbuffer:");
	print_caps_fmt_mask(&caps->vertexbuffer);
	printf("\n");
	printf("set:");
#define TESTB(name) \
	if (caps->bset.name) \
		printf(" " #name)
	TESTB(indep_blend_enable);
	TESTB(indep_blend_func);
	TESTB(cube_map_array);
	TESTB(shader_stencil_export);
	TESTB(conditional_render);
	TESTB(start_instance);
	TESTB(primitive_restart);
	TESTB(blend_eq_sep);
	TESTB(instanceid);
	TESTB(vertex_element_instance_divisor);
	TESTB(seamless_cube_map);
	TESTB(occlusion_query);
	TESTB(timer_query);
	TESTB(streamout_pause_resume);
	TESTB(texture_multisample);
	TESTB(fragment_coord_conventions);
	TESTB(depth_clip_disable);
	TESTB(seamless_cube_map_per_texture);
	TESTB(ubo);
	TESTB(color_clamping);
	TESTB(poly_stipple);
	TESTB(mirror_clamp);
	TESTB(texture_query_lod);
	TESTB(has_fp64);
	TESTB(has_tessellation_shaders);
	TESTB(has_indirect_draw);
	TESTB(has_sample_shading);
	TESTB(has_cull);
	TESTB(conditional_render_inverted);
	TESTB(derivative_control);
	TESTB(polygon_offset_clamp);
	TESTB(transform_feedback_overflow_query);
#undef TESTB
	printf("\n");
	printf("glsl level: %" PRIu32 "\n", caps->glsl_level);
	printf("max texture array layers: %" PRIu32 "\n", caps->max_texture_array_layers);
	printf("max streamout buffers: %" PRIu32 "\n", caps->max_streamout_buffers);
	printf("max dual source render targets: %" PRIu32 "\n", caps->max_dual_source_render_targets);
	printf("max render targets: %" PRIu32 "\n", caps->max_render_targets);
	printf("max samples: %" PRIu32 "\n", caps->max_samples);
	printf("prim mask: %" PRIu32 "\n", caps->prim_mask);
	printf("max tbo size: %" PRIu32 "\n", caps->max_tbo_size);
	printf("max uniform blocks: %" PRIu32 "\n", caps->max_uniform_blocks);
	printf("max viewports: %" PRIu32 "\n", caps->max_viewports);
	printf("max texture gather components: %" PRIu32 "\n", caps->max_texture_gather_components);
}

static uint32_t get_resource_id(struct ctx *ctx)
{
	/* XXX bitmap */
	return __atomic_add_fetch(&ctx->object_id, 1, __ATOMIC_SEQ_CST);
}

static int ctx_alloc_id(struct ctx *ctx, uint32_t *id)
{
	*id = get_resource_id(ctx);
	return 0;
}

static void ctx_free_id(struct ctx *ctx, uint32_t id)
{
	(void)ctx;
	(void)id;
	/* XXX */
}

int virgl_req_alloc(struct ctx *ctx,
                    enum virgl_context_cmd cmd,
                    enum virgl_object_type type,
                    uint32_t size,
                    uint32_t **requestp)
{
	/* XXX should be ringbuf to allow asynchronous submission */
	size_t bytes = 4 * (1 + size);
	if (ctx->cmd_buf.pos + bytes > ctx->cmd_buf.size)
	{
		int ret = vgl_submit(ctx);
		if (ret)
			return ret;
	}
	uint32_t *request = (uint32_t*)&ctx->cmd_buf.data[ctx->cmd_buf.pos];
	*request = VIRGL_CMD0(cmd, type, size);
	ctx->cmd_buf.pos += bytes;
	*requestp = request;
	return 0;
}

static int virgl_clear(struct ctx *ctx,
                       uint32_t buffers,
                       const float color[4],
                       double depth,
                       float stencil)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CLEAR,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_OBJ_CLEAR_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_CLEAR_BUFFERS] = buffers;
	request[VIRGL_OBJ_CLEAR_COLOR_0] = fconv(color[0]);
	request[VIRGL_OBJ_CLEAR_COLOR_1] = fconv(color[1]);
	request[VIRGL_OBJ_CLEAR_COLOR_2] = fconv(color[2]);
	request[VIRGL_OBJ_CLEAR_COLOR_3] = fconv(color[3]);
	request[VIRGL_OBJ_CLEAR_DEPTH_0] = dconv(depth);
	request[VIRGL_OBJ_CLEAR_DEPTH_1] = dconv(depth) >> 32;
	request[VIRGL_OBJ_CLEAR_STENCIL] = fconv(stencil);
	return 0;
}

static int virgl_create_surface(struct ctx *ctx,
                                uint32_t surface,
                                uint32_t resource,
                                enum virgl_formats format,
                                uint32_t level,
                                uint32_t first_layer,
                                uint32_t last_layer)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_SURFACE,
	                      VIRGL_OBJ_SURFACE_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_SURFACE_HANDLE] = surface;
	request[VIRGL_OBJ_SURFACE_RES_HANDLE] = resource;
	request[VIRGL_OBJ_SURFACE_FORMAT] = format;
	request[VIRGL_OBJ_SURFACE_TEXTURE_LEVEL] = level;
	request[VIRGL_OBJ_SURFACE_TEXTURE_LAYERS] = ((first_layer & 0xFFFF) <<  0)
	                                          | (( last_layer & 0xFFFF) << 16);
	return 0;
}

static int virgl_set_framebuffer_state(struct ctx *ctx,
                                       uint32_t nr_cbufs,
                                       const uint32_t *cbufs,
                                       uint32_t zsurf)
{
	uint32_t *request;
	int ret;

	if (nr_cbufs > VIRGL_MAX_COLOR_BUFS)
		return -EINVAL;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_FRAMEBUFFER_STATE,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_FRAMEBUFFER_STATE_SIZE(nr_cbufs),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_FRAMEBUFFER_STATE_NR_CBUFS] = nr_cbufs;
	request[VIRGL_SET_FRAMEBUFFER_STATE_NR_ZSURF_HANDLE] = zsurf;
	for (size_t i = 0; i < nr_cbufs; ++i)
		request[VIRGL_SET_FRAMEBUFFER_STATE_CBUF_HANDLE(i)] = cbufs[i];
	return 0;
}

static int virgl_set_vertex_buffers(struct ctx *ctx,
                                    uint32_t count,
                                    const struct virgl_attrib *attribs)
{
	uint32_t *request;
	int ret;

	if (count > PIPE_MAX_ATTRIBS)
		return -EINVAL;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_VERTEX_BUFFERS,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_VERTEX_BUFFERS_SIZE(count),
	                      &request);
	if (ret)
		return ret;
	for (size_t i = 0; i < count; ++i)
	{
		request[VIRGL_SET_VERTEX_BUFFER_STRIDE(i)] = attribs[i].stride;
		request[VIRGL_SET_VERTEX_BUFFER_OFFSET(i)] = attribs[i].offset;
		request[VIRGL_SET_VERTEX_BUFFER_HANDLE(i)] = attribs[i].handle;
	}
	return 0;
}

static int virgl_set_index_buffer(struct ctx *ctx,
                                  uint32_t handle,
                                  uint32_t index_size,
                                  uint32_t offset)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_INDEX_BUFFER,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_INDEX_BUFFER_SIZE(handle),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_INDEX_BUFFER_HANDLE] = handle;
	if (handle)
	{
		request[VIRGL_SET_INDEX_BUFFER_INDEX_SIZE] = index_size;
		request[VIRGL_SET_INDEX_BUFFER_OFFSET] = offset;
	}
	return 0;
}

static int virgl_set_blend_color(struct ctx *ctx,
                                 const float color[4])
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_BLEND_COLOR,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_BLEND_COLOR_SIZE,
	                      &request);
	if (ret)
		return ret;
	for (size_t i = 0; i < 4; ++i)
		request[VIRGL_SET_BLEND_COLOR(i)] = fconv(color[i]);
	return 0;
}

static int virgl_create_blend(struct ctx *ctx,
                              uint32_t handle,
                              uint32_t s0,
                              uint32_t s1,
                              const uint32_t s2[VIRGL_MAX_COLOR_BUFS])
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_BLEND,
	                      VIRGL_OBJ_BLEND_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_BLEND_HANDLE] = handle;
	request[VIRGL_OBJ_BLEND_S0] = s0;
	request[VIRGL_OBJ_BLEND_S1] = s1;
	for (size_t i = 0; i < VIRGL_MAX_COLOR_BUFS; ++i)
		request[VIRGL_OBJ_BLEND_S2(i)] = s2[i];
	return 0;
}

static int virgl_destroy_handle(struct ctx *ctx,
                                enum virgl_object_type type,
                                uint32_t handle)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_DESTROY_OBJECT,
	                      type,
	                      VIRGL_OBJ_DESTROY_HANDLE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_DESTROY_HANDLE] = handle;
	return 0;
}

static int virgl_create_dsa(struct ctx *ctx,
                            uint32_t handle,
                            uint32_t s0,
                            uint32_t s1,
                            uint32_t s2,
                            float alpha_ref)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_DSA,
	                      VIRGL_OBJ_DSA_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_DSA_HANDLE] = handle;
	request[VIRGL_OBJ_DSA_S0] = s0;
	request[VIRGL_OBJ_DSA_S1] = s1;
	request[VIRGL_OBJ_DSA_S2] = s2;
	request[VIRGL_OBJ_DSA_ALPHA_REF] = fconv(alpha_ref);
	return 0;
}

static int virgl_create_rasterizer(struct ctx *ctx,
                                   uint32_t handle,
                                   uint32_t s0,
                                   float point_size,
                                   uint32_t sprite_coord_enable,
                                   uint32_t s3,
                                   float line_width,
                                   float offset_units,
                                   float offset_scale,
                                   float offset_clamp)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_RASTERIZER,
	                      VIRGL_OBJ_RS_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_RS_HANDLE] = handle;
	request[VIRGL_OBJ_RS_S0] = s0;
	request[VIRGL_OBJ_RS_POINT_SIZE] = fconv(point_size);
	request[VIRGL_OBJ_RS_SPRITE_COORD_ENABLE] = sprite_coord_enable;
	request[VIRGL_OBJ_RS_S3] = s3;
	request[VIRGL_OBJ_RS_LINE_WIDTH] = fconv(line_width);
	request[VIRGL_OBJ_RS_OFFSET_UNITS] = fconv(offset_units);
	request[VIRGL_OBJ_RS_OFFSET_SCALE] = fconv(offset_scale);
	request[VIRGL_OBJ_RS_OFFSET_CLAMP] = fconv(offset_clamp);
	return 0;
}

static int virgl_set_stencil_ref(struct ctx *ctx,
                                 uint8_t back,
                                 uint8_t front)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_STENCIL_REF,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_STENCIL_REF_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_STENCIL_REF] = VIRGL_STENCIL_REF_VAL(front, back);
	return 0;
}

static int virgl_draw_indexed(struct ctx *ctx,
                              enum pipe_prim_type mode,
                              uint32_t count,
                              uint32_t start)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_DRAW_VBO,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_DRAW_VBO_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_DRAW_VBO_START] = start;
	request[VIRGL_DRAW_VBO_COUNT] = count;
	request[VIRGL_DRAW_VBO_MODE] = mode;
	request[VIRGL_DRAW_VBO_INDEXED] = 1;
	request[VIRGL_DRAW_VBO_INSTANCE_COUNT] = 1;
	request[VIRGL_DRAW_VBO_INDEX_BIAS] = 0;
	request[VIRGL_DRAW_VBO_START_INSTANCE] = 0;
	request[VIRGL_DRAW_VBO_PRIMITIVE_RESTART] = 0;
	request[VIRGL_DRAW_VBO_RESTART_INDEX] = 0;
	request[VIRGL_DRAW_VBO_MIN_INDEX] = 0;
	request[VIRGL_DRAW_VBO_MAX_INDEX] = (uint32_t)-1;
	request[VIRGL_DRAW_VBO_COUNT_FROM_SO] = 0;
	return 0;
}

static int virgl_create_vertex_elements(struct ctx *ctx,
                                        uint32_t handle,
                                        uint32_t count,
                                        const struct input_layout_entry *elements)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_VERTEX_ELEMENTS,
	                      VIRGL_OBJ_VERTEX_ELEMENTS_SIZE(count),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_VERTEX_ELEMENTS_HANDLE] = handle;
	for (size_t i = 0; i < count; ++i)
	{
		request[VIRGL_OBJ_VERTEX_ELEMENTS_V0_SRC_OFFSET(i)] = elements[i].offset;
		request[VIRGL_OBJ_VERTEX_ELEMENTS_V0_INSTANCE_DIVISOR(i)] = elements[i].instance_divisor;
		request[VIRGL_OBJ_VERTEX_ELEMENTS_V0_VERTEX_BUFFER_INDEX(i)] = elements[i].buffer_index;
		request[VIRGL_OBJ_VERTEX_ELEMENTS_V0_SRC_FORMAT(i)] = elements[i].format;
	}
	return 0;
}

static int virgl_bind_object(struct ctx *ctx,
                             enum virgl_object_type type,
                             uint32_t handle)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_BIND_OBJECT,
	                      type,
	                      VIRGL_OBJ_BIND_HANDLE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_BIND_HANDLE] = handle;
	return 0;
}

static int virgl_create_shader(struct ctx *ctx,
                               uint32_t handle,
                               enum pipe_shader_type type,
                               uint32_t outputs_count,
                               const struct shader_output *outputs,
                               size_t code_size,
                               const void *code)
{
	uint32_t hdr_size = VIRGL_OBJ_SHADER_HDR_SIZE(outputs_count);
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_SHADER,
	                      hdr_size + (code_size + 3) / 4,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_SHADER_HANDLE] = handle;
	request[VIRGL_OBJ_SHADER_TYPE] = type;
	request[VIRGL_OBJ_SHADER_OFFSET] = code_size;
	request[VIRGL_OBJ_SHADER_NUM_TOKENS] = 50; /* XXX less arbitrary */
	request[VIRGL_OBJ_SHADER_SO_NUM_OUTPUTS] = outputs_count;
	for (size_t i = 0; i < outputs_count; ++i)
	{
		request[VIRGL_OBJ_SHADER_SO_STRIDE(i)] = outputs[i].stride;
		request[VIRGL_OBJ_SHADER_SO_OUTPUT0(i)] = outputs[i].attr;
		request[VIRGL_OBJ_SHADER_SO_OUTPUT0_SO(i)] = outputs[i].bind;
	}
	memcpy(&request[hdr_size + 1], code, code_size);
	return 0;
}

static int virgl_bind_shader(struct ctx *ctx,
                             enum pipe_shader_type type,
                             uint32_t handle)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_BIND_SHADER,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_BIND_SHADER_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_BIND_SHADER_HANDLE] = handle;
	request[VIRGL_BIND_SHADER_TYPE] = type;
	return 0;
}

static int virgl_link_shader(struct ctx *ctx,
                             uint32_t vs,
                             uint32_t fs)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_LINK_SHADER,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_LINK_SHADER_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_LINK_SHADER_VERTEX_HANDLE] = vs;
	request[VIRGL_LINK_SHADER_FRAGMENT_HANDLE] = fs;
	request[VIRGL_LINK_SHADER_GEOMETRY_HANDLE] = 0;
	request[VIRGL_LINK_SHADER_TESS_CTRL_HANDLE] = 0;
	request[VIRGL_LINK_SHADER_TESS_EVAL_HANDLE] = 0;
	request[VIRGL_LINK_SHADER_COMPUTE_HANDLE] = 0;
	return 0;
}

static int virgl_set_viewport_state(struct ctx *ctx,
                                    uint32_t first,
                                    uint32_t count,
                                    const struct virgl_viewport *viewports)
{
	uint32_t *request;
	int ret;

	if (!count
	 || first >= PIPE_MAX_VIEWPORTS
	 || first + count >= PIPE_MAX_VIEWPORTS)
		return -EINVAL;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_VIEWPORT_STATE,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_VIEWPORT_STATE_SIZE(count),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_VIEWPORT_START_SLOT] = first;
	for (uint32_t i = 0; i < count; ++i)
	{
		request[VIRGL_SET_VIEWPORT_STATE_SCALE_0(i)] = fconv(viewports[i].scale_x);
		request[VIRGL_SET_VIEWPORT_STATE_SCALE_1(i)] = fconv(viewports[i].scale_y);
		request[VIRGL_SET_VIEWPORT_STATE_SCALE_2(i)] = fconv(viewports[i].scale_z);
		request[VIRGL_SET_VIEWPORT_STATE_TRANSLATE_0(i)] = fconv(viewports[i].translate_x);
		request[VIRGL_SET_VIEWPORT_STATE_TRANSLATE_1(i)] = fconv(viewports[i].translate_y);
		request[VIRGL_SET_VIEWPORT_STATE_TRANSLATE_2(i)] = fconv(viewports[i].translate_z);
	}
	return 0;
}

static int virgl_set_uniform_buffer(struct ctx *ctx,
                                    enum pipe_shader_type shader,
                                    uint32_t index,
                                    uint32_t offset,
                                    uint32_t length,
                                    uint32_t handle)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_UNIFORM_BUFFER,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_UNIFORM_BUFFER_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_UNIFORM_BUFFER_SHADER_TYPE] = shader;
	request[VIRGL_SET_UNIFORM_BUFFER_INDEX] = index;
	request[VIRGL_SET_UNIFORM_BUFFER_OFFSET] = offset;
	request[VIRGL_SET_UNIFORM_BUFFER_LENGTH] = length;
	request[VIRGL_SET_UNIFORM_BUFFER_RES_HANDLE] = handle;
	return 0;
}

static int virgl_create_sampler_state(struct ctx *ctx,
                                      uint32_t handle,
                                      uint32_t s0,
                                      float lod_bias,
                                      uint32_t min_lod,
                                      uint32_t max_lod,
                                      const float border[4])
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_SAMPLER_STATE,
	                      VIRGL_OBJ_SAMPLER_STATE_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_SAMPLER_STATE_HANDLE] = handle;
	request[VIRGL_OBJ_SAMPLER_STATE_S0] = s0;
	request[VIRGL_OBJ_SAMPLER_STATE_LOD_BIAS] = fconv(lod_bias);
	request[VIRGL_OBJ_SAMPLER_STATE_MIN_LOD] = min_lod;
	request[VIRGL_OBJ_SAMPLER_STATE_MAX_LOD] = max_lod;
	request[VIRGL_OBJ_SAMPLER_STATE_BORDER_COLOR(0)] = fconv(border[0]);
	request[VIRGL_OBJ_SAMPLER_STATE_BORDER_COLOR(1)] = fconv(border[1]);
	request[VIRGL_OBJ_SAMPLER_STATE_BORDER_COLOR(2)] = fconv(border[2]);
	request[VIRGL_OBJ_SAMPLER_STATE_BORDER_COLOR(3)] = fconv(border[3]);
	return 0;
}

static int virgl_create_sampler_view(struct ctx *ctx,
                                     uint32_t handle,
                                     uint32_t res,
                                     enum virgl_formats format,
                                     uint32_t first,
                                     uint32_t last,
                                     uint32_t layer,
                                     uint32_t level,
                                     uint32_t swizzle)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_SAMPLER_VIEW,
	                      VIRGL_OBJ_SAMPLER_VIEW_SIZE,
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_OBJ_SAMPLER_VIEW_HANDLE] = handle;
	request[VIRGL_OBJ_SAMPLER_VIEW_RES_HANDLE] = res;
	request[VIRGL_OBJ_SAMPLER_VIEW_FORMAT] = format;
	request[VIRGL_OBJ_SAMPLER_VIEW_BUFFER_FIRST_ELEMENT] = first;
	request[VIRGL_OBJ_SAMPLER_VIEW_BUFFER_LAST_ELEMENT] = last;
	request[VIRGL_OBJ_SAMPLER_VIEW_TEXTURE_LAYER] = layer;
	request[VIRGL_OBJ_SAMPLER_VIEW_TEXTURE_LEVEL] = level;
	request[VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE] = swizzle;
	return 0;
}

static int virgl_set_sampler_views(struct ctx *ctx,
                                   enum pipe_shader_type shader_type,
                                   uint32_t first,
                                   uint32_t count,
                                   const uint32_t *sampler_views)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_SAMPLER_VIEWS,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_SAMPLER_VIEWS_SIZE(count),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_SAMPLER_VIEWS_SHADER_TYPE] = shader_type;
	request[VIRGL_SET_SAMPLER_VIEWS_START_SLOT] = first;
	for (size_t i = 0; i < count; ++i)
		request[VIRGL_SET_SAMPLER_VIEWS_V0_HANDLE + i] = sampler_views[i];
	return 0;
}

static int virgl_bind_sampler_states(struct ctx *ctx,
                                     enum pipe_shader_type shader_type,
                                     uint32_t first,
                                     uint32_t count,
                                     const uint32_t *sampler_states)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_BIND_SAMPLER_STATES,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_BIND_SAMPLER_STATES(count),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_BIND_SAMPLER_STATES_SHADER_TYPE] = shader_type;
	request[VIRGL_BIND_SAMPLER_STATES_START_SLOT] = first;
	for (size_t i = 0; i < count; ++i)
		request[VIRGL_BIND_SAMPLER_STATES_S0_HANDLE + i] = sampler_states[i];
	return 0;
}

static int virgl_set_scissor_state(struct ctx *ctx,
                                   uint32_t first,
                                   uint32_t count,
                                   const struct virgl_scissor *scissors)
{
	uint32_t *request;
	int ret;

	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_SET_SCISSOR_STATE,
	                      VIRGL_OBJECT_NULL,
	                      VIRGL_SET_SCISSOR_STATE_SIZE(count),
	                      &request);
	if (ret)
		return ret;
	request[VIRGL_SET_SCISSOR_START_SLOT] = first;
	for (uint32_t i = 0; i < count; ++i)
	{
		request[VIRGL_SET_SCISSOR_MINX_MINY(i)] = (scissors[i].min_x << 0)
		                                        | (scissors[i].min_y << 16);
		request[VIRGL_SET_SCISSOR_MAXX_MAXY(i)] = (scissors[i].max_x << 0)
		                                        | (scissors[i].max_y << 16);
	}
	return 0;
}

int blend_state_alloc(struct ctx *ctx,
                      const struct blend_state_create_info *create_info,
                      struct blend_state **statep)
{
	uint32_t blend_s2[VIRGL_MAX_COLOR_BUFS];
	struct blend_state *state;
	int ret;

	state = calloc(1, sizeof(*state));
	if (!state)
		return ENOMEM;
	ret = ctx_alloc_id(ctx, &state->id);
	if (ret)
		goto err;
	for (size_t i = 0; i < create_info->nattachments; ++i)
	{
		const struct blend_attachment *attachment = &create_info->attachments[i];
		blend_s2[i] = VIRGL_OBJ_BLEND_S2_RT_BLEND_ENABLE(attachment->enable)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_FUNC(attachment->rgb_eq)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_SRC_FACTOR(attachment->rgb_src)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_DST_FACTOR(attachment->rgb_dst)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_FUNC(attachment->alpha_eq)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_SRC_FACTOR(attachment->alpha_src)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_DST_FACTOR(attachment->alpha_dst)
		            | VIRGL_OBJ_BLEND_S2_RT_COLORMASK(attachment->color_mask);
	}
	for (size_t i = create_info->nattachments; i < VIRGL_MAX_COLOR_BUFS; ++i)
	{
		blend_s2[i] = VIRGL_OBJ_BLEND_S2_RT_BLEND_ENABLE(1)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_FUNC(PIPE_BLEND_ADD)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_SRC_FACTOR(PIPE_BLENDFACTOR_SRC_ALPHA)
		            | VIRGL_OBJ_BLEND_S2_RT_RGB_DST_FACTOR(PIPE_BLENDFACTOR_INV_SRC_ALPHA)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_FUNC(PIPE_BLEND_ADD)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_SRC_FACTOR(PIPE_BLENDFACTOR_SRC_ALPHA)
		            | VIRGL_OBJ_BLEND_S2_RT_ALPHA_DST_FACTOR(PIPE_BLENDFACTOR_INV_SRC_ALPHA)
		            | VIRGL_OBJ_BLEND_S2_RT_COLORMASK(PIPE_MASK_RGBAZS);
	};
	ret = virgl_create_blend(ctx, state->id,
	                         VIRGL_OBJ_BLEND_S0_INDEPENDENT_BLEND_ENABLE(1)
	                       | VIRGL_OBJ_BLEND_S0_LOGICOP_ENABLE(create_info->logic_enable)
	                       | VIRGL_OBJ_BLEND_S0_DITHER(0)
	                       | VIRGL_OBJ_BLEND_S0_ALPHA_TO_COVERAGE(0)
	                       | VIRGL_OBJ_BLEND_S0_ALPHA_TO_ONE(0),
	                         VIRGL_OBJ_BLEND_S1_LOGICOP_FUNC(create_info->logic_op),
	                         blend_s2);
	if (ret)
		goto err;
	*statep = state;
	return 0;

err:
	blend_state_free(ctx, state);
	return ret;
}

int blend_state_bind(struct ctx *ctx, struct blend_state *state)
{
	return virgl_bind_object(ctx, VIRGL_OBJECT_BLEND, state->id);
}

void blend_state_free(struct ctx *ctx, struct blend_state *state)
{
	if (!state)
		return;
	if (state->id)
	{
		if (virgl_destroy_handle(ctx, VIRGL_OBJECT_BLEND, state->id))
			fprintf(stderr, "blend state free failed\n");
		ctx_free_id(ctx, state->id);
	}
	free(state);
}

int shader_alloc(struct ctx *ctx,
                 const struct shader_create_info *create_info,
                 struct shader **shaderp)
{
	struct shader *shader;
	int ret;

	shader = calloc(1, sizeof(*shader));
	if (!shader)
		return -ENOMEM;
	ret = ctx_alloc_id(ctx, &shader->id);
	if (ret)
		goto err;
	ret = virgl_create_shader(ctx,
	                          shader->id,
	                          create_info->type,
	                          create_info->noutputs,
	                          create_info->outputs,
	                          create_info->code_size,
	                          create_info->code);
	if (ret)
		goto err;
	*shaderp = shader;
	return 0;

err:
	shader_free(ctx, shader);
	return ret;
}

void shader_free(struct ctx *ctx, struct shader *shader)
{
	if (!shader)
		return;
	if (shader->id)
	{
		if (virgl_destroy_handle(ctx, VIRGL_OBJECT_SHADER, shader->id))
			fprintf(stderr, "shader free failed\n");
		ctx_free_id(ctx, shader->id);
	}
	free(shader);
}

int depth_stencil_state_alloc(struct ctx *ctx,
                              const struct depth_stencil_state_create_info *create_info,
                              struct depth_stencil_state **statep)
{
	struct depth_stencil_state *state;
	int ret;

	state = calloc(1, sizeof(*state));
	if (!state)
		return -ENOMEM;
	ret = ctx_alloc_id(ctx, &state->id);
	if (ret)
		goto err;
	ret = virgl_create_dsa(ctx,
	                       state->id,
	                       VIRGL_OBJ_DSA_S0_DEPTH_ENABLE(create_info->depth_test_enable)
	                     | VIRGL_OBJ_DSA_S0_DEPTH_WRITEMASK(create_info->depth_write_enable)
	                     | VIRGL_OBJ_DSA_S0_DEPTH_FUNC(create_info->depth_func)
	                     | VIRGL_OBJ_DSA_S0_ALPHA_ENABLED(0),
	                       VIRGL_OBJ_DSA_S1_STENCIL_ENABLED(create_info->stencil_enable)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_FUNC(create_info->front.compare_func)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_FAIL_OP(create_info->front.fail_op)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_ZPASS_OP(create_info->front.pass_op)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_ZFAIL_OP(create_info->front.zfail_op)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_VALUEMASK(create_info->front.compare_mask)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_WRITEMASK(create_info->front.write_mask),
	                       VIRGL_OBJ_DSA_S1_STENCIL_ENABLED(create_info->stencil_enable)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_FUNC(create_info->back.compare_func)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_FAIL_OP(create_info->back.fail_op)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_ZPASS_OP(create_info->back.pass_op)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_ZFAIL_OP(create_info->back.zfail_op)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_VALUEMASK(create_info->back.compare_mask)
	                     | VIRGL_OBJ_DSA_S1_STENCIL_WRITEMASK(create_info->back.write_mask),
	                       0);
	if (ret)
		goto err;
	*statep = state;
	return 0;

err:
	depth_stencil_state_free(ctx, state);
	return ret;
}

int depth_stencil_state_bind(struct ctx *ctx, struct depth_stencil_state *state)
{
	return virgl_bind_object(ctx, VIRGL_OBJECT_DSA, state->id);
}

void depth_stencil_state_free(struct ctx *ctx, struct depth_stencil_state *state)
{
	if (!state)
		return;
	if (state->id)
	{
		if (virgl_destroy_handle(ctx, VIRGL_OBJECT_DSA, state->id))
			fprintf(stderr, "depth stencil state free failed\n");
		ctx_free_id(ctx, state->id);
	}
	free(state);
}

int rasterizer_state_alloc(struct ctx *ctx,
                           const struct rasterizer_state_create_info *create_info,
                           struct rasterizer_state **statep)
{
	struct rasterizer_state *state;
	int ret;

	state = calloc(1, sizeof(*state));
	if (!state)
		return -ENOMEM;
	ret = ctx_alloc_id(ctx, &state->id);
	if (ret)
		goto err;
	ret = virgl_create_rasterizer(ctx,
	                              state->id,
	                              VIRGL_OBJ_RS_S0_FLATSHADE(0)
	                            | VIRGL_OBJ_RS_S0_DEPTH_CLIP(create_info->depth_clamp)
	                            | VIRGL_OBJ_RS_S0_CLIP_HALFZ(0)
	                            | VIRGL_OBJ_RS_S0_RASTERIZER_DISCARD(create_info->discard_enable)
	                            | VIRGL_OBJ_RS_S0_FLATSHADE_FIRST(0)
	                            | VIRGL_OBJ_RS_S0_LIGHT_TWOSIZE(0)
	                            | VIRGL_OBJ_RS_S0_SPRITE_COORD_MODE(0)
	                            | VIRGL_OBJ_RS_S0_POINT_QUAD_RASTERIZATION(0)
	                            | VIRGL_OBJ_RS_S0_CULL_FACE(create_info->cull_face)
	                            | VIRGL_OBJ_RS_S0_FILL_FRONT(create_info->fill_mode)
	                            | VIRGL_OBJ_RS_S0_FILL_BACK(create_info->fill_mode)
	                            | VIRGL_OBJ_RS_S0_SCISSOR(1)
	                            | VIRGL_OBJ_RS_S0_FRONT_CCW(create_info->front_ccw)
	                            | VIRGL_OBJ_RS_S0_CLAMP_VERTEX_COLOR(0)
	                            | VIRGL_OBJ_RS_S0_CLAMP_FRAGMENT_COLOR(0)
	                            | VIRGL_OBJ_RS_S0_OFFSET_LINE(0)
	                            | VIRGL_OBJ_RS_S0_OFFSET_POINT(0)
	                            | VIRGL_OBJ_RS_S0_OFFSET_TRI(0)
	                            | VIRGL_OBJ_RS_S0_POLY_SMOOTH(0)
	                            | VIRGL_OBJ_RS_S0_POLY_STIPPLE_ENABLE(0)
	                            | VIRGL_OBJ_RS_S0_POINT_SMOOTH(create_info->point_smooth)
	                            | VIRGL_OBJ_RS_S0_POINT_SIZE_PER_VERTEX(0)
	                            | VIRGL_OBJ_RS_S0_MULTISAMPLE(create_info->multisample)
	                            | VIRGL_OBJ_RS_S0_LINE_SMOOTH(create_info->line_smooth)
	                            | VIRGL_OBJ_RS_S0_LINE_LAST_PIXEL(0)
	                            | VIRGL_OBJ_RS_S0_HALF_PIXEL_CENTER(0)
	                            | VIRGL_OBJ_RS_S0_BOTTOM_EDGE_RULE(0)
	                            | VIRGL_OBJ_RS_S0_FORCE_PERSAMPLE_INTERP(0),
	                              create_info->point_size,
	                              0,
	                              VIRGL_OBJ_RS_S3_LINE_STIPPLE_PATTERN(0)
	                            | VIRGL_OBJ_RS_S3_LINE_STIPPLE_FACTOR(0)
	                            | VIRGL_OBJ_RS_S3_CLIP_PLANE_ENABLE(0),
	                              create_info->line_width,
	                              0,
	                              0,
	                              0);
	if (ret)
		goto err;
	*statep = state;
	return 0;
err:
	rasterizer_state_free(ctx, state);
	return ret;
}

int rasterizer_state_bind(struct ctx *ctx, struct rasterizer_state *state)
{
	return virgl_bind_object(ctx, VIRGL_OBJECT_RASTERIZER, state->id);
}

void rasterizer_state_free(struct ctx *ctx, struct rasterizer_state *state)
{
	if (!state)
		return;
	if (state->id)
	{
		if (virgl_destroy_handle(ctx, VIRGL_OBJECT_RASTERIZER, state->id))
			fprintf(stderr, "rasterizer state free failed\n");
		ctx_free_id(ctx, state->id);
	}
	free(state);
}

static void res_free(struct res *res)
{
	if (!res)
		return;
	if (res->data != MAP_FAILED)
		munmap(res->data, res->bytes);
	if (res->fd != -1)
		close(res->fd);
	free(res);
}

int texture2d_alloc(struct ctx *ctx,
                    const struct texture_create_info *create_info,
                    struct res **resp)
{
	struct res *res = NULL;
	int ret;

	res = calloc(1, sizeof(*res));
	if (!res)
	{
		fprintf(stderr, "%s: calloc: %s\n", ctx->env->progname,
		        strerror(errno));
		return -ENOMEM;
	}
	res->ctx = ctx;
	res->target = create_info->target;
	res->format = create_info->format;
	res->bind = create_info->bind;
	res->width = create_info->width;
	res->height = create_info->height;
	res->depth = create_info->depth;
	res->array_size = create_info->array_size;
	res->last_level = create_info->last_level;
	res->nr_samples = create_info->nr_samples;
	res->flags = create_info->flags;
	res->bytes = create_info->bytes;
	res->fd = -1;
	res->data = MAP_FAILED;
	ret = vgl_create_resource(res);
	if (ret)
	{
		fprintf(stderr, "%s: failed to create texture2d\n",
		        ctx->env->progname);
		goto err;
	}
	*resp = res;
	return 0;

err:
	res_free(res);
	return ret;
}

int texture2d_upload(struct res *res, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data, size_t size, size_t offset)
{
	memcpy(&((uint8_t*)res->data)[offset], data, size);
	return vgl_transfer_out(res, x, y, 0, width, height, 0, offset, 0, 0, 0);
}

int texture_alloc(struct ctx *ctx,
                  const struct texture_create_info *texture_create_info,
                  const struct sampler_create_info *sampler_create_info,
                  struct texture **texturep)
{
	struct texture *texture;
	int ret;

	texture = calloc(1, sizeof(*texture));
	if (!texture)
		return -ENOMEM;
	ret = texture2d_alloc(ctx, texture_create_info, &texture->res);
	if (ret)
		goto err;
	ret = ctx_alloc_id(ctx, &texture->sampler_id);
	if (ret)
		goto err;
	ret = virgl_create_sampler_state(ctx,
	                                 texture->sampler_id,
	                                 VIRGL_OBJ_SAMPLE_STATE_S0_WRAP_S(sampler_create_info->wrap_s)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_WRAP_T(sampler_create_info->wrap_t)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_WRAP_R(sampler_create_info->wrap_r)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_MIN_IMG_FILTER(sampler_create_info->min_filter)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_MIN_MIP_FILTER(sampler_create_info->mip_filter)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_MAG_IMG_FILTER(sampler_create_info->mag_filter)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_COMPARE_MODE(sampler_create_info->compare_enable)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_COMPARE_FUNC(sampler_create_info->compare_func)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_SEAMLESS_CUBE_MAP(sampler_create_info->seamless_cubemap)
	                               | VIRGL_OBJ_SAMPLE_STATE_S0_MAX_ANISOTROPY(sampler_create_info->anisotropy),
	                                 sampler_create_info->lod_bias,
	                                 sampler_create_info->min_lod,
	                                 sampler_create_info->max_lod,
	                                 sampler_create_info->border_color);
	if (ret)
		goto err;
	ret = ctx_alloc_id(ctx, &texture->view_id);
	if (ret)
		goto err;
	ret = virgl_create_sampler_view(ctx,
	                                texture->view_id,
	                                texture->res->id,
	                                texture_create_info->format,
	                                0, 0, 0, 0,
	                                VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE_R(PIPE_SWIZZLE_RED)
	                              | VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE_G(PIPE_SWIZZLE_GREEN)
	                              | VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE_B(PIPE_SWIZZLE_BLUE)
	                              | VIRGL_OBJ_SAMPLER_VIEW_SWIZZLE_A(PIPE_SWIZZLE_ALPHA));
	if (ret)
		goto err;
	*texturep = texture;
	return 0;

err:
	texture_free(ctx, texture);
	return ret;
}

int texture_bind(struct ctx *ctx, struct texture *texture,
                 enum pipe_shader_type shader_type, uint32_t slot)
{
	int ret;

	ret = virgl_set_sampler_views(ctx, shader_type, slot, 1, &texture->view_id);
	if (ret)
		return ret;
	ret = virgl_bind_sampler_states(ctx, shader_type, slot, 1, &texture->sampler_id);
	if (ret)
		return ret;
	return 0;
}

void texture_free(struct ctx *ctx, struct texture *texture)
{
	if (!texture)
		return;
	if (texture->view_id)
	{
		if (virgl_destroy_handle(ctx, VIRGL_OBJECT_SAMPLER_VIEW, texture->view_id))
			fprintf(stderr, "sampler view free failed\n");
		ctx_free_id(ctx, texture->view_id);
	}
	if (texture->sampler_id)
	{
		if (virgl_destroy_handle(ctx, VIRGL_OBJECT_SAMPLER_STATE, texture->sampler_id))
			fprintf(stderr, "sampler state free failed\n");
		ctx_free_id(ctx, texture->sampler_id);
	}
	res_free(texture->res);
	free(texture);
}

int input_layout_alloc(struct ctx *ctx,
                       const struct input_layout_create_info *create_info,
                       struct input_layout **input_layoutp)
{
	struct input_layout *input_layout;
	int ret;

	input_layout = calloc(1, sizeof(*input_layout));
	if (!input_layout)
		return -ENOMEM;
	ret = ctx_alloc_id(ctx, &input_layout->id);
	if (ret)
		goto err;
	ret = virgl_create_vertex_elements(ctx,
	                                   input_layout->id,
	                                   create_info->nentries,
	                                   create_info->entries);
	if (ret)
		goto err;
	*input_layoutp = input_layout;
	return 0;

err:
	input_layout_free(ctx, input_layout);
	return ret;
}

int input_layout_bind(struct ctx *ctx, struct input_layout *input_layout)
{
	return virgl_bind_object(ctx, VIRGL_OBJECT_VERTEX_ELEMENTS, input_layout->id);
}

void input_layout_free(struct ctx *ctx, struct input_layout *input_layout)
{
	if (!input_layout)
		return;
	if (input_layout->id)
	{
		if (virgl_destroy_handle(ctx, VIRGL_OBJECT_VERTEX_ELEMENTS, input_layout->id))
			fprintf(stderr, "input layout free failed\n");
		ctx_free_id(ctx, input_layout->id);
	}
	free(input_layout);
}

int buffer_alloc(struct ctx *ctx, uint32_t bind, size_t size,
                 struct res **resp)
{
	struct res *res = NULL;
	int ret;

	res = calloc(1, sizeof(*res));
	if (!res)
	{
		fprintf(stderr, "%s: calloc: %s\n", ctx->env->progname,
		        strerror(errno));
		goto err;
	}
	res->ctx = ctx;
	res->target = PIPE_BUFFER;
	res->format = VIRGL_FORMAT_NONE;
	res->bind = bind;
	res->width = size;
	res->height = 1;
	res->depth = 1;
	res->array_size = 1;
	res->last_level = 0;
	res->nr_samples = 1;
	/* XXX VIRGL_RESOURCE_FLAG_MAP_PERSISTENT | VIRGL_RESOURCE_FLAG_MAP_COHERENT */
	res->flags = 0;
	res->bytes = size;
	res->fd = -1;
	res->data = MAP_FAILED;
	ret = vgl_create_resource(res);
	if (ret)
	{
		fprintf(stderr, "%s: failed to create buffer\n",
		        ctx->env->progname);
		goto err;
	}
	*resp = res;
	return 0;

err:
	res_free(res);
	return 1;
}

int buffer_upload(struct res *res, const void *data, size_t size, size_t offset)
{
	memcpy(&((uint8_t*)res->data)[offset], data, size);
	return vgl_transfer_out(res, 0, 0, 0, size, 0, 0, offset, 0, 0, 0);
}

static void ctx_free(struct ctx *ctx)
{
	if (!ctx)
		return;
	free(ctx->cmd_buf.data);
	free(ctx);
}

static int ctx_alloc(struct env *env, struct ctx **ctxp)
{
	struct ctx *ctx = NULL;
	int ret;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
	{
		fprintf(stderr, "%s: calloc: %s\n", env->progname,
		        strerror(errno));
		goto err;
	}
	ctx->env = env;
	ctx->cmd_buf.size = 4096; /* XXX less arbitrary? */
	ctx->cmd_buf.data = malloc(ctx->cmd_buf.size); /* XXX mmap from virgl? */
	if (!ctx->cmd_buf.data)
	{
		fprintf(stderr, "%s: malloc: %s\n", env->progname,
		        strerror(errno));
		goto err;
	}
	if (vgl_create_context(ctx))
		goto err;
	ctx->surface = get_resource_id(ctx);
	if (virgl_create_surface(ctx, ctx->surface, ctx->fb,
	                         VIRGL_FORMAT_B8G8R8A8_UNORM, 0, 0, 0))
	{
		fprintf(stderr, "%s: failed to create surface\n",
		        env->progname);
		goto err;
	}
	ret = virgl_set_framebuffer_state(ctx, 1, &ctx->surface, 0);
	if (ret)
	{
		fprintf(stderr, "%s: failed to set framebuffer state\n",
		        env->progname);
		goto err;
	}
	*ctxp = ctx;
	return 0;

err:
	ctx_free(ctx);
	return 1;
}

static void usage(const char *progname)
{
	printf("%s [-h]\n", progname);
	printf("-h: show this help\n");
}

int main(int argc, char **argv)
{
	struct depth_stencil_state *depth_stencil_state;
	struct rasterizer_state *rasterizer_state;
	struct input_layout *input_layout;
	struct blend_state *blend_state;
	struct texture *texture;
	struct shader *vs;
	struct shader *fs;
	struct env env;
	int ret;
	int c;

	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	while ((c = getopt(argc, argv, "h")) != -1)
	{
		switch (c)
		{
			case 'h':
				usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	env.fd = open("/dev/virgl", O_RDONLY);
	if (env.fd == -1)
	{
		fprintf(stderr, "%s: open(%s): %s\n", argv[0], "/dev/virgl",
		        strerror(errno));
		return EXIT_FAILURE;
	}
	if (vgl_get_capset_info(&env, &env.capset_id, &env.capset_version,
	                        &env.capset_size))
		return EXIT_FAILURE;
	if (env.capset_size != sizeof(env.caps_v1))
	{
		fprintf(stderr, "%s: invalid capset size\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (vgl_get_capset(&env, &env.caps_v1, sizeof(env.caps_v1)))
		return EXIT_FAILURE;
	print_caps_v1(&env.caps_v1);
	if (ctx_alloc(&env, &env.ctx))
		return EXIT_FAILURE;
	static const uint16_t indexes[] =
	{
		0, 1, 2,
		0, 2, 3,
	};
	if (buffer_alloc(env.ctx, VIRGL_BIND_INDEX_BUFFER, sizeof(indexes),
	                 &env.index_buffer))
		return EXIT_FAILURE;
	if (buffer_upload(env.index_buffer, indexes, sizeof(indexes), 0))
		return EXIT_FAILURE;
	static const float vertexes[] =
	{
		0.25, 0.25, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
		0.50, 0.25, 0.0, 1.0, 0.6, 0.6, 0.6, 1.0, 1.0, 0.0,
		0.50, 0.50, 0.0, 1.0, 0.3, 0.3, 0.3, 1.0, 1.0, 1.0,
		0.25, 0.50, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0,
	};
	if (buffer_alloc(env.ctx, VIRGL_BIND_VERTEX_BUFFER, sizeof(vertexes),
	                 &env.vertex_buffer))
		return EXIT_FAILURE;
	if (buffer_upload(env.vertex_buffer, vertexes, sizeof(vertexes), 0))
		return EXIT_FAILURE;
	static const float matrix[] =
	{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0,
	};
	if (buffer_alloc(env.ctx, VIRGL_BIND_CONSTANT_BUFFER, sizeof(float) * 16,
	                 &env.uniform_buffer))
		return EXIT_FAILURE;
	if (buffer_upload(env.uniform_buffer, matrix, sizeof(matrix), 0))
		return EXIT_FAILURE;
	{
		struct texture_create_info texture_create_info;
		struct sampler_create_info sampler_create_info;
		texture_create_info.target = PIPE_TEXTURE_2D;
		texture_create_info.format = VIRGL_FORMAT_R8G8B8A8_UNORM;
		texture_create_info.bind = VIRGL_BIND_SAMPLER_VIEW;
		texture_create_info.width = 2;
		texture_create_info.height = 2;
		texture_create_info.depth = 1;
		texture_create_info.array_size = 1;
		texture_create_info.last_level = 0;
		texture_create_info.nr_samples = 1;
		texture_create_info.flags = 0;
		texture_create_info.bytes = 2 * 2 * 4;
		sampler_create_info.wrap_s = PIPE_TEX_WRAP_REPEAT;
		sampler_create_info.wrap_t = PIPE_TEX_WRAP_REPEAT;
		sampler_create_info.wrap_r = PIPE_TEX_WRAP_REPEAT;
		sampler_create_info.min_filter = PIPE_TEX_FILTER_NEAREST;
		sampler_create_info.mag_filter = PIPE_TEX_FILTER_NEAREST;
		sampler_create_info.mip_filter = PIPE_TEX_MIPFILTER_LINEAR;
		sampler_create_info.compare_enable = 0;
		sampler_create_info.compare_func = PIPE_FUNC_ALWAYS;
		sampler_create_info.seamless_cubemap = 0;
		sampler_create_info.anisotropy = 16;
		sampler_create_info.lod_bias = 0;
		sampler_create_info.min_lod = 0;
		sampler_create_info.max_lod = 0;
		sampler_create_info.border_color[0] = 0;
		sampler_create_info.border_color[1] = 0;
		sampler_create_info.border_color[2] = 0;
		sampler_create_info.border_color[3] = 0;
		ret = texture_alloc(env.ctx,
		                    &texture_create_info,
		                    &sampler_create_info,
		                    &texture);
		if (ret)
			return ret;
	}
	static const uint8_t texture_data[] =
	{
		0xFF, 0x00, 0x00, 0xFF,
		0x00, 0xFF, 0x00, 0xFF,
		0x00, 0x00, 0xFF, 0xFF,
		0xFF, 0xFF, 0x00, 0xFF,
	};
	if (texture2d_upload(texture->res, 0, 0, 2, 2, texture_data, sizeof(texture_data), 0))
		return EXIT_FAILURE;
	const struct virgl_attrib attribs[] =
	{
		{
			sizeof(float) * 10,
			0,
			env.vertex_buffer->id,
		},
	};
	if (virgl_set_vertex_buffers(env.ctx, 1, attribs))
		return EXIT_FAILURE;
	if (virgl_set_index_buffer(env.ctx, env.index_buffer->id, 2, 0))
		return EXIT_FAILURE;
	{
		const struct input_layout_entry entries[] =
		{
			{
				0,
				0,
				0,
				VIRGL_FORMAT_R32G32B32A32_FLOAT,
			},
			{
				sizeof(float) * 4,
				0,
				0,
				VIRGL_FORMAT_R32G32B32A32_FLOAT,
			},
			{
				sizeof(float) * 8,
				0,
				0,
				VIRGL_FORMAT_R32G32_FLOAT,
			},
		};
		struct input_layout_create_info create_info;
		create_info.nentries = sizeof(entries) / sizeof(*entries);
		create_info.entries = entries;
		ret = input_layout_alloc(env.ctx, &create_info, &input_layout);
		if (ret)
			return EXIT_FAILURE;
	}
	{
		const struct shader_output vs_outputs[] =
		{
			{
				sizeof(float) * 4,
				  VIRGL_OBJ_SHADER_SO_OUTPUT_REGISTER_INDEX(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_START_COMPONENT(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_NUM_COMPONENTS(4)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_BUFFER(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_DST_OFFSET(0),
				1,
			},
			{
				sizeof(float) * 4,
				  VIRGL_OBJ_SHADER_SO_OUTPUT_REGISTER_INDEX(1)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_START_COMPONENT(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_NUM_COMPONENTS(4)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_BUFFER(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_DST_OFFSET(0),
				1,
			},
			{
				sizeof(float) * 2,
				  VIRGL_OBJ_SHADER_SO_OUTPUT_REGISTER_INDEX(2)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_START_COMPONENT(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_NUM_COMPONENTS(2)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_BUFFER(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_DST_OFFSET(0),
				1,
			},
		};
		static const char *vs_code =
			"VERT\n"
			"DCL IN[0]\n"
			"DCL IN[1]\n"
			"DCL IN[2]\n"
			"DCL OUT[0], POSITION\n"
			"DCL OUT[1], COLOR, PERSPECTIVE\n"
			"DCL OUT[2], TEXCOORD, PERSPECTIVE\n"
			"DCL CONST[1][0..3]\n"
			"DP4 OUT[0].x, IN[0], CONST[1][0]\n"
			"DP4 OUT[0].y, IN[0], CONST[1][1]\n"
			"DP4 OUT[0].z, IN[0], CONST[1][2]\n"
			"DP4 OUT[0].w, IN[0], CONST[1][3]\n"
			"MOV OUT[1], IN[1]\n"
			"MOV OUT[2], IN[2]\n"
			"END\n";
		struct shader_create_info create_info;
		create_info.type = PIPE_SHADER_VERTEX;
		create_info.code = vs_code;
		create_info.code_size = strlen(vs_code) + 1;
		create_info.outputs = vs_outputs;
		create_info.noutputs = sizeof(vs_outputs) / sizeof(*vs_outputs);
		ret = shader_alloc(env.ctx, &create_info, &vs);
		if (ret)
			return EXIT_FAILURE;
	}
	{
		const struct shader_output fs_outputs[] =
		{
			{
				sizeof(float) * 4,
				  VIRGL_OBJ_SHADER_SO_OUTPUT_REGISTER_INDEX(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_START_COMPONENT(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_NUM_COMPONENTS(4)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_BUFFER(0)
				| VIRGL_OBJ_SHADER_SO_OUTPUT_DST_OFFSET(0),
				1,
			},
		};
		static const char *fs_code =
			"FRAG\n"
			"DCL IN[0], COLOR, PERSPECTIVE\n"
			"DCL IN[1], TEXCOORD, PERSPECTIVE\n"
			"DCL OUT[0], COLOR\n"
			"DCL SAMP[0]\n"
			"DCL SVIEW[0], 2D, FLOAT, FLOAT, FLOAT, FLOAT\n"
			"DCL TEMP[0]\n"
			"TEX TEMP[0], IN[1], SAMP[0], 2D\n"
			"MUL OUT[0], IN[0], TEMP[0]\n"
			"END\n";
		struct shader_create_info create_info;
		create_info.type = PIPE_SHADER_FRAGMENT;
		create_info.code = fs_code;
		create_info.code_size = strlen(fs_code) + 1;
		create_info.outputs = fs_outputs;
		create_info.noutputs = sizeof(fs_outputs) / sizeof(*fs_outputs);
		ret = shader_alloc(env.ctx, &create_info, &fs);
		if (ret)
			return EXIT_FAILURE;
	}
	if (virgl_bind_shader(env.ctx, PIPE_SHADER_VERTEX, vs->id))
		return EXIT_FAILURE;
	if (virgl_bind_shader(env.ctx, PIPE_SHADER_FRAGMENT, fs->id))
		return EXIT_FAILURE;
	{
		struct blend_state_create_info create_info;
		struct blend_attachment attachment;
		attachment.enable = 1;
		attachment.rgb_eq = PIPE_BLEND_ADD;
		attachment.rgb_src = PIPE_BLENDFACTOR_SRC_ALPHA;
		attachment.rgb_dst = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
		attachment.alpha_eq = PIPE_BLEND_ADD;
		attachment.alpha_src = PIPE_BLENDFACTOR_SRC_ALPHA;
		attachment.alpha_dst = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
		attachment.color_mask = PIPE_MASK_RGBAZS;
		create_info.logic_enable = 0;
		create_info.logic_op = PIPE_LOGICOP_CLEAR;
		create_info.attachments = &attachment;
		create_info.nattachments = 1;
		ret = blend_state_alloc(env.ctx, &create_info, &blend_state);
		if (ret)
			return EXIT_FAILURE;
	}
	{
		struct depth_stencil_state_create_info create_info;
		create_info.depth_write_enable = 0;
		create_info.depth_test_enable = 0;
		create_info.depth_func = PIPE_FUNC_ALWAYS;
		create_info.stencil_enable = 0;
		create_info.front.compare_func = PIPE_FUNC_ALWAYS;
		create_info.front.compare_mask = 0;
		create_info.front.fail_op = PIPE_STENCIL_OP_KEEP;
		create_info.front.pass_op = PIPE_STENCIL_OP_KEEP;
		create_info.front.zfail_op = PIPE_STENCIL_OP_KEEP;
		create_info.front.write_mask = 0xFF;
		create_info.back.compare_func = PIPE_FUNC_ALWAYS;
		create_info.back.compare_mask = 0;
		create_info.back.fail_op = PIPE_STENCIL_OP_KEEP;
		create_info.back.pass_op = PIPE_STENCIL_OP_KEEP;
		create_info.back.zfail_op = PIPE_STENCIL_OP_KEEP;
		create_info.back.write_mask = 0xFF;
		ret = depth_stencil_state_alloc(env.ctx, &create_info, &depth_stencil_state);
		if (ret)
			return EXIT_FAILURE;
	}
	{
		struct rasterizer_state_create_info create_info;
		create_info.depth_clamp = 0;
		create_info.discard_enable = 0;
		create_info.cull_face = PIPE_FACE_BACK;
		create_info.fill_mode = PIPE_POLYGON_MODE_FILL;
		create_info.front_ccw = 0;
		create_info.line_width = 1;
		create_info.point_size = 1;
		create_info.point_smooth = 1;
		create_info.line_smooth = 1;
		create_info.multisample = 1;
		ret = rasterizer_state_alloc(env.ctx, &create_info, &rasterizer_state);
		if (ret)
			return EXIT_FAILURE;
	}
	if (virgl_set_uniform_buffer(env.ctx, PIPE_SHADER_VERTEX, 1, 0, sizeof(matrix), env.uniform_buffer->id))
		return EXIT_FAILURE;
	while (1)
	{
		const struct virgl_viewport viewports[] =
		{
			{
				320, -240, 2,
				320, 240, -1,
			},
		};
		if (virgl_set_viewport_state(env.ctx, 0, 1, viewports))
			return EXIT_FAILURE;
		const struct virgl_scissor scissors[] =
		{
			{
				0, 640,
				0, 480,
			},
		};
		if (virgl_set_scissor_state(env.ctx, 0, 1, scissors))
			return EXIT_FAILURE;
		float clear_color[4] =
		{
			rand() / (float)RAND_MAX * 25,
			rand() / (float)RAND_MAX * 25,
			rand() / (float)RAND_MAX * 25,
			1,
		};
		if (virgl_clear(env.ctx,
		                PIPE_CLEAR_COLOR0
		              | PIPE_CLEAR_COLOR1
		              | PIPE_CLEAR_COLOR2
		              | PIPE_CLEAR_COLOR3
		              | PIPE_CLEAR_DEPTH
		              | PIPE_CLEAR_STENCIL,
		                clear_color, 0, 0))
			return EXIT_FAILURE;
		if (blend_state_bind(env.ctx, blend_state))
			return EXIT_FAILURE;
		if (rasterizer_state_bind(env.ctx, rasterizer_state))
			return EXIT_FAILURE;
		if (depth_stencil_state_bind(env.ctx, depth_stencil_state))
			return EXIT_FAILURE;
		if (texture_bind(env.ctx, texture, PIPE_SHADER_FRAGMENT, 0))
			return EXIT_FAILURE;
		if (input_layout_bind(env.ctx, input_layout))
			return EXIT_FAILURE;
		if (virgl_link_shader(env.ctx, vs->id, fs->id))
			return EXIT_FAILURE;
		if (virgl_draw_indexed(env.ctx, PIPE_PRIM_TRIANGLES, 6, 0))
			return EXIT_FAILURE;
		if (vgl_submit(env.ctx))
			return EXIT_FAILURE;
		if (vgl_flush(&env))
			return EXIT_FAILURE;
	}
	depth_stencil_state_free(env.ctx, depth_stencil_state);
	rasterizer_state_free(env.ctx, rasterizer_state);
	input_layout_free(env.ctx, input_layout);
	blend_state_free(env.ctx, blend_state);
	texture_free(env.ctx, texture);
	shader_free(env.ctx, fs);
	shader_free(env.ctx, vs);
	return EXIT_SUCCESS;
}
