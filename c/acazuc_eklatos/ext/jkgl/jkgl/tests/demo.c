#define __BSD_VISIBLE 1
#include <X11/keysym.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "common.h"
#include "glu.h"
#include "gl.h"

#define FOV 65

#define Z_MIN 0.01
#define Z_MAX 100

#define ROT_FAC 0.01
#define MOV_FAC 0.01

#define KEY_A     (1 << 0)
#define KEY_D     (1 << 1)
#define KEY_W     (1 << 2)
#define KEY_S     (1 << 3)
#define KEY_SHIFT (1 << 4)
#define KEY_SPACE (1 << 5)
#define KEY_LEFT  (1 << 6)
#define KEY_RIGHT (1 << 7)
#define KEY_UP    (1 << 8)
#define KEY_DOWN  (1 << 9)

struct vec2
{
	GLfloat x;
	GLfloat y;
};

struct vec3
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct vec4
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
};

struct obj
{
	struct vec4 *colors;
	struct vec3 *positions;
	struct vec2 *tex_coords;
	struct vec3 *normals;
	GLushort *indices;
	GLushort vertexes_nb;
	GLushort indices_nb;
	GLenum primitive;
	GLfloat px;
	GLfloat py;
	GLfloat pz;
	GLfloat rx;
	GLfloat ry;
	GLfloat rz;
	GLfloat sx;
	GLfloat sy;
	GLfloat sz;
};

struct env
{
	const char *progname;
	struct window window;
	float posx;
	float posy;
	float posz;
	float rotx;
	float roty;
	float rotz;
	uint16_t keys;
	struct obj *objects;
	size_t objects_nb;
};

float light01_ambient[4] = {0, 0, 0, 0};
float light01_diffuse[4] = {0, 0, .5, 1};
float light01_specular[4] = {0, 0, 1, 1};
float light23_ambient[4] = {0, 0, 0, 0};
float light23_diffuse[4] = {.5, 0, 0, 1};
float light23_specular[4] = {1, 0, 0, 1};
float light45_ambient[4] = {0, 0, 0, 0};
float light45_diffuse[4] = {0, .5, 0, 1};
float light45_specular[4] = {0, 1, 0, 1};
float light6_ambient[4] = {.05, .05, .05, 1};
float light6_diffuse[4] = {.5, .5, .5, 1};
float light6_specular[4] = {0, 0, 0, 0};
float light6_position[4] = {-5, -5, 5, 1};

float material_ambient[4] = {1, 1, 1, 1};
float material_diffuse[4] = {1, 1, 1, 1};
float material_specular[4] = {1, 1, 1, 1};
float material_emission[4] = {0, 0, 0, 1};

static void move(struct env *env)
{
	int left  = env->keys & KEY_LEFT;
	int right = env->keys & KEY_RIGHT;
	int up = env->keys & KEY_UP;
	int down = env->keys & KEY_DOWN;
	if (left && right)
	{
		left = 0;
		right = 0;
	}
	if (up && down)
	{
		up = 0;
		down = 0;
	}
	if (left)
		env->roty += ROT_FAC;
	if (right)
		env->roty -= ROT_FAC;
	if (up)
		env->rotx -= ROT_FAC;
	if (down)
		env->rotx += ROT_FAC;
	env->roty = fmod(env->roty, M_PI * 2);
	if (env->rotx > 90)
		env->rotx = 90;
	else if (env->rotx < -90)
		env->rotx = -90;
	int a = env->keys & KEY_A;
	int d = env->keys & KEY_D;
	int w = env->keys & KEY_W;
	int s = env->keys & KEY_S;
	if (a && d)
	{
		a = 0;
		d = 0;
	}
	if (w && s)
	{
		w = 0;
		s = 0;
	}
	if (env->keys & KEY_SHIFT)
		env->posy += MOV_FAC;
	if (env->keys & KEY_SPACE)
		env->posy -= MOV_FAC;
	if (!a && !d && !w && !s)
		return;
	double angle = TO_DEGREES(env->roty);
	if (s)
	{
		if (d)
			angle += 135;
		else if (a)
			angle += 45;
		else
			angle += 90;
	}
	else if (w)
	{
		if (d)
			angle += -135;
		else if (a)
			angle += -45;
		else
			angle += -90;
	}
	else if (d)
		angle += 180;
	else if (a)
		angle += 0;
	env->posx -= cos(TO_RADIANS(angle)) * MOV_FAC;
	env->posz += sin(TO_RADIANS(angle)) * MOV_FAC;
}

