#ifndef INTERNAL_H
#define INTERNAL_H

#include "utils.h"
#include "mat4.h"
#include "gl.h"

#ifdef ENABLE_GCCJIT
#include <libgccjit.h>
#endif

#include <sys/queue.h>

#define MODELVIEW_MAX_STACK_DEPTH  32
#define PROJECTION_MAX_STACK_DEPTH 2

#define MAX_VERTEX_ATTRIBS 64
#define MAX_VARYING_FLOATS 64

#define MAX_LIGHTS 8

#define GL_CTX_DIRTY_BLEND_SRC_RGB        (1 << 0)
#define GL_CTX_DIRTY_BLEND_SRC_ALPHA      (1 << 1)
#define GL_CTX_DIRTY_BLEND_DST_RGB        (1 << 2)
#define GL_CTX_DIRTY_BLEND_DST_ALPHA      (1 << 3)
#define GL_CTX_DIRTY_BLEND_EQUATION_RGB   (1 << 4)
#define GL_CTX_DIRTY_BLEND_EQUATION_ALPHA (1 << 5)
#define GL_CTX_DIRTY_COLOR_MASK_R         (1 << 6)
#define GL_CTX_DIRTY_COLOR_MASK_G         (1 << 7)
#define GL_CTX_DIRTY_COLOR_MASK_B         (1 << 8)
#define GL_CTX_DIRTY_COLOR_MASK_A         (1 << 9)
#define GL_CTX_DIRTY_BLENDING             (1 << 10)
#define GL_CTX_DIRTY_DEPTH_RANGE_MIN      (1 << 11)
#define GL_CTX_DIRTY_DEPTH_RANGE_MAX      (1 << 12)
#define GL_CTX_DIRTY_DEPTH_TEST           (1 << 13)
#define GL_CTX_DIRTY_DEPTH_FUNC           (1 << 14)
#define GL_CTX_DIRTY_DEPTH_WRITE          (1 << 15)

#define GL_TEXTURE_DIRTY_WIDTH      (1 << 0)
#define GL_TEXTURE_DIRTY_HEIGTH     (1 << 1)
#define GL_TEXTURE_DIRTY_DEPTH      (1 << 2)
#define GL_TEXTURE_DIRTY_FORMAT     (1 << 3)
#define GL_TEXTURE_DIRTY_TARGET     (1 << 4)
#define GL_TEXTURE_DIRTY_WRAP_S     (1 << 5)
#define GL_TEXTURE_DIRTY_WRAP_T     (1 << 6)
#define GL_TEXTURE_DIRTY_WRAP_R     (1 << 7)
#define GL_TEXTURE_DIRTY_MIN_FILTER (1 << 8)
#define GL_TEXTURE_DIRTY_MAG_FILTER (1 << 9)

typedef void (*vertex_shader_fn_t)(struct vert *vert);
typedef GLboolean (*fragment_shader_fn_t)(const struct vert *vert, GLfloat *color);

#ifdef ENABLE_GCCJIT

typedef void (*fragment_set_fn_t)(GLfloat *color_buf, GLfloat *depth_buf, GLfloat *color, GLfloat depth, const GLfloat *constant);

struct fragment_set_state
{
	gcc_jit_result *jit_res;
	fragment_set_fn_t fn;
	GLuint hash;
	GLboolean blend;
	GLenum blend_src_rgb;
	GLenum blend_src_alpha;
	GLenum blend_dst_rgb;
	GLenum blend_dst_alpha;
	GLenum blend_equation_rgb;
	GLenum blend_equation_alpha;
	GLboolean color_mask[4];
	GLboolean depth_write;
	TAILQ_ENTRY(fragment_set_state) hash_chain;
	TAILQ_ENTRY(fragment_set_state) fifo_chain;
};

TAILQ_HEAD(fragment_set_head, fragment_set_state);

typedef GLboolean (*depth_test_fn_t)(GLfloat depth, const GLfloat *range, GLfloat cur);

