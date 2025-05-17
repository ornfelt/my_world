#ifndef JKGL_INTERNAL_H
#define JKGL_INTERNAL_H

#include "jkg/jkg.h"
#include "utils.h"
#include "mat4.h"
#include "gl.h"

#include <sys/queue.h>

#define MODELVIEW_MAX_STACK_DEPTH  32
#define PROJECTION_MAX_STACK_DEPTH 2

#define MAX_VERTEX_ATTRIBS         32
#define MAX_VERTEX_ATTRIB_BINDINGS 32
#define MAX_TEXTURES               32
#define MAX_COLOR_ATTACHMENTS      32
#define MAX_DRAW_BUFFERS           32
#define MAX_UNIFORM_BLOCKS         32

#define MAX_LIGHTS 8

#define GL_CTX_DIRTY_BLEND_COLOR          (1 << 0)
#define GL_CTX_DIRTY_VIEWPORT             (1 << 1)
#define GL_CTX_DIRTY_VERTEX_ARRAY         (1 << 2)
#define GL_CTX_DIRTY_SCISSOR              (1 << 3)
#define GL_CTX_DIRTY_DEPTH_STENCIL_STATE  (1 << 4)
#define GL_CTX_DIRTY_RASTERIZER_STATE     (1 << 5)
#define GL_CTX_DIRTY_STENCIL_REF          (1 << 6)
#define GL_CTX_DIRTY_BLEND_STATE          (1 << 7)
#define GL_CTX_DIRTY_INDEX_TYPE           (1 << 8)
#define GL_CTX_DIRTY_IMAGE_VIEWS          (1 << 9)
#define GL_CTX_DIRTY_SAMPLERS             (1 << 10)
#define GL_CTX_DIRTY_PROGRAM              (1 << 11)
#define GL_CTX_DIRTY_DRAW_FBO             (1 << 12)
#define GL_CTX_DIRTY_UNIFORM_BLOCKS       (1 << 13)

#define GL_VERTEX_ARRAY_DIRTY_INDEX_BUFFER   (1 << 0)
#define GL_VERTEX_ARRAY_DIRTY_VERTEX_BUFFERS (1 << 1)
#define GL_VERTEX_ARRAY_DIRTY_INPUT_LAYOUT   (1 << 2)

#define GL_TEXTURE_DIRTY_SAMPLER    (1 << 0)
#define GL_TEXTURE_DIRTY_IMAGE_VIEW (1 << 1)
#define GL_TEXTURE_DIRTY_SURFACE    (1 << 2)

#define GL_BUFFER_DIRTY_VERTEX  (1 << 0)
#define GL_BUFFER_DIRTY_INDEX   (1 << 1)
#define GL_BUFFER_DIRTY_UNIFORM (1 << 2)

#ifdef NDEBUG
#define GL_SET_ERR(ctx, e) \
do \
{ \
	gl_set_err(ctx, e); \
} while (0)
#else
#define GL_SET_ERR(ctx, e) \
do \
{ \
	gl_set_err(ctx, e, __func__, __FILE__, __LINE__); \
} while (0)
#endif

enum conv_type
{
	CONV_INT,
	CONV_SCALED,
	CONV_NORM,
};

struct client_array
{
	GLboolean enable;
	GLboolean interleaved;
	GLint size;
	GLenum type;
	GLsizei stride;
	const GLvoid *pointer;
};

struct client_arrays
{
	struct client_array vertex;
	struct client_array color;
	struct client_array texcoord;
	struct client_array normal;
	struct client_array interleaved;
};

struct light
{
	GLboolean enable;
	GLfloat spot_exponent;
	GLfloat spot_cutoff;
	GLfloat attenuations[3];
	GLfloat spot_direction[3];
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat position[4];
};

struct material
{
	GLfloat shininess;
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat emission[4];
};

struct immediate_vertex
{
	GLfloat tex_coord[4];
	GLfloat color[4];
	GLfloat normal[3];
	GLfloat position[4];
};

struct immediate
{
	GLboolean enable;
	struct immediate_vertex *vertexes;
	size_t vertexes_len;
	size_t vertexes_pos;
	GLenum mode;
	GLfloat color[4];
	GLfloat normal[3];
	GLfloat tex_coord[4];
};

struct fixed_state
{
	uint32_t light_enable;
	uint32_t client_arrays_enable;
	GLboolean lighting_enable;
	GLboolean fog_enable;
	GLenum fog_mode;
	GLenum fog_coord_src;
	GLboolean texture_1d_enable;
	GLboolean texture_2d_enable;
	GLboolean texture_3d_enable;
};