static void handle_key_press(struct window *window, KeySym sym)
{
	struct env *env = window->userptr;
	switch (sym)
	{
		case XK_Escape:
			exit(EXIT_SUCCESS);
		case XK_a:
			env->keys |= KEY_A;
			break;
		case XK_d:
			env->keys |= KEY_D;
			break;
		case XK_w:
			env->keys |= KEY_W;
			break;
		case XK_s:
			env->keys |= KEY_S;
			break;
		case XK_space:
			env->keys |= KEY_SPACE;
			break;
		case XK_Shift_L:
			env->keys |= KEY_SHIFT;
			break;
		case XK_Left:
			env->keys |= KEY_LEFT;
			break;
		case XK_Right:
			env->keys |= KEY_RIGHT;
			break;
		case XK_Up:
			env->keys |= KEY_UP;
			break;
		case XK_Down:
			env->keys |= KEY_DOWN;
			break;
	}
}

static void handle_key_release(struct window *window, KeySym sym)
{
	struct env *env = window->userptr;
	switch (sym)
	{
		case XK_a:
			env->keys &= ~KEY_A;
			break;
		case XK_d:
			env->keys &= ~KEY_D;
			break;
		case XK_w:
			env->keys &= ~KEY_W;
			break;
		case XK_s:
			env->keys &= ~KEY_S;
			break;
		case XK_space:
			env->keys &= ~KEY_SPACE;
			break;
		case XK_Shift_L:
			env->keys &= ~KEY_SHIFT;
			break;
		case XK_Left:
			env->keys &= ~KEY_LEFT;
			break;
		case XK_Right:
			env->keys &= ~KEY_RIGHT;
			break;
		case XK_Up:
			env->keys &= ~KEY_UP;
			break;
		case XK_Down:
			env->keys &= ~KEY_DOWN;
			break;
	}
}