struct depth_test_state
{
	gcc_jit_result *jit_res;
	depth_test_fn_t fn;
	GLuint hash;
	GLboolean depth_test;
	GLenum depth_func;
	GLclampf depth_range[2];
	TAILQ_ENTRY(depth_test_state) hash_chain;
	TAILQ_ENTRY(depth_test_state) fifo_chain;
};

TAILQ_HEAD(depth_test_head, depth_test_state);

typedef void (*texture_fetch_fn_t)(GLfloat *color, GLvoid *data, GLint s, GLint t, GLint p, GLint q);

struct texture_fetch_state
{
	gcc_jit_result *jit_res;
	texture_fetch_fn_t fn;
	GLuint hash;
	GLuint width; /* XXX this should not really be part of the state, but because textures dimensions are often PoT.... */
	GLuint height;
	GLuint depth;
	GLenum format;
	GLenum target;
	GLenum wrap_s;
	GLenum wrap_t;
	GLenum wrap_r;
	TAILQ_ENTRY(texture_fetch_state) hash_chain;
	TAILQ_ENTRY(texture_fetch_state) fifo_chain;
};

TAILQ_HEAD(texture_fetch_head, texture_fetch_state);

struct jit_ctx
{
	gcc_jit_context *ctx;
	gcc_jit_type *type_void;
	gcc_jit_type *type_voidp;
	gcc_jit_type *type_float;
	gcc_jit_type *type_floatp;
	gcc_jit_type *type_float4;
	gcc_jit_type *type_int;
	gcc_jit_type *type_uint8;
	gcc_jit_type *type_uint8p;
	gcc_jit_type *type_int32;
	gcc_jit_type *type_size_t;
	struct fragment_set_state *fragment_set_state;
	struct fragment_set_head fragment_set_fifo;
	struct fragment_set_head fragment_set_hash[512];
	uint32_t fragment_set_count;
	struct depth_test_state *depth_test_state;
	struct depth_test_head depth_test_fifo;
	struct depth_test_head depth_test_hash[512];
	uint32_t depth_test_count;
	struct texture_fetch_head texture_fetch_fifo;
	struct texture_fetch_head texture_fetch_hash[512];
	uint32_t texture_fetch_count;
};

#endif
struct vert
{
	GLuint front_face;
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
	GLfloat attr[MAX_VERTEX_ATTRIBS];
	GLfloat varying[MAX_VERTEX_ATTRIBS];
};

struct texture
{
	GLboolean initialized;
	GLuint dirty;
	GLuint width;
	GLuint height;
	GLuint depth;
	GLenum format;
	GLenum target;
	GLenum wrap_s;
	GLenum wrap_t;
	GLenum wrap_r;
	GLenum min_filter;
	GLenum mag_filter;
	GLvoid *data;
#ifdef ENABLE_GCCJIT
	struct texture_fetch_state *texture_fetch_state;
#endif
};

struct client_array
{
	GLboolean enabled;
	GLint size;
	GLenum type;
	GLsizei stride;
	const GLvoid *pointer;
};