struct fixed_block
{
	GLfloat projection_matrix[16];
	GLfloat modelview_matrix[16];
	GLfloat fog_color[4];
	GLfloat fog_factors[4];
	struct light lights[MAX_LIGHTS];
	GLboolean lighting_enable;
	struct material materials[2];
	GLboolean fog_enable;
	GLfloat fog_density;
	GLfloat fog_index;
	GLfloat fog_start;
	GLfloat fog_end;
	GLenum fog_mode;
	GLenum fog_coord_src;
	GLboolean texture_1d_enable;
	GLboolean texture_2d_enable;
	GLboolean texture_3d_enable;
};

struct fixed_pipeline
{
	GLenum matrix_mode;
	GLfloat projection_matrix[PROJECTION_MAX_STACK_DEPTH][16];
	GLfloat modelview_matrix[MODELVIEW_MAX_STACK_DEPTH][16];
	GLint projection_max_stack_depth;
	GLint modelview_max_stack_depth;
	GLint projection_stack_depth;
	GLint modelview_stack_depth;
	GLuint max_lights;
	GLenum shade_model;
	struct fixed_entry *entry;
	struct fixed_state state;
	struct fixed_block block;
	struct vertex_array *vao;
	struct buffer *vertex_buffer;
	struct buffer *color_buffer;
	struct buffer *texcoord_buffer;
	struct buffer *normal_buffer;
	struct buffer *interleaved_buffer;
	struct buffer *indice_buffer;
	struct buffer *uniform_buffer;
	struct client_arrays arrays;
};

enum object_type
{
	OBJECT_BUFFER,
	OBJECT_FRAMEBUFFER,
	OBJECT_LIST,
	OBJECT_PROGRAM,
	OBJECT_RENDERBUFFER,
	OBJECT_SAMPLER,
	OBJECT_SHADER,
	OBJECT_TEXTURE,
	OBJECT_VERTEX_ARRAY,
};

struct object
{
	enum object_type type;
	GLuint id;
};

struct buffer
{
	struct object object;
	GLsizeiptr size;
	struct jkg_buffer *buffer;
	GLuint dirty;
	GLenum target; /* XXX this is a hack */
};

struct sampler_state
{
	GLenum wrap_s;
	GLenum wrap_t;
	GLenum wrap_r;
	GLenum min_filter;
	GLenum mag_filter;
	GLenum compare_mode;
	GLenum compare_func;
	GLfloat lod_bias;
	GLfloat min_lod;
	GLfloat max_lod;
	GLuint max_anisotropy;
	GLfloat border_color[4];
};

struct image_view_state
{
	GLuint min_level;
	GLuint max_level;
	GLuint min_layer;
	GLuint num_layers;
	GLenum swizzle[4];
};

struct texture
{
	struct object object;
	GLuint dirty;
	GLuint width;
	GLuint height;
	GLuint depth;
	GLenum format;
	GLenum target;
	GLuint levels;
	enum jkg_format jkg_format;
	GLboolean immutable;
	struct sampler_state sampler_state;
	struct sampler_entry *sampler;
	struct image_view_state image_view_state;
	struct jkg_image_view *image_view;
	struct jkg_image *image;
};

struct sampler
{
	struct object object;
	struct sampler_state state;
	struct sampler_entry *sampler;
	bool dirty;
};

struct shader_uniform_block
{
	char *name;
	uint32_t binding;
};

struct shader_texture
{
	char *name;
	uint32_t binding;
};

struct shader_input
{
	char *name;
	uint32_t binding;
	enum jkg_format format;
};

struct shader_output
{
	char *name;
	uint32_t binding;
	enum jkg_format format;
};

struct shader
{
	struct object object;
	GLenum type;
	struct jkg_shader *shader;
	void *data;
	struct shader_uniform_block *uniform_blocks;
	struct shader_texture *textures;
	struct shader_input *inputs;
	struct shader_output *outputs;
	uint32_t size;
	uint32_t uniform_blocks_count;
	uint32_t shader_textures_count;
	uint32_t inputs_count;
	uint32_t outputs_count;
};

struct program
{
	struct object object;
	struct shader *vs;
	struct shader *fs;
	struct jkg_shader_state *shader_state;
};

struct vertex_buffer
{
	uint32_t stride;
	uint32_t offset;
	struct buffer *buffer;
};

struct input_layout_state
{
	struct jkg_input_layout_attrib attribs[MAX_VERTEX_ATTRIBS];
};

struct vertex_array
{
	struct object object;
	struct vertex_buffer vertex_buffers[MAX_VERTEX_ATTRIB_BINDINGS];
	struct input_layout_state input_layout_state;
	struct input_layout_entry *input_layout;
	struct buffer *index_buffer;
	GLuint dirty;
};