static int setup_ctx(struct env *env)
{
	GL_CALL(glViewport, 0, 0, env->window.width, env->window.height);
	GL_CALL(glEnable, GL_DEPTH_TEST);
	GL_CALL(glClearDepth, 1);
	GL_CALL(glClearStencil, 0);
	GL_CALL(glEnableClientState, GL_VERTEX_ARRAY);
	GL_CALL(glEnableClientState, GL_COLOR_ARRAY);

#if 1
	GL_CALL(glFogi, GL_FOG_MODE, GL_LINEAR);
	GL_CALL(glFogf, GL_FOG_START, 1);
	GL_CALL(glFogf, GL_FOG_END, 3);
	GL_CALL(glEnable, GL_FOG);
#endif

#if 0
	GL_CALL(glFogi, GL_FOG_MODE, GL_EXP2);
	GL_CALL(glFogf, GL_FOG_DENSITY, 1);
	GL_CALL(glEnable, GL_FOG);
#endif

	GL_CALL(glDepthFunc, GL_LESS);
	GL_CALL(glEnable, GL_BLEND);
	GL_CALL(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GL_CALL(glEnable, GL_LIGHT0);
	GL_CALL(glLightfv, GL_LIGHT0, GL_AMBIENT, light01_ambient);
	GL_CALL(glLightfv, GL_LIGHT0, GL_DIFFUSE, light01_diffuse);
	GL_CALL(glLightfv, GL_LIGHT0, GL_SPECULAR, light01_specular);
	GL_CALL(glLightf, GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
	GL_CALL(glLightf, GL_LIGHT0, GL_LINEAR_ATTENUATION, 0);

	GL_CALL(glEnable, GL_LIGHT1);
	GL_CALL(glLightfv, GL_LIGHT1, GL_AMBIENT, light01_ambient);
	GL_CALL(glLightfv, GL_LIGHT1, GL_DIFFUSE, light01_diffuse);
	GL_CALL(glLightfv, GL_LIGHT1, GL_SPECULAR, light01_specular);
	GL_CALL(glLightf, GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1);
	GL_CALL(glLightf, GL_LIGHT1, GL_LINEAR_ATTENUATION, 0);

	GL_CALL(glEnable, GL_LIGHT2);
	GL_CALL(glLightfv, GL_LIGHT2, GL_AMBIENT, light23_ambient);
	GL_CALL(glLightfv, GL_LIGHT2, GL_DIFFUSE, light23_diffuse);
	GL_CALL(glLightfv, GL_LIGHT2, GL_SPECULAR, light23_specular);
	GL_CALL(glLightf, GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1);
	GL_CALL(glLightf, GL_LIGHT2, GL_LINEAR_ATTENUATION, 0);

	GL_CALL(glEnable, GL_LIGHT3);
	GL_CALL(glLightfv, GL_LIGHT3, GL_AMBIENT, light23_ambient);
	GL_CALL(glLightfv, GL_LIGHT3, GL_DIFFUSE, light23_diffuse);
	GL_CALL(glLightfv, GL_LIGHT3, GL_SPECULAR, light23_specular);
	GL_CALL(glLightf, GL_LIGHT3, GL_CONSTANT_ATTENUATION, 1);
	GL_CALL(glLightf, GL_LIGHT3, GL_LINEAR_ATTENUATION, 0);

	GL_CALL(glEnable, GL_LIGHT4);
	GL_CALL(glLightfv, GL_LIGHT4, GL_AMBIENT, light45_ambient);
	GL_CALL(glLightfv, GL_LIGHT4, GL_DIFFUSE, light45_diffuse);
	GL_CALL(glLightfv, GL_LIGHT4, GL_SPECULAR, light45_specular);
	GL_CALL(glLightf, GL_LIGHT4, GL_CONSTANT_ATTENUATION, 1);
	GL_CALL(glLightf, GL_LIGHT4, GL_LINEAR_ATTENUATION, 0);

	GL_CALL(glEnable, GL_LIGHT5);
	GL_CALL(glLightfv, GL_LIGHT5, GL_AMBIENT, light45_ambient);
	GL_CALL(glLightfv, GL_LIGHT5, GL_DIFFUSE, light45_diffuse);
	GL_CALL(glLightfv, GL_LIGHT5, GL_SPECULAR, light45_specular);
	GL_CALL(glLightf, GL_LIGHT5, GL_CONSTANT_ATTENUATION, 1);
	GL_CALL(glLightf, GL_LIGHT5, GL_LINEAR_ATTENUATION, 0);

	GL_CALL(glEnable, GL_LIGHT6);
	GL_CALL(glLightfv, GL_LIGHT6, GL_AMBIENT, light6_ambient);
	GL_CALL(glLightfv, GL_LIGHT6, GL_DIFFUSE, light6_diffuse);
	GL_CALL(glLightfv, GL_LIGHT6, GL_SPECULAR, light6_specular);
	GL_CALL(glLightf, GL_LIGHT6, GL_CONSTANT_ATTENUATION, 1);
	GL_CALL(glLightf, GL_LIGHT6, GL_LINEAR_ATTENUATION, 0);

	GL_CALL(glMaterialfv, GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
	GL_CALL(glMaterialfv, GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);
	GL_CALL(glMaterialfv, GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
	GL_CALL(glMaterialfv, GL_FRONT_AND_BACK, GL_EMISSION, material_emission);
	GL_CALL(glMaterialf, GL_FRONT_AND_BACK, GL_SHININESS, 50);

	GLuint texture;
	GL_CALL(glGenTextures, 1, &texture);
	GL_CALL(glBindTexture, GL_TEXTURE_2D, texture);
	GL_CALL(glShadeModel, GL_SMOOTH);
	static uint8_t data[16] =
	{
		0xFF, 0x00, 0x00, 0x40,
		0x00, 0xFF, 0x00, 0x80,
		0x00, 0x00, 0xFF, 0xC0,
		0xFF, 0xFF, 0xFF, 0xFF,
	};
	GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	GL_CALL(glEnable, GL_CULL_FACE);
	GL_CALL(glCullFace, GL_BACK);
	GL_CALL(glFrontFace, GL_CCW);
	return 0;
}

static int mkobj(struct obj *obj, uint16_t vertexes_nb, uint16_t indices_nb)
{
	obj->px = 0;
	obj->py = 0;
	obj->pz = 0;
	obj->rx = 0;
	obj->ry = 0;
	obj->rz = 0;
	obj->sx = 1;
	obj->sy = 1;
	obj->sz = 1;
	obj->vertexes_nb = vertexes_nb;
	obj->indices_nb = indices_nb;
	obj->colors = malloc(sizeof(*obj->colors) * vertexes_nb);
	if (!obj->colors)
		return 1;
	obj->positions = malloc(sizeof(*obj->positions) * vertexes_nb);
	if (!obj->positions)
		return 1;
	obj->tex_coords = malloc(sizeof(*obj->tex_coords) * vertexes_nb);
	if (!obj->tex_coords)
		return 1;
	obj->normals = malloc(sizeof(*obj->normals) * vertexes_nb);
	if (!obj->normals)
		return 1;
	obj->indices = malloc(sizeof(*obj->indices) * indices_nb);
	if (!obj->indices)
		return 1;
	return 0;
}

static int mkcube(struct obj *obj)
{
	if (mkobj(obj, 8, 36))
		return 1;
	static const struct vec4 colors[8] =
	{
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
	};
	static const struct vec3 positions[8] =
	{
		{-.5, -.5, +.5},
		{+.5, -.5, +.5},
		{+.5, +.5, +.5},
		{-.5, +.5, +.5},
		{-.5, -.5, -.5},
		{+.5, -.5, -.5},
		{+.5, +.5, -.5},
		{-.5, +.5, -.5},
	};
	static const struct vec2 tex_coords[8] =
	{
		{-5, -5},
		{ 5, -5},
		{ 5,  5},
		{-5,  5},
		{-5,  5},
		{ 5,  5},
		{ 5, -5},
		{-5, -5},
	};
	static const struct vec3 normals[8] =
	{
		{-1,  1,  1},
		{ 1,  1,  1},
		{ 1, -1,  1},
		{-1, -1,  1},
		{-1,  1, -1},
		{ 1,  1, -1},
		{ 1, -1, -1},
		{-1, -1, -1},
	};
	static const GLushort indices[36] =
	{
		0, 1, 2, 0, 2, 3, /* front */
		5, 4, 7, 5, 7, 6, /* back */
		4, 5, 1, 4, 1, 0, /* top */
		3, 2, 6, 3, 6, 7, /* bottom */
		4, 0, 3, 4, 3, 7, /* left */
		1, 5, 6, 1, 6, 2, /* right */
	};
	memcpy(obj->colors, colors, sizeof(colors));
	memcpy(obj->positions, positions, sizeof(positions));
	memcpy(obj->tex_coords, tex_coords, sizeof(tex_coords));
	memcpy(obj->normals, normals, sizeof(normals));
	memcpy(obj->indices, indices, sizeof(indices));
	obj->primitive = GL_TRIANGLES;
	return 0;
}

static int mksphere(struct obj *obj, uint16_t n)
{
	if (n < 3)
		return 1;
	if (mkobj(obj, n * n, n * (n - 1) * 6))
		return 1;
	size_t i = 0;
	for (size_t y = 0; y < n; ++y)
	{
		float fy = y / (float)(n - 1);
		float cy = cosf(fy * M_PI);
		float sy = sinf(fy * M_PI);
		for (size_t x = 0; x < n; ++x)
		{
			struct vec4 *color = &obj->colors[i];
			struct vec3 *position = &obj->positions[i];
			struct vec2 *tex_coord = &obj->tex_coords[i];
			struct vec3 *normal = &obj->normals[i];
			float fx = x / (float)n;
			float cx = cosf(fx * M_PI * 2);
			float sx = sinf(fx * M_PI * 2);
			color->x = 1;
			color->y = 1;
			color->z = 1;
			color->w = 1;
			position->x = cx * sy;
			position->y = -cy;
			position->z = sx * sy;
			tex_coord->x = fx * 20;
			tex_coord->y = fy * 20;
			normal->x = position->x;
			normal->y = position->y;
			normal->z = position->z;
			i++;
		}
	}
	i = 0;
	for (size_t y = 0; y < (n - 1u); ++y)
	{
		size_t idx_base = y * n;
		for (size_t x = 0; x < n; ++x)
		{
			size_t t = (x + 1) % n;
			obj->indices[i++] = idx_base + x;
			obj->indices[i++] = idx_base + x + n;
			obj->indices[i++] = idx_base + t + n;
			obj->indices[i++] = idx_base + x;
			obj->indices[i++] = idx_base + t + n;
			obj->indices[i++] = idx_base + t;
		}
	}
	obj->primitive = GL_TRIANGLES;
	return 0;
}

static void draw_obj(const struct obj *obj)
{
	GL_CALL(glPushMatrix);
	GL_CALL(glTranslatef, obj->px, obj->py, obj->pz);
	GL_CALL(glRotatef, obj->rz, 0, 0, 1);
	GL_CALL(glRotatef, obj->ry, 0, 1, 0);
	GL_CALL(glRotatef, obj->rx, 1, 0, 0);
	GL_CALL(glScalef, obj->sx, obj->sy, obj->sz);
	GL_CALL(glColorPointer, 4, GL_FLOAT, 0, obj->colors);
	GL_CALL(glVertexPointer, 3, GL_FLOAT, 0, obj->positions);
	GL_CALL(glTexCoordPointer, 2, GL_FLOAT, 0, obj->tex_coords);
	GL_CALL(glNormalPointer, GL_FLOAT, 0, obj->normals);
	GL_CALL(glDrawElements, obj->primitive, obj->indices_nb, GL_UNSIGNED_SHORT, obj->indices);
	GL_CALL(glPopMatrix);
}

static void render(struct env *env)
{
	uint64_t frametime = nanotime();

	GL_CALL(glViewport, 0, 0, env->window.width, env->window.height);

	float hsl[3];
	float rgb[4];
	hsl[0] = fmodf((frametime % 5000000000) / 5000000000., 1);
	hsl[1] = 0.5;
	hsl[2] = 0.5;
	hsl2rgb(rgb, hsl);
	GL_CALL(glFogfv, GL_FOG_COLOR, rgb);
	GL_CALL(glClearColor, rgb[0], rgb[1], rgb[2], 1);
	GL_CALL(glClear, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	GL_CALL(glMatrixMode, GL_PROJECTION);
	GL_CALL(glLoadIdentity);
	GL_CALL(gluPerspective, FOV, env->window.width / (float)env->window.height, Z_MIN, Z_MAX);

	GL_CALL(glMatrixMode, GL_MODELVIEW);
	GL_CALL(glLoadIdentity);
	GL_CALL(glRotated, -env->rotx, 1, 0, 0);
	GL_CALL(glRotated, -env->roty, 0, 1, 0);
	GL_CALL(glTranslated, -env->posx, -env->posy, -env->posz);
	GL_CALL(glScaled, 1, 1, 1);

#if 1
	{
		GL_CALL(glEnableClientState, GL_VERTEX_ARRAY);
		GL_CALL(glEnableClientState, GL_COLOR_ARRAY);
		GL_CALL(glDisable, GL_LIGHTING);
		float n = (frametime % 2000000000) / 2000000000.0;
		float c = cosf(n * M_PI);
		float s = sinf(n * M_PI);
		float d = 1;
		float light0_position[4] = {+c * d, +s * d, 0, 1};
		float light1_position[4] = {-c * d, -s * d, 0, 1};
		float light2_position[4] = {0, +c * d, -s * d, 1};
		float light3_position[4] = {0, -c * d, +s * d, 1};
		float light4_position[4] = {+c * d, 0, -s * d, 1};
		float light5_position[4] = {-c * d, 0, +s * d, 1};
		GL_CALL(glLightfv, GL_LIGHT0, GL_POSITION, light0_position);
		GL_CALL(glLightfv, GL_LIGHT1, GL_POSITION, light1_position);
		GL_CALL(glLightfv, GL_LIGHT2, GL_POSITION, light2_position);
		GL_CALL(glLightfv, GL_LIGHT3, GL_POSITION, light3_position);
		GL_CALL(glLightfv, GL_LIGHT4, GL_POSITION, light4_position);
		GL_CALL(glLightfv, GL_LIGHT5, GL_POSITION, light5_position);
		GL_CALL(glLightfv, GL_LIGHT6, GL_POSITION, light6_position);
		GL_CALL(glPointSize, 5);
		GL_CALL(glVertexPointer, 3, GL_FLOAT, 0, light0_position);
		GL_CALL(glColorPointer, 3, GL_FLOAT, 0, light01_diffuse);
		GL_CALL(glDrawArrays, GL_POINTS, 0, 1);
		GL_CALL(glVertexPointer, 3, GL_FLOAT, 0, light1_position);
		GL_CALL(glColorPointer, 3, GL_FLOAT, 0, light01_diffuse);
		GL_CALL(glDrawArrays, GL_POINTS, 0, 1);
		GL_CALL(glVertexPointer, 3, GL_FLOAT, 0, light2_position);
		GL_CALL(glColorPointer, 3, GL_FLOAT, 0, light23_diffuse);
		GL_CALL(glDrawArrays, GL_POINTS, 0, 1);
		GL_CALL(glVertexPointer, 3, GL_FLOAT, 0, light3_position);
		GL_CALL(glColorPointer, 3, GL_FLOAT, 0, light23_diffuse);
		GL_CALL(glDrawArrays, GL_POINTS, 0, 1);
		GL_CALL(glVertexPointer, 3, GL_FLOAT, 0, light4_position);
		GL_CALL(glColorPointer, 3, GL_FLOAT, 0, light45_diffuse);
		GL_CALL(glDrawArrays, GL_POINTS, 0, 1);
		GL_CALL(glVertexPointer, 3, GL_FLOAT, 0, light5_position);
		GL_CALL(glColorPointer, 3, GL_FLOAT, 0, light45_diffuse);
		GL_CALL(glDrawArrays, GL_POINTS, 0, 1);
		GL_CALL(glDisableClientState, GL_VERTEX_ARRAY);
		GL_CALL(glDisableClientState, GL_COLOR_ARRAY);
	}
#endif

#if 0
	{
		int64_t t = (frametime / 1000000000) % 3;
		static const GLenum filters[] = {GL_NEAREST, GL_LINEAR, GL_CUBIC};
		GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filters[t]);
	}
#endif

#if 0
	{
		int64_t t = (frametime / 1000000000) % 4;
		GL_CALL(glColorMask, t != 0, t != 1, t != 2, 1);
	}
#endif

#if 1
	GL_CALL(glEnable, GL_LIGHTING);
	GL_CALL(glEnableClientState, GL_TEXTURE_COORD_ARRAY);
	GL_CALL(glEnableClientState, GL_NORMAL_ARRAY);
	GL_CALL(glEnableClientState, GL_COLOR_ARRAY);
	GL_CALL(glEnableClientState, GL_VERTEX_ARRAY);
	GL_CALL(glEnable, GL_TEXTURE_2D);
	GL_CALL(glMatrixMode, GL_MODELVIEW);
	for (size_t i = 0; i < env->objects_nb; ++i)
		draw_obj(&env->objects[i]);
	GL_CALL(glDisable, GL_TEXTURE_2D);
	GL_CALL(glDisableClientState, GL_VERTEX_ARRAY);
	GL_CALL(glDisableClientState, GL_COLOR_ARRAY);
	GL_CALL(glDisableClientState, GL_NORMAL_ARRAY);
	GL_CALL(glDisableClientState, GL_TEXTURE_COORD_ARRAY);
	GL_CALL(glDisable, GL_LIGHTING);
#endif
}

int main(int argc, char **argv)
{
	struct env env;

	(void)argc;
	memset(&env, 0, sizeof(env));
	env.progname = argv[0];
	if (setup_window(argv[0], &env.window))
		return EXIT_FAILURE;
	env.window.userptr = &env;
	env.window.on_key_down = handle_key_press;
	env.window.on_key_up = handle_key_release;
	if (setup_ctx(&env))
		return EXIT_FAILURE;

	env.objects_nb = 2;
	env.objects = malloc(sizeof(*env.objects) * env.objects_nb);
	if (!env.objects)
	{
		fprintf(stderr, "%s: failed to malloc objects\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (mkcube(&env.objects[0]))
	{
		fprintf(stderr, "%s: failed to create cube\n", argv[0]);
		return EXIT_FAILURE;
	}
	env.objects[0].py = 1;
	if (mksphere(&env.objects[1], 100))
	{
		fprintf(stderr, "%s: failed to create sphere\n", argv[0]);
		return EXIT_FAILURE;
	}
	env.objects[1].sx = 0.75;
	env.objects[1].sy = 0.75;
	env.objects[1].sz = 0.75;
	env.posz = 2;
	while (1)
	{
		handle_events(&env.window);
		render(&env);
		swap_buffers(&env.window);
		move(&env);
	}
	return EXIT_SUCCESS;
}