struct light
{
	GLboolean enabled;
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

struct immediate
{
	GLboolean enabled;
	GLenum mode;
	GLfloat color[4];
	GLfloat normal[3];
	GLfloat tex_coord[4];
	struct vert verts[5];
	GLsizei vert_len; /* number of processed vertexes */
	GLsizei vert_pos; /* position of current write */
};

struct vertex_shader
{
	vertex_shader_fn_t fn;
	GLuint attr_nb;
	GLuint varying_nb;
};

struct fragment_shader
{
	fragment_shader_fn_t fn;
	GLuint varying_nb;
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
	struct list_cmd *cmds;
	GLsizei cmd_count;
	GLsizei cmd_size;
};

struct gl_ctx
{
	GLfloat *depth_buffer;
	GLfloat *color_buffer;
	GLubyte *stencil_buffer;
	GLsizei width;
	GLsizei height;
	GLenum matrix_mode;
	struct mat4 projection_matrix[PROJECTION_MAX_STACK_DEPTH];
	struct mat4 modelview_matrix[MODELVIEW_MAX_STACK_DEPTH];
	GLint projection_max_stack_depth;
	GLint modelview_max_stack_depth;
	GLint projection_stack_depth;
	GLint modelview_stack_depth;
	GLclampf clear_color[4];
	GLclampf clear_depth;
	GLclampf clear_stencil;
	GLboolean depth_test;
	GLboolean depth_write;
	GLenum depth_func;
	GLclampf depth_range[2];
	GLboolean blend;
	GLenum blend_src_rgb;
	GLenum blend_src_alpha;
	GLenum blend_dst_rgb;
	GLenum blend_dst_alpha;
	GLenum blend_equation_rgb;
	GLenum blend_equation_alpha;
	GLclampf blend_color[4];
	GLboolean fog;
	GLclampf fog_color[4];
	GLfloat fog_density;
	GLfloat fog_index;
	GLfloat fog_start;
	GLfloat fog_end;
	GLenum fog_mode;
	GLenum fog_coord_src;
	struct client_array vertex_array;
	struct client_array color_array;
	struct client_array texture_coord_array;
	struct client_array normal_array;
	struct texture **textures;
	GLuint textures_capacity;
	GLuint texture_binding_1d;
	GLuint texture_binding_2d;
	GLuint texture_binding_3d;
	GLboolean texture1d;
	GLboolean texture2d;
	GLboolean texture3d;
	GLenum errno;
	GLenum shade_model;
	GLfloat point_size;
	GLfloat line_width;
	GLboolean point_smooth;
	GLboolean line_smooth;
	GLboolean color_mask[4];
	GLenum front_face;
	GLenum cull_face;
	GLboolean enable_cull;
	struct light lights[MAX_LIGHTS];
	GLuint max_lights;
	GLboolean lighting;
	const GLvoid *indices;
	GLenum indice_type;
	struct material materials[2];
	GLboolean scissor_test;
	GLint scissor_x;
	GLint scissor_y;
	GLsizei scissor_width;
	GLsizei scissor_height;
	struct immediate immediate;
	struct vertex_shader vs;
	struct fragment_shader fs;
	GLuint dirty;
#ifdef ENABLE_GCCJIT
	struct jit_ctx jit;
#endif
};

GLsizei glSizeof(GLenum type);

void rast_fragment(struct vert *vs_out);
void rast_point(struct vert *vn);
void rast_line(struct vert *v1, struct vert *v2);
void rast_triangle(struct vert *v1, struct vert *v2, struct vert *v3);
void rast_texture_sample(const struct texture *texture, const GLfloat *coord, GLfloat *color);
void rast_normalize_vert(struct vert *vert);
GLboolean rast_depth_test(GLint x, GLint y, GLfloat test);

#ifdef ENABLE_GCCJIT
GLboolean jit_update_depth_test(struct gl_ctx *gl_ctx);
GLboolean jit_update_fragment_set(struct gl_ctx *gl_ctx);
GLboolean jit_update_texture_fetch(struct gl_ctx *gl_ctx, struct texture *texture);
#endif

void fixed_vertex_shader(struct vert *vert);
GLboolean fixed_fragment_shader(const struct vert *vert, GLfloat *color);

extern struct gl_ctx *g_ctx;

static inline void vert_mix(struct vert *dst, struct vert *v1,
                            struct vert *v2, GLfloat f)
{
	dst->front_face = v1->front_face;
	dst->x = mixf(v1->x, v2->x, f);
	dst->y = mixf(v1->y, v2->y, f);
	dst->z = mixf(v1->z, v2->z, f);
	dst->w = mixf(v1->w, v2->w, f);
	for (GLuint i = 0; i < g_ctx->vs.attr_nb; ++i)
		dst->attr[i] = mixf(v1->attr[i], v2->attr[i], f);
}

#endif