enum list_cmd_type
{
	CMD_DRAW,
};

struct list_cmd
{
	enum list_cmd_type type;
	GLsizei values[4];
};

struct list
{
	struct object object;
	struct list_cmd *cmds;
	GLsizei cmd_count;
	GLsizei cmd_size;
};

struct framebuffer_attachment
{
	GLenum type;
	union
	{
		struct object *object;
		struct jkg_image *image;
		struct texture *texture;
		struct renderbuffer *renderbuffer;
	};
	struct jkg_surface *surface;
	enum jkg_format format;
	bool dirty;
};

struct framebuffer
{
	struct object object;
	struct framebuffer_attachment color_attachments[MAX_COLOR_ATTACHMENTS];
	struct framebuffer_attachment depth_attachment;
	struct framebuffer_attachment stencil_attachment;
	GLenum draw_buffers[MAX_DRAW_BUFFERS];
	GLenum read_buffer;
	bool dirty;
};

struct renderbuffer
{
	struct object object;
	enum jkg_format format;
	GLuint width;
	GLuint height;
	struct jkg_image *image;
	bool dirty;
};

struct objects_range
{
	GLuint min;
	GLuint max;
	TAILQ_ENTRY(objects_range) chain;
};

TAILQ_HEAD(objects_range_head, objects_range);

struct stencil_op_state
{
	GLenum fail_op;
	GLenum pass_op;
	GLenum zfail_op;
	GLenum compare_op;
	GLuint compare_mask;
	GLuint write_mask;
};

struct depth_stencil_state
{
	GLboolean depth_test_enable;
	GLboolean depth_write_enable;
	GLenum depth_compare;
	GLboolean stencil_enable;
	struct stencil_op_state front;
	struct stencil_op_state back;
};

struct blend_attachment
{
	GLboolean enable;
	GLenum src_rgb;
	GLenum src_alpha;
	GLenum dst_rgb;
	GLenum dst_alpha;
	GLenum equation_rgb;
	GLenum equation_alpha;
	GLboolean color_mask[4];
};

struct blend_state
{
	struct blend_attachment attachments[MAX_COLOR_ATTACHMENTS];
	GLboolean logic_enable;
	GLenum logic_op;
};

struct rasterizer_state
{
	GLfloat point_size;
	GLfloat line_width;
	GLboolean point_smooth_enable;
	GLboolean line_smooth_enable;
	GLboolean scissor_enable;
	GLboolean depth_clamp_enable;
	GLboolean rasterizer_discard_enable;
	GLboolean multisample_enable;
	GLenum front_face;
	GLenum cull_face;
	GLboolean cull_enable;
	GLenum fill_front;
	GLenum fill_back;
};

struct cache_entry
{
	uint32_t hash;
	uint32_t ref;
	TAILQ_ENTRY(cache_entry) hash_chain;
	TAILQ_ENTRY(cache_entry) fifo_chain;
};

TAILQ_HEAD(cache_entry_head, cache_entry);

struct cache_op
{
	struct cache_entry *(*alloc)(struct gl_ctx *ctx, uint32_t hash, const void *state);
	void (*free)(struct gl_ctx *ctx, struct cache_entry *entry);
	uint32_t (*hash)(struct gl_ctx *ctx, const void *state);
	bool (*eq)(struct gl_ctx *ctx, const struct cache_entry *entry, const void *state);
};

struct cache
{
	const struct cache_op *op;
	struct cache_entry_head hash_head[512];
	struct cache_entry_head fifo_head;
	uint32_t hash_count;
	uint32_t fifo_count;
};

struct depth_stencil_entry
{
	struct cache_entry cache;
	struct depth_stencil_state state;
	struct jkg_depth_stencil_state *depth_stencil_state;
};

struct blend_entry
{
	struct cache_entry cache;
	struct blend_state state;
	struct jkg_blend_state *blend_state;
};

struct rasterizer_entry
{
	struct cache_entry cache;
	struct rasterizer_state state;
	struct jkg_rasterizer_state *rasterizer_state;
};

struct input_layout_entry
{
	struct cache_entry cache;
	struct input_layout_state state;
	struct jkg_input_layout *input_layout;
};

struct sampler_entry
{
	struct cache_entry cache;
	struct sampler_state state;
	struct jkg_sampler *sampler;
};

struct fixed_entry
{
	struct cache_entry cache;
	struct fixed_state state;
	struct program *program;
};

struct uniform_block
{
	struct buffer *buffer;
	uint32_t offset;
	uint32_t length;
};

struct gl_ctx
{
	const struct jkg_caps *jkg_caps;
	const struct jkg_op *jkg_op;
	struct jkg_ctx *jkg_ctx;
	void *jkg_dl;
	enum jkg_index_type index_type;
	GLfloat clear_color[4];
	GLfloat clear_depth;
	GLint clear_stencil;
	struct depth_stencil_state depth_stencil_state;
	struct depth_stencil_entry *depth_stencil;
	GLint stencil_ref[2];
	GLfloat viewport[4];
	GLfloat depth_range[2];
	struct blend_state blend_state;
	struct blend_entry *blend;
	GLfloat blend_color[4];
	struct rasterizer_state rasterizer_state;
	struct rasterizer_entry *rasterizer;
	GLenum active_texture;
	struct texture *textures[MAX_TEXTURES]; /* XXX should be a struct for each binding target */
	struct sampler *samplers[MAX_TEXTURES];
	struct uniform_block uniform_blocks[MAX_UNIFORM_BLOCKS];
#ifndef NDEBUG
	const char *err_file;
	const char *err_func;
	int err_line;
#endif
	GLenum err;
	GLfloat scissor[4];
	struct framebuffer *read_fbo;
	struct framebuffer *draw_fbo;
	struct framebuffer *default_fbo;
	GLboolean primitive_restart_enable;
	GLboolean primitive_restart_fixed_index_enable;
	GLuint primitive_restart_index;
	struct immediate immediate;
	struct fixed_pipeline fixed;
	GLuint dirty;
	GLuint version_major;
	GLuint version_minor;
	char **extensions;
	GLuint nextensions;
	struct object **objects;
	GLuint objects_size;
	struct jkg_id_list id_list;
	GLboolean back_buffer;
	GLuint width;
	GLuint height;
	struct objects_range_head objects_ranges;
	struct buffer *array_buffer;
	struct buffer *copy_read_buffer;
	struct buffer *copy_write_buffer;
	struct buffer *uniform_buffer;
	struct vertex_array *default_vao;
	struct vertex_array *vao;
	struct program *program;
	struct renderbuffer *renderbuffer;
	struct cache blend_cache;
	struct cache rasterizer_cache;
	struct cache depth_stencil_cache;
	struct cache input_layout_cache;
	struct cache sampler_cache;
	struct cache fixed_cache;
};

/* buffer */
struct buffer *buffer_alloc(struct gl_ctx *ctx);
void buffer_free(struct gl_ctx *ctx, struct buffer *buffer);
void buffer_storage(struct gl_ctx *ctx, struct buffer *buffer, GLsizeiptr size, const GLvoid *data, GLbitfield flags);
void buffer_data(struct gl_ctx *ctx, struct buffer *buffer, GLsizeiptr size, const GLvoid *data, GLenum usage);
void buffer_subdata(struct gl_ctx *ctx, struct buffer *buffer, GLintptr offset, GLsizeiptr size, const GLvoid *data);
void buffer_bind_range(struct gl_ctx *ctx, struct buffer *buffer, GLuint index, GLintptr offset, GLsizeiptr length);

/* cache */
void cache_init(struct cache *cache, const struct cache_op *op);
struct cache_entry *cache_get(struct gl_ctx *ctx, struct cache *cache, const void *state);
void cache_unref(struct gl_ctx *ctx, struct cache *cache, struct cache_entry *entry);

/* ctx */
int gl_ctx_commit(struct gl_ctx *ctx, GLuint mask);
int get_attrib_format(struct gl_ctx *ctx, GLint size, GLenum type, enum conv_type conv, enum jkg_format *formatp);
bool get_image_internal_format(GLenum internalformat, enum jkg_format *formatp);
GLenum get_jkg_error(int ret);
void gl_set_err(struct gl_ctx *ctx, GLenum err
#ifndef NDEBUG
, const char *func, const char *file, int line
#endif
);

/* draw */
void draw_arrays(struct gl_ctx *ctx, GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);

/* fixed */
bool fixed_init(struct gl_ctx *ctx);
bool fixed_update(struct gl_ctx *ctx, const GLvoid *indices, GLenum indice_type, GLsizei count);
void interleaved_arrays(struct gl_ctx *ctx, GLenum format, GLsizei stride, const GLvoid *pointer);

/* framebuffer */
int update_default_fbo(struct gl_ctx *ctx);
struct framebuffer *framebuffer_alloc(struct gl_ctx *ctx);
void framebuffer_free(struct gl_ctx *ctx, struct framebuffer *fbo);
int framebuffer_commit(struct gl_ctx *ctx, struct framebuffer *fbo);

/* object */
bool object_alloc(struct gl_ctx *ctx, struct object *object);
void object_free(struct gl_ctx *ctx, struct object *object);
void *object_get(struct gl_ctx *ctx, GLuint id, enum  object_type type);
bool object_is(struct gl_ctx *ctx, GLuint id, enum object_type type);

/* program */
struct program *program_alloc(struct gl_ctx *ctx);
void program_free(struct gl_ctx *ctx, struct program *program);
void program_bind(struct gl_ctx *ctx, struct program *program);
bool program_link(struct gl_ctx *ctx, struct program *program, struct shader *vs, struct shader *fs);

/* renderbuffer */
struct renderbuffer *renderbuffer_alloc(struct gl_ctx *ctx);
void renderbuffer_free(struct gl_ctx *ctx, struct renderbuffer *renderbuffer);
void renderbuffer_bind(struct gl_ctx *ctx, struct renderbuffer *renderbuffer);

/* sampler */
struct sampler *sampler_alloc(struct gl_ctx *ctx);
void sampler_free(struct gl_ctx *ctx, struct sampler *sampler);
void sampler_state_init(struct sampler_state *state);
bool sampler_state_set_wrap_s(struct gl_ctx *ctx, struct sampler_state *state, GLenum value);
bool sampler_state_set_wrap_t(struct gl_ctx *ctx, struct sampler_state *state, GLenum value);
bool sampler_state_set_wrap_r(struct gl_ctx *ctx, struct sampler_state *state, GLenum value);
bool sampler_state_set_min_filter(struct gl_ctx *ctx, struct sampler_state *state, GLenum value);
bool sampler_state_set_mag_filter(struct gl_ctx *ctx, struct sampler_state *state, GLenum value);
bool sampler_state_set_lod_bias(struct gl_ctx *ctx, struct sampler_state *state, GLfloat value);
bool sampler_state_set_min_lod(struct gl_ctx *ctx, struct sampler_state *state, GLint value);
bool sampler_state_set_max_lod(struct gl_ctx *ctx, struct sampler_state *state, GLint value);
bool sampler_state_set_compare_mode(struct gl_ctx *ctx, struct sampler_state *state, GLenum value);
bool sampler_state_set_compare_func(struct gl_ctx *ctx, struct sampler_state *state, GLenum value);
bool sampler_state_set_max_anisotropy(struct gl_ctx *ctx, struct sampler_state *state, GLuint value);
bool sampler_state_set_border_color(struct gl_ctx *ctx, struct sampler_state *state, const GLfloat *color);

/* shader */
struct shader *shader_alloc(struct gl_ctx *ctx, GLenum type);
void shader_free(struct gl_ctx *ctx, struct shader *shader);
bool shader_compile(struct gl_ctx *ctx, struct shader *shader, const struct jkg_shader_create_info *create_info);

/* texture */
struct texture *texture_alloc(struct gl_ctx *ctx);
void texture_free(struct gl_ctx *ctx, struct texture *texture);

/* vertex array */
struct vertex_array *vertex_array_alloc(struct gl_ctx *ctx);
void vertex_array_free(struct gl_ctx *ctx, struct vertex_array *vao);
void vertex_array_bind(struct gl_ctx *ctx, struct vertex_array *vao);
void vertex_array_index_buffer(struct gl_ctx *ctx, struct vertex_array *vao, struct buffer *buffer);
void vertex_array_attrib_enable(struct gl_ctx *ctx, struct vertex_array *vao, uint32_t index, bool enable);
int vertex_array_attrib_format(struct gl_ctx *ctx, struct vertex_array *vao, uint32_t index, uint32_t size, GLenum type, enum conv_type conv, uint32_t offset);
void vertex_array_attrib_binding(struct gl_ctx *ctx, struct vertex_array *vao, uint32_t attribindex, uint32_t bindingindex);
void vertex_array_binding_divisor(struct gl_ctx *ctx, struct vertex_array *vao, uint32_t bindingindex, uint32_t divisor);
void vertex_array_vertex_buffer(struct gl_ctx *ctx, struct vertex_array *vao, uint32_t index, struct buffer *buffer, uint32_t offset, uint32_t stride);

extern __thread struct gl_ctx *g_ctx;
extern const struct cache_op blend_cache_op;
extern const struct cache_op rasterizer_cache_op;
extern const struct cache_op depth_stencil_cache_op;
extern const struct cache_op input_layout_cache_op;
extern const struct cache_op sampler_cache_op;
extern const struct cache_op image_view_cache_op;
extern const struct cache_op fixed_cache_op;

#endif
